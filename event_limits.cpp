#include "event_limits.h"
#include "pick_points.h"
#include "tba.h"
#include "playoff_limits.h"
#include "skill_opr.h"
#include "declines.h"
#include "lock2.h"
#include "pick_points.h"
#include "winners.h"

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
 *
 * Would be nice if had a display page for all of this
 * -Per team, what do the limits look like for each stage and overall, limits of rank?
 *  could
 *
 *  -would be good to make a graph of rank entropy as more matches are played
 *  -also each stage should produce a listing of what it thinks its status is
 * */

using Team=tba::Team_key;

ENUM_CLASS_PRINT(Tournament_status,TOURNAMENT_STATUS)

auto options(Tournament_status const*){
	return std::array{
	#define X(A) Tournament_status::A,
	TOURNAMENT_STATUS(X)
	#undef X
	};
}

Tournament_status rand(Tournament_status const* x){
	return choose(options(x));
}

ENUM_CLASS_PRINT(District_status,DISTRICT_STATUS)

template<typename K,typename V>
void reserve_if_able(flat_map<K,V> &a,size_t n){
	a.reserve(n);
}

void reserve_if_able(auto&,size_t){}

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

auto entropy(Rank_results<tba::Team_key> const& a){
	return entropy(a.points);//might actually want to look at ranks instead
}

template<typename T>
auto entropy(std::vector<T> const& a){
	return sum(MAP(entropy,a));
}

auto entropy(std::map<tba::Team_key,Interval<Point>> a){
	return entropy(values(a));
}

auto entropy(Pick_limits const& a){
	return entropy(a.points);
}

