#include "event_limits.h"
#include "pick_points.h"
#include "tba.h"
#include "playoff_limits.h"
#include "skill_opr.h"
#include "declines.h"

using namespace std;

using Team=tba::Team_key;

template<
	template<typename,typename>typename MAP1,
	template<typename,typename>typename MAP2,
	typename K,
	typename V1,
	typename V2
>
auto join_hard(MAP1<K,V1> a,MAP2<K,V2> b){
	auto ka=to_set(keys(a));
	auto kb=to_set(keys(b));
	if(ka!=kb){
		diff(ka,kb);
	}
	assert(ka==kb);
	using P=std::pair<V1,V2>;
	map<K,P> r;
	for(auto k:keys(a)){
		r[k]=P(a[k],b[k]);
	}
	return r;
}

template<typename T>
vector<T> range(Interval<T> const& a){
	vector<T> r;
	for(T at=a.min;at<=a.max;++at){
		r|=at;
	}
	return r;
}

template<typename T>
auto make_dist(std::vector<T> a){
	return to_dist(to_multiset(a));
}

map<Point,Pr> normalize(map<Point,Pr> a){
	auto s=sum(values(a));
	assert(s>0);
	return map_values([=](auto x){ return x/s; },a);
}

map<Point,Pr> cut_negatives(map<Point,Pr> a){
	return map_values([](auto x){ return max(x,0.0); },a);
}

map<Point,Pr> uniform(Interval<Point> a){
	auto options=a.max-a.min+1;
	map<Point,Pr> r;
	for(auto x:range(a)){
		r[x]=1.0/options;
	}
	return r;
}

template<template<typename,typename> typename MAP>
map<Point,Pr> cut(MAP<Point,Pr> prior,Interval<Point> allowed){
	map<Point,Pr> r;
	for(auto x:range(allowed)){
		r[x]=prior[x];
	}
	return normalize(r);
}

auto force_probability(map<Team,Interval<Point>> by_team,flat_map2<Point,Pr> prior_dist){
	bool verbose=0;
	std::map<Point,Pr> existing;
	for(auto v:values(by_team)){
		existing+=uniform(v);
	}
	if(verbose){
		cout<<"Existing, no prior:\n";
		cout<<existing<<"\n";
		PRINT(sum(values(existing)));
		cout<<"\n";
	}

	std::map<Point,Pr> by_cut;
	for(auto v:values(by_team)){
		by_cut+=cut(prior_dist,v);
	}
	if(verbose){
		std::cout<<"By cut:\n";
		cout<<by_cut<<"\n";
		//print_lines(by_cut);
		PRINT(sum(values(by_cut)));
		cout<<"\n";
	}

	/*auto residual=prior_dist-normalize(by_cut);
	cout<<"Residual:\n";
	cout<<residual<<"\n";*/
	/*map<Point,double> ratio=map_values(
		[](auto p){
			auto [a,b]=p;
			return a/b;
		},
		join_hard(by_cut,prior_dist)
	);*/
	map<Point,double> ratio;
	for(auto [k,v]:prior_dist){
		assert(v);
		ratio[k]=by_cut[k]/v;
	}
	//might want to fill in zeros for the rest of the places.
	if(verbose){
		cout<<"Ratio:\n";
		cout<<ratio<<"\n";
	}

	map<Point,Pr> after_ratio=map_values(
		[](auto x){ return x.first*x.second; },
		join_hard(prior_dist,ratio)
	);

	//auto r2=normalize(cut_negatives(residual));

	//map<Team,Team_dist> r=map_values(
	auto r=map_values(
		[=](auto x){ return cut(after_ratio,x); },
		by_team
	);
	return r;
}

//map<Point,Pr> create_prior(std::map<Team,Interval<Point>> by_team,Point unassigned){
auto create_prior(std::map<Team,Interval<Point>> by_team,Point unassigned){
	//this is going to be ugly
	//and for test pruposes only
	//just find an example that works, and return that distribution
	auto v=values(by_team);
	auto working_room=sum(mapf([](auto x){ return x.max-x.min; },v));
	//PRINT(working_room);
	//PRINT(unassigned);
	assert(working_room>=unassigned);
	
	vector<Point> found;
	for(auto &v1:v){
		Point space=v1.max-v1.min; //TODO: Figure out why this doesn't match auto
		auto to_take=min(unassigned,space);
		unassigned-=to_take;
		found|=v1.min+to_take;
	}
	assert(unassigned==0);
	using T=decltype(make_dist(found));
	if(found.empty()){
		//this will be an invalid distribution. because the total probability will be 0, not 1.
		return T();
	}
	return make_dist(found);
}

