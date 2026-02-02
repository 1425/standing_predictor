#include "skill_opr.h"
#include "set.h"
#include "../tba/tba.h"
#include "tba.h"
#include "vector_void.h"
#include "optional.h"

using Year=tba::Year;
using District_abbreviation=tba::District_abbreviation;
using Team=tba::Team_key;
using Team_key=tba::Team_key;
using District_key=tba::District_key;
using Event_Simple=tba::Event_Simple;

#define MAP_VALUES(A,B) map_values([&](auto x){ return (A)(x); },B)

using namespace std;

template<typename Func>
auto filter(Func f,Team_dist b){
	Team_dist r;
	for(auto p:b){
		if(f(p)){
			r[p.first]=p.second;
		}
	}
	return r;
}

void check_dist(Team_dist a){
	auto s=sum(values(a));
	bool ok=(s>=.99 && s<=1.01);
	if(!ok){
		PRINT(a);
		PRINT(s);
	}
	assert(ok);
}

template<typename T>
auto to_dist(std::multiset<T> const& a){
	assert(!a.empty());
	flat_map2<T,Pr> r;
	for(auto k:to_set(a)){
		r[k]=(0.0+a.count(k))/a.size();
	}
	check_dist(r);
	return r;
}

template<typename T>
auto to_dist(std::vector<T> const& a){
	return to_dist(to_multiset(a));
}

Team_dist cut_dist(Team_dist const& a){
	check_dist(a);

	//take out the values that are less than 0 because those are actual point totals.
	auto f=filter([](auto x){ return x.first>=0; },a);
	auto total=sum(values(f));
	assert(total>0);

	Team_dist r;
	for(auto [k,v]:f){
		r[k]=v/total;
	}
	check_dist(r);
	return r;
}

template<typename K,typename V>
V get(flat_map2<K,V> const& a,auto const& k,auto const& otherwise){
	auto f=a.find(k);
	if(f==a.end()){
		return otherwise;
	}
	return (*f).second;
}

template<typename K,typename V>
V get(flat_map2<K,V> const& a,auto const& k){
	auto f=a.find(k);
	assert(f!=a.end());
	return (*f).second;
}

template<typename K,typename V>
V get(std::map<K,V> const& a,auto const& k){
	auto f=a.find(k);
	assert(f!=a.end());
	return f->second;
}

template<typename K,typename V>
V get(std::map<K,V> const& a,auto const& k,auto const& otherwise){
	auto f=a.find(k);
	if(f==a.end()){
		return otherwise;
	}
	return f->second;
}

Team_dist weighted_average(Team_dist const& a,Team_dist const& b,double weight){
	Team_dist r;
	for(auto k:to_set(keys(a))|to_set(keys(b))){
		auto v1=get(a,k,0);
		auto v2=get(b,k,0);
		r[k]=v1*weight+v2*(1-weight);
	}
	return r;
}

template<typename K,typename V,typename K2>
auto mapf(flat_map2<K,V> const& a,std::vector<K2> const& b){
	return mapf([&](auto const& x){ return get(a,x); },b);
}

Team_dist smooth_by(int n,Team_dist const& a){
	check_dist(a);
	Team_dist r;            
	auto ks=keys(a);
	for(auto k:range(min(ks)-n,max(ks)+n+1)){
		//auto items=mapf(a,range(k-n,k+n+1));
		auto items=mapf([&](auto x){ return get(a,x,0); },range(k-n,k+n+1));

		r[k]=mean(items);
	}
	check_dist(r);
	return r;
}

auto smooth_by(size_t,auto const& a){
	nyi
	return a;
}

bool normal_event(Event_Simple const& a){
	using enum tba::Event_type;
	switch(a.event_type){
		case CMP_FINALS:
		case OFFSEASON:
		case PRESEASON:
		case FOC:
		case REMOTE:
			return 0;
		case REGIONAL:
		case CMP_DIVISION:
		case DISTRICT:
		case DISTRICT_CMP:
		case DISTRICT_CMP_DIVISION:
			return 1;
		default:
			PRINT(a.event_type);
			assert(0);
	}
}

auto normal_events(TBA_fetcher &f,Year year){
	auto found=filter(normal_event,events_simple(f,year));
	return mapf([](auto x){ return x.key; },found);
}

std::vector<Year> years(){
	return range(Year(1992),Year(2027));
}

std::map<District_abbreviation,std::set<Year>> normal_district_years(TBA_fetcher &f){
	const auto district_names=[&](){
		std::set<tba::District_abbreviation> r;
		for(auto year:years()){
			for(auto elem:districts(f,year)){
				r|=elem.abbreviation;
			}
		}
		return r;
	}();

	std::map<District_abbreviation,std::set<Year>> r;
	for(auto d:district_names){
		auto x=tba::dcmp_history(f,d);
		auto years=to_set(::mapf([](auto x){ return x.event.year; },x));
		years-=Year(2020);
		r[d]=years;
	}
	return r;
}

