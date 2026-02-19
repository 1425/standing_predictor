#include "event_limits.h"
#include "pick_points.h"
#include "tba.h"
#include "playoff_limits.h"
#include "skill_opr.h"
#include "declines.h"

using namespace std;

/*
 * Might we worthwhile to do a quick study of the amount of entropy coming in and out of each of the 
 * tournament stages
 *
 * also, should do a comparison of the overall entropy calculated for event/district with these methods
 * compared to the basic ones
 *
 * also, on the list of things to do should be to look for third events so that the points don't get added
 * first pass, this should just go when adding up a district
 * but could eventually go deeper because I'm pretty sure that they won't give teams the chairmans award
 * at their third event, etc. but should look at that empirically.
 * */

using Team=tba::Team_key;

template<typename K,typename V>
void reserve_if_able(flat_map<K,V> &a,size_t n){
	a.reserve(n);
}

void reserve_if_able(auto&,size_t){}

template<typename Func,typename K,typename V>
auto map_values(Func f,flat_map2<K,V> const& a){
	return a.map_values(f);
}

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

template<template<typename,typename>typename MAP>
MAP<Point,Pr> normalize(MAP<Point,Pr> a){
	auto s=sum(values(a));
	assert(s>0);
	return map_values([=](auto x){ return x/s; },a);
}

map<Point,Pr> cut_negatives(map<Point,Pr> a){
	return map_values([](auto x){ return max(x,0.0); },a);
}

//map<Point,Pr> uniform(Interval<Point> a){
auto uniform(Interval<Point> a){
	auto options=a.max-a.min+1;
	flat_map2<Point,Pr> r;
	reserve_if_able(r,options);
	for(auto x:range(a)){
		r[x]=1.0/options;
	}
	return r;
}

template<template<typename,typename> typename MAP>
//map<Point,Pr> cut(MAP<Point,Pr> prior,Interval<Point> allowed){
auto cut(MAP<Point,Pr> prior,Interval<Point> allowed){
	MAP<Point,Pr> r;
	reserve_if_able(r,allowed.max-allowed.min+1);
	/* N=prior.size()
	 * M=range(allowed).size()
	 *
	 * original version O(M*log(N)+M*log(M))
	 * more complicated version: O(log(N)+M)
	 * because the input is sorted
	 * */
	/*for(auto x:range(allowed)){
		r[x]=prior[x];
	}*/

	using P=std::pair<Point,Pr>;
	auto start=lower_bound(prior.begin(),prior.end(),P(allowed.min,0));

	auto end=lower_bound(prior.begin(),prior.end(),P(allowed.max,0));
	//because Interval is inclusive.
	if(end!=prior.end() && (*end).first==allowed.max){
		++end;
	}

	auto out=r.begin();
	for(auto it=start;it!=end;++it){
		out=r.emplace_hint(out,*it);
	}

	return normalize(r);
}

template<typename T>
concept Map_like = requires(T m, typename T::key_type k, typename T::mapped_type v) {
    // Requires key_type and mapped_type to exist
    typename T::key_type;
    typename T::mapped_type;

    // Requires operator[] for insertion/access
    //{ m[k] } -> std::same_as<typename T::mapped_type&>;

    // Requires the insert function
    //{ m.insert({k, v}) };
};

template<
	Map_like MAP1,
	template<typename,typename>typename MAP2,
	typename K,
	typename V
>
MAP1& operator+=(MAP1 &a,MAP2<K,V> const& b){
	for(auto const& [k,v]:b){
		a[k]+=v;
	}
	return a;
}

auto force_probability(map<Team,Interval<Point>> by_team,flat_map2<Point,Pr> prior_dist){
	bool verbose=0;
	flat_map2<Point,Pr> existing;
	for(auto v:values(by_team)){
		existing+=uniform(v);
	}
	if(verbose){
		cout<<"Existing, no prior:\n";
		cout<<existing<<"\n";
		PRINT(sum(values(existing)));
		cout<<"\n";
	}

	flat_map2<Point,Pr> by_cut;
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

Rank_status district_limits(TBA_fetcher &f,tba::District_key const& district){
	auto e=events(f,district);

	//Ignore DCMP points for now.
	auto e2=filter([](auto x){ return x.event_type==tba::Event_type::DISTRICT; },e);

	//sort so that can figure out third plays
	e2=sort_by(e2,[](auto x){ return x.start_date; });

	auto m=mapf([&](auto const& x){ return event_limits(f,x); },e2);
	map<Team,int> plays;
	Rank_status r;
	for(auto here:m){
		for(auto [k,v]:here.by_team){
			auto&p=plays[k];
			if(p<2){
				r.by_team[k]+=v;
			}
			p++;
		}
		r.unclaimed+=here.unclaimed;
	}
	PRINT(count(values(plays)));
	return r;
	//return sum(m);
}

int event_limits_demo(TBA_fetcher &f){
	for(auto district:take(20,districts(f))){
		PRINT(district);
		auto a=district_limits(f,district);
		//print_r(a);
	}
	return 0;
	for(auto const& event:events(f)){
		auto a=event_limits(f,event.key);
		(void)a;
	}
	return 0;
}