auto entropy(Playoff_limits const& a){
	return entropy(a.by_team);
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

//Rank_status event_limits(TBA_fetcher &f,tba::Event_key const& event){
Rank_status<Tournament_status> event_limits(TBA_fetcher &f,tba::Event_key const& event){
	Tournament_status tstatus=Tournament_status::FUTURE;
	//PRINT(event);
	auto ranks=rank_limits(f,event);
	ranks.check();
	auto t1=teams(ranks.ranks);
	auto t2=teams(ranks.points);
	assert(t1==t2);
	create_prior(ranks.points,ranks.unclaimed_points);
	//PRINT(entropy(ranks))
	//PRINT(ranks.status);
	std::visit([&](auto x){
		using T=std::decay_t<decltype(x)>;
		if constexpr(std::is_same<T,Qual_status_future>()){
			//do nothing
		}else if constexpr(std::is_same<T,Qual_status_in_progress>()){
			tstatus=Tournament_status::QUAL_MATCHES_IN_PROGRESS;
		}else if constexpr(std::is_same<T,Qual_status_complete>()){
			tstatus=Tournament_status::QUAL_MATCHES_COMPLETE;
		}else{
			assert(0);
		}
	},ranks.status);

	auto picks=pick_limits(f,event,ranks.ranks);
	auto t3=teams(picks.points);
	auto t4=teams(picks.picked);
	assert(t3==t4);
	assert(subset(t1,t3));
	t1=t3;
	create_prior(picks.points,picks.unclaimed);
	//p needs to bring both point totals for each of the teams
	//and also for each team whether they are on an alliance or not or don't know. interval<bool>?
	//and also how many selection points may be left
	//PRINT(entropy(picks));
	//PRINT(picks.status);
	switch(picks.status){
		case Event_status::FUTURE:
			break;
		case Event_status::IN_PROGRESS:
			tstatus=Tournament_status::PICKING_IN_PROGRESS;
			break;
		case Event_status::COMPLETE:
			tstatus=Tournament_status::PICKING_COMPLETE;
			break;
		default:
			assert(0);
	}

	//playoff_limits(f,in_playoffs);
	//just needs be team -> interval<Point>
	auto playoffs=playoff_limits(f,event,picks.picked);
	auto t5=teams(playoffs.by_team);
	assert(subset(t1,t5));
	//print_r(playoffs);
	create_prior(playoffs.by_team,playoffs.unclaimed_points);
	//PRINT(entropy(playoffs));
	//PRINT(playoffs.status);

	switch(playoffs.status){
		case Event_status::FUTURE:
			break;
		case Event_status::IN_PROGRESS:
			tstatus=Tournament_status::ELIMINATIONS_IN_PROGRESS;
			break;
		case Event_status::COMPLETE:
			tstatus=Tournament_status::ELIMINATIONS_COMPLETE;
			break;
		default:
			assert(0);
	}

	auto d=award_limits(f,event,t1);
	auto t6=teams(d.by_team);
	//PRINT(entropy(d));
	//PRINT(d.status);
	//because you can just show up at the end and win rookie all-star at a district championship
	//and not have played any matches, etc.
	//and it probably doesn't make sense to force the earlier steps to include those teams

	if(!subset(t5,t6)){
		diff(t5,t6);
	}

	assert(subset(t5,t6));

	switch(d.status){
		case Event_status::FUTURE:
			break;
		case Event_status::IN_PROGRESS:
			tstatus=Tournament_status::AWARDS_IN_PROGRESS;
			break;
		case Event_status::COMPLETE:
			tstatus=Tournament_status::COMPLETE;
			break;
		default:
			assert(0);
	}

	{
		auto [by_team,unclaimed,status]=points_only(d);
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
		auto [by_team,unclaimed,status]=points_only(d);
		auto c=create_prior(by_team,unclaimed);
		auto f=force_probability(by_team,c);
		//print_r(f);
	}

	Rank_status<Tournament_status> r;
	r.by_team=d.by_team;
	r.unclaimed=d.unclaimed;
	r.status=tstatus;
	return r;
}

auto event_limits(TBA_fetcher &f,tba::Event const& e){
	return event_limits(f,e.key);
}

Tournament_status dcmp_status(TBA_fetcher &f,tba::District_key const& district){
	auto e=events(f,district);
	auto e2=filter([](auto x){ return x.event_type!=tba::Event_type::DISTRICT; },e);
	auto status=mapf(
		[&](auto x){ return make_pair(x,event_limits(f,x.key).status); },
		e2
	);

	if(status.empty()){
		return Tournament_status::COMPLETE;
	}

	if(status.size()==1){
		return status[0].second;
	}

	auto divisions=filter([](auto x){ return x.first.event_type==tba::Event_type::DISTRICT_CMP_DIVISION; },status);

	if(!divisions.empty()){
		auto latest_division_status=min(mapf([](auto x){ return x.second; },divisions));
		if(latest_division_status!=Tournament_status::COMPLETE){
			return latest_division_status;
		}
	}

	auto main_fields=filter([](auto x){ return x.first.event_type==tba::Event_type::DISTRICT_CMP; },status);
	return min(seconds(main_fields));
}

Rank_status<District_status> district_limits(TBA_fetcher &f,tba::District_key const& district){
	auto e=events(f,district);

	//Ignore DCMP points for now.
	auto e2=filter([](auto x){ return x.event_type==tba::Event_type::DISTRICT; },e);

	//sort so that can figure out third plays
	e2=sort_by(e2,[](auto x){ return x.start_date; });

	auto m=mapf([&](auto const& x){ return event_limits(f,x); },e2);

	/*for(auto [i,x]:enumerate(m)){
		cout<<i<<"\t"<<entropy(x)<<"\n";
	}*/

	auto local_status=to_set(mapf([](auto x){ return x.status; },m));
	//PRINT(local_status);

	map<Team,int> plays;
	Rank_status<District_status> r;
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
	//PRINT(count(values(plays)));
	
	r.status=[&](){
		if(local_status==set<Tournament_status>{Tournament_status::FUTURE}){
			return District_status::FUTURE;
		}
		if(local_status!=set<Tournament_status>{Tournament_status::COMPLETE}){
			return District_status::LOCALS_IN_PROGRESS;
		}
		auto d=dcmp_status(f,district);
		if(d==Tournament_status::FUTURE){
			return District_status::LOCALS_COMPLETE;
		}
		if(d==Tournament_status::COMPLETE){
			return District_status::COMPLETE;
		}
		return District_status::DCMP_IN_PROGRESS;
	}();

	return r;
	//return sum(m);
}

void xx(TBA_fetcher &f){
	(void)f;
	//future
	//qual matches in progress
	//qual matches complete
	//picking in progress
	//picking complete
	//elimination matches in progress
	//elimination matches complete
	//awards in progress
	//complete
}

int event_limits_demo(TBA_fetcher &f){
	//return lock2_demo(f);
	//return winners_demo(f);

	if(0){
		auto a=event_limits(f,tba::Event_key("2026cahal"));
		PRINT(entropy(a));
		return 0;
	}

	if(0){
		auto a=district_limits(f,tba::District_key("2025fim"));
		PRINT(a);
		return 0;
	}

	/*auto d=district_limits(f,tba::District_key("2021fin"));
	PRINT(entropy(d));
	return 0;*/

	/*for(auto const& event:events(f)){
		PRINT(event.key);
		auto a=event_limits(f,event.key);
		(void)a;
	}*/
	for(auto district:districts(f)){
		PRINT(district);
		auto a=district_limits(f,district);
		//print_r(a);
		PRINT(a.status);
		PRINT(entropy(a));
	}
	return 0;
}