Skill_estimates calc_skill_opr(TBA_fetcher& f,tba::District_key const& district){
	//3% baseline; 10points of smoothing
	//download all of the OPR data for all of the events ever
	//make a list of (team,year)->(list(OPR),pre-dcmp points)
	//baseline for each of the items
	
	auto district_years=normal_district_years(f);

	//figure out OPR for each team/year

	using OPR=double;

	const auto oprs=[&](){
		std::map<pair<Year,Team>,OPR> r;
		for(auto year:years()){
			map<Team,vector<double>> by_team;
			//for(auto event:events_keys(f,year)){
			for(auto event:normal_events(f,year)){
				auto e=tba::event_oprs(f,event);
				if(!e) continue;
				if(!e->oprs) continue;
				for(auto [k,v]:*(e->oprs)){
					by_team[k]|=v;
				}
			}
			auto this_year=MAP_VALUES(max,by_team);
			for(auto [k,v]:this_year){
				r[make_pair(year,k)]=v;
			}
		}
		return r;
	}();

	static constexpr auto RANK_GRANULARITY=2000;
	//technically this could be as big as you want as long as it doesn't overflow and you're OK with 
	//sitting around and waiting for things.

	auto rank_team_set=[=](auto& r,Year year,std::vector<Team> teams){
		vector<double> oprs_here=nonempty(mapf(
			[=](auto team)->optional<OPR>{
				auto f=oprs.find(make_pair(year,team));
				if(f==oprs.end()){
					return std::nullopt;
				}
				return f->second;
			},
			teams
		));
		std::sort(oprs_here.begin(),oprs_here.end());
		for(auto team:teams){
			auto p=make_pair(year,team);
			auto f=oprs.find(p);
			if(f==oprs.end()){
				continue;
			}
			auto it=std::lower_bound(oprs_here.begin(),oprs_here.end(),f->second);
			auto index=it-oprs_here.begin();
			auto rank=index*RANK_GRANULARITY/oprs_here.size();
			r[p]=rank;
		}
	};

	//put in the OPR ranking within the district here...
	auto opr_ranks=[&](){
		std::map<pair<Year,Team>,int> r;
		for(auto [k,v]:district_years){
			for(auto year:v){
				District_key key(::as_string(year)+k);
				auto d=district_rankings(f,key);
				if(!d) continue;
				const auto teams=mapf([](auto x){ return x.team_key; },*d);
				rank_team_set(r,year,teams);
			}
		}
		return r;
	}();

	const auto results=[&](){ //pre-dcmp points
		std::map<pair<Year,Team>,Point> r;
		for(auto [k,v]:district_years){
			for(auto year:v){
				District_key key(::as_string(year)+k);
				auto d=tba::district_rankings(f,key);
				if(!d) continue;
				for(auto a:*d){
					auto district_events=take(2,a.event_points);
					int n=a.rookie_bonus+sum(mapf([](auto x){ return (int)x.total; },district_events));
					r[make_pair(year,a.team_key)]=n;
				}
			}
		}
		return r;
	}();

	const auto pairs=[=](){ //pairs of measurement -> outcome
		std::map<int,std::multiset<Point>> r;
		for(auto [k,v]:opr_ranks){
			auto [year,team]=k;
			auto f=results.find(make_pair(year+1,team));
			if(f==results.end()){
				continue;
			}
			r[v]|=f->second;
		}
		return r;
	}();

	//not look at the baseline of what the distribution might look like if had no other data.
	const auto baseline=[=](){
		auto m=flatten(seconds(pairs));
		return cut_dist(smooth_by(80,to_dist(m)));
	}();
	
	const auto ranking_values=range(RANK_GRANULARITY+1);

	auto make_dist=[=](auto rank){
		static constexpr size_t MIN_SAMPLE_SIZE=100;
		static constexpr size_t MAX_WIDTH=2000;

		std::multiset<Point> found;
		size_t i;
		for(i=0;i<MAX_WIDTH && found.size()<MIN_SAMPLE_SIZE;i++){
			found|=get(pairs,rank+i,std::multiset<Point>{});
			found|=get(pairs,(int)rank-(int)i,std::multiset<Point>{});
		}
		assert(i<MAX_WIDTH);
		return found;
	};

	using Rank=int;

	auto smoothed=[=](){
		map<Rank,std::multiset<Point>> r;

		for(auto rank:ranking_values){
			r[rank]=make_dist(rank);
		}
		return r;
	}();

	auto model_core=[=](){
		std::map<Rank,Team_dist> r;
		for(auto [k,v]:smoothed){
			static constexpr int SMOOTHING_DISTANCE=10; //This was found empirically.
			static constexpr double BASELINE_WEIGHT=.03;//This was also found empirically.
			auto d1=smooth_by(SMOOTHING_DISTANCE,to_dist(v));
			r[k]=cut_dist(weighted_average(baseline,d1,BASELINE_WEIGHT));
			//r[k]=to_dist(v);
		}
		return r;
	}();

	//Now look at the district that we actually got asked about.
	const auto last_year=year(district)-1;
	std::map<Team_key,Team_dist> pre_dcmp;
	auto teams=district_teams_keys(f,district);

	auto f1=filter([=](auto x){ return opr_ranks.find(make_pair(last_year,x))==opr_ranks.end(); },teams);
	if(f1.size()>10){
		rank_team_set(opr_ranks,last_year,teams);

	}
	auto f2=filter([=](auto x){ return opr_ranks.find(make_pair(last_year,x))==opr_ranks.end(); },teams);
	PRINT(f2);

	for(auto team:district_teams_keys(f,district)){
		auto p=make_pair(last_year,team);
		auto f=opr_ranks.find(p);
		if(f!=opr_ranks.end()){
			auto indicator=f->second;
			pre_dcmp[team]=model_core[indicator];
			continue;
		}

		//auto f2=oprs.find(p);
		//if(f2==oprs.end()){
		pre_dcmp[team]=baseline;
		//}
		//PRINT(*f2);
		//nyi
	}

	auto orig=calc_skill(f,district);

	{
		//Make it so that the at_dcmp distributions go all the way to 0.
		auto worst_known=min(keys(orig.at_dcmp));
		for(auto i:range(worst_known)){
			orig.at_dcmp[i]=orig.at_dcmp[worst_known];
		}
	}

	return Skill_estimates(pre_dcmp,orig.at_dcmp,orig.second_event);
}