pair<map<Team,Interval<Point>>,Point> points_only(Rank_status a){
	//map<Team,Interval<Point>> m;
	//nyi
	auto m=map_values(
		[](auto x){
			return Interval{x.min.second,x.max.second};
		},
		a.by_team
	);
	return make_pair(m,a.unclaimed.second);
}

double entropy(Interval<Rank_value> a){
	auto f1=Interval{a.min.first,a.max.first};
	auto f2=Interval{a.min.second,a.max.second};

	return entropy(f1)+entropy(f2);
}

double entropy(Rank_status const& a){
	return sum(MAP(entropy,values(a.by_team)));
}

std::set<tba::Team_key> teams(Rank_range<tba::Team_key> a){
	return keys(a);
}

std::set<tba::Team_key> teams(Point_range<tba::Team_key> a){
	return keys(a);
}

template<typename T>
std::set<tba::Team_key> teams(std::map<tba::Team_key,T> a){
	return keys(a);
}

Rank_status event_limits(TBA_fetcher &f,tba::Event_key const& event){
	PRINT(event);
	auto ranks=rank_limits(f,event);
	ranks.check();
	auto t1=teams(ranks.ranks);
	auto t2=teams(ranks.points);
	assert(t1==t2);
	create_prior(ranks.points,ranks.unclaimed_points);

	auto picks=pick_limits(f,event,ranks.ranks);
	auto t3=teams(picks.points);
	auto t4=teams(picks.picked);
	assert(t3==t4);
	assert(t3==t1);
	create_prior(picks.points,picks.unclaimed);
	//p needs to bring both point totals for each of the teams
	//and also for each team whether they are on an alliance or not or don't know. interval<bool>?
	//and also how many selection points may be left

	//playoff_limits(f,in_playoffs);
	//just needs be team -> interval<Point>
	auto playoffs=playoff_limits(f,picks.picked);
	assert(playoffs.by_team.size()==ranks.ranks.size());
	auto t5=teams(playoffs.by_team);
	assert(t5==t1);
	//print_r(playoffs);
	create_prior(playoffs.by_team,playoffs.unclaimed_points);

	auto d=award_limits(f,event,t1);
	auto t6=teams(d.by_team);

	//because you can just show up at the end and win rookie all-star at a district championship
	//and not have played any matches, etc.
	//and it probably doesn't make sense to force the earlier steps to include those teams
	assert(subset(t1,t6));

	{
		auto [by_team,unclaimed]=points_only(d);
		create_prior(by_team,unclaimed);
	}

	auto add_map=[&](auto x){
		for(auto [k,v]:x){
			d.by_team[k]+=Interval(Rank_value(0,v.min),Rank_value(0,v.max));
		}
	};
	add_map(ranks.points);
	d.unclaimed.second+=ranks.unclaimed_points;

	add_map(picks.points);
	d.unclaimed.second+=picks.unclaimed;

	add_map(playoffs.by_team);
	d.unclaimed.second+=playoffs.unclaimed_points;

	{
		auto [by_team,unclaimed]=points_only(d);
		auto c=create_prior(by_team,unclaimed);
		auto f=force_probability(by_team,c);
		//print_r(f);
	}

	return d;
}

auto event_limits(TBA_fetcher &f,tba::Event const& e){
	return event_limits(f,e.key);
}

auto district_limits(TBA_fetcher &f,tba::District_key const& district){
	auto e=events(f,district);
	auto m=mapf([&](auto const& x){ return event_limits(f,x); },e);
	return sum(m);
}

int event_limits_demo(TBA_fetcher &f){
	for(auto district:districts(f)){
		PRINT(district);
		auto a=district_limits(f,district);
		//print_r(a);
	}
	for(auto const& event:events(f)){
		auto a=event_limits(f,event.key);
		(void)a;
	}
	return 0;
}
