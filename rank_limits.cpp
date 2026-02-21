#include "rank_limits.h"
#include "rand.h"
#include "../tba/data.h"
#include "set_fixed.h"
#include "util.h"
#include "pick_points.h"
#include "../tba/tba.h"
#include "interval.h"
#include "rank_pts.h"
#include "tba.h"
#include "print_r.h"
#include "multiset_flat.h"
#include "set_limited.h"
#include "multiset_fixed.h"
#include<type_traits>
#include<bitset>
#include "map_small_int.h"
#include "map_fixed.h"
#include "map_auto.h"
#include "declines.h"
#include "rp.h"
#include "rank_pts.h"
#include "ranking_match_status.h"

using namespace std;

template<typename A,typename B>
auto teams(std::pair<A,B> const& p){
	auto t=teams(p.first);
	//t|=teams(p.second);
	return t;
}

using Team_alias=Int_limited<0,MAX_TEAMS_PER_EVENT>;

template<typename V>
auto teams(map_small_int<Team_alias,V> const& a){
	/*using E=decltype(teams(*a.begin()));
	std::set<E> r;
	for(auto p:a){
		r|=teams(p);
	}
	return r;*/
	return keys(a);
}

template<typename V>
std::set<Team_alias> teams(map_fixed<Team_alias,V> const& a){
	return keys(a);
}




std::set<tba::Team_key> teams(std::set<tba::Team_key> const& a){
	return a;
}

auto teams(std::set<Team_alias> const& a){
	return a;
}

template<long long MIN,long long MAX>
auto teams(Interval<Int_limited<MIN,MAX>> const&){
	return std::set<tba::Team_key>{};
}

auto teams(std::pair<Team_alias,Rank> a){
	return std::set<Team_alias>{a.first};
}

template<typename K,typename V>
auto teams(std::map<K,V> const& a){
	//return teams(keys(a))|teams(values(a));
	return or_all(MAP(teams,a));
}

template<typename T>
std::set<Team_alias> teams(std::map<Team_alias,T> const& a){
	return keys(a);
}

template<typename A,typename B>
auto teams(map_auto<A,B> const& a){
	return teams(a.get());
}

optional<map<tba::Team_key,Rank>> listed_ranks(TBA_fetcher &f,tba::Event_key const& event){
	//should try to compare the results of this with the results of get().
	auto e1=event_rankings(f,event);
	if(!e1) return std::nullopt;
	auto const& e=*e1;
	auto const& r1=e.rankings;
	if(!r1) return std::nullopt;
	auto const& r=*r1;
	if(r.empty()) return std::nullopt;
	auto found=dict(
		mapf(
			[](auto x){
				assert(x.rank);
				Rank rank=*x.rank;
				return make_pair(x.team_key,rank);
			},
			r
		)
	);
	if(check_rank_limits(found)){
		//then the data is invalid
		//this is known to be the case for a number of offseason events with unusual ranking systems
		//and also for a bunch of the 2021ir... groups
		return std::nullopt;
	}
	return found;
}

using Match_result=std::array<RP,2>;

template<typename Team>
Ranking_match_status<Team> apply(Ranking_match_status<Team> a,int match_index,Match_result match_result){
	auto match=a.schedule.at(match_index);
	for(auto [alliance,alliance_result]:zip(match,match_result)){
		for(auto team:alliance){
			a.standings[team]+=alliance_result;
		}
	}
	a.schedule.erase(a.schedule.begin()+match_index);
	return a;
}

template<typename Team>
Ranking_match_status<Team> assume_wins(Ranking_match_status<Team> a,Team const& team){
	Schedule<Team> &s=a.schedule;
	auto out=s.begin();
	for(auto match:a.schedule){
		if(match[0].count(team)){
			for(auto team:match[0]){
				a.standings[team]+=max_rp_per_match(a.year);
			}
		}else if(match[1].count(team)){
			for(auto team:match[1]){
				a.standings[team]+=max_rp_per_match(a.year);
			}
		}else{
			*out=match;
			++out;
		}
	}

	//now remove the last items beyond what's been written
	s.erase(out,s.end());

	return std::move(a);
}

template<typename Team>
Ranking_match_status<Team> assume_losses(Ranking_match_status<Team> a,Team const& team){
	Schedule<Team> &s=a.schedule;
	auto out=s.begin();
	for(auto match:a.schedule){
		if(match[0].count(team)){
			for(auto team:match[1]){
				a.standings[team]+=max_rp_per_match(a.year);
			}
		}else if(match[1].count(team)){
			for(auto team:match[0]){
				a.standings[team]+=max_rp_per_match(a.year);
			}
		}else{
			*out=match;
			++out;
		}
	}

	//now remove the last items beyond what's been written
	s.erase(out,s.end());

	return std::move(a);
}

template<typename Team>
map_auto<Team,Interval<Rank>> rank_limits_basic(Ranking_match_status<Team> const& status);

//std::map<Team,Interval<Point>> rank_limits_m(std::map<Team,RP> const& existing_standings,Schedule const& schedule){
template<typename Team>
map_auto<Team,Interval<Rank>> rank_limits_m(Ranking_match_status<Team> const& status){
	//this will attempt to narrow down based on assuming the results for that team

	//This if is necessary for performance on the very-large ficticious events like 2021irhce
	//which claim >1700 teams.
	if(status.schedule.empty()){
		return rank_limits_basic(status);
	}

	map_auto<Team,Interval<Rank>> r;
	for(auto team:keys(status.standings)){
		//first, look for upper bound

		auto best=[=](){
			auto a=assume_wins(status,team);
			auto out=rank_limits_basic(a);
			auto interval=out[team];
			//cout<<"if win: "<<interval<<"\n";
			return min(interval);
		}();

		auto worst=[=](){
			auto a=assume_losses(status,team);
			auto out=rank_limits_basic(a);
			auto interval=out[team];
			//cout<<"if lose:"<<interval<<"\n\n";
			return max(interval);
		}();

		r[team]=Interval<Rank>{best,worst};
	}
	return r;
}

//std::map<Team,Interval<Point>> rank_limits(std::map<Team,RP> const& existing_standings,Schedule const& schedule){
template<typename Team>
map_auto<Team,Interval<Rank>> rank_limits_basic(Ranking_match_status<Team> const& status){
	/*if(status.standings.size()<10 || status.standings.size()>=80){
		//Not in the range of event size where normal district points are earned.
		std::map<Team,Interval<Rank>> r;
		for(auto team:teams(status)){
			r[team]=Interval<Rank>{1,status.standings.size()};
		}
		return r;
	}*/

	//auto [existing_standings,schedule]=status;

	//going to start by assuming that there is only win or loss, no ties
	//and no bonus RP

	//const auto event_size=existing_standings.size();

	//PRINT(event_size);
	auto matches_left=[&](){
		map_auto<Team,unsigned> r;//value here should be in the range 0-12
		for(auto const& match:status.schedule){
			for(auto const& alliance:match){
				for(auto team:alliance){
					r[team]++;
				}
			}
		}
		return r;
	}();

	//print_r(matches_left);
	/*for(auto k:keys(existing_standings)){
		cout<<k<<" "<<matches_left[k]<<"\n";
	}*/
	//PRINT(matches_left[Team("frc172")]);
	//pass 1: brute force all of the outcomes
	//pass 2: use intervals based on the number of matches scheduled, etc.
	// and figure out the number of teams that can be on either side of a given team theoretically
	//pass 3: integrate bonus RP
	//pass 4: try to make limits based on per-match results, so best case assumes that the team in question
	//  will get all the pts in its remaining matches, etc. and then the other team gets as few pts as possible given that
	// and the opposite for worst case
	//pass 5: try to find coherent match results without brute force.  Probably not terribly feasible.

	//for each team, find interval of RP
	const auto rp_ranges=[&](){
		map_auto<Team,Interval<RP>> r;
		for(auto [team,rp]:status.standings){
			r[team]=Interval<RP>(rp,rp+max_rp_per_match(status.year)*matches_left[team]);
		}
		return r;
	}();

	//cout<<"RP ranges:\n";
	//print_r(count(values(rp_ranges)));

	const auto rank_ranges=[=](){
		map_auto<Team,Interval<Rank>> r;
		for(auto const& [team,range]:rp_ranges){
			//std::multiset<Interval_compare> found;
			//multiset_flat<Interval_compare> found;
			multiset_fixed<Interval_compare> found;
			for(auto const& [team2,range2]:rp_ranges){
				if(team2==team) continue;
				found|=compare(range,range2);
			}
			//print_r(found
			Rank best_rank=1+found.count(Interval_compare::LESS);
			Rank worst_rank=1+
				found.count(Interval_compare::LESS)+
				found.count(Interval_compare::EQUAL)+
				found.count(INDETERMINATE);
			r[team]=Interval<Rank>(best_rank,worst_rank);
		}
		return r;
	}();

	return rank_ranges;
}

template<typename Team>
map_auto<Team,Interval<Point>> points(map_auto<Team,Interval<Rank>> const& ranks,size_t event_size){
	if(event_size<10 || event_size>=80){
		//Not in the range of event size where normal district points are earned.
		map_auto<Team,Interval<Point>> r;
		for(auto team:teams(ranks)){
			r[team]=0;
		}
		return r;
	}

	return map_values(
		[=](auto x){
			return apply_monotonic(
				[=](auto y){ return Point(rank_pts(event_size,y)); },
				x
			);
		},
		ranks
	);
}

class Team_namer{
	//Could alter this to have a template argument that is how many teams it needs to handle
	//And then make it so that will go up to the next integer size if there are too many teams.

	//Also, could make it so that there is not only the (int)->(string) via the vector
	//but also a map<string->int> for faster lookups.

	std::vector<tba::Team_key> data;

	public:

	auto get()const{
		return data;
	}

	Team_alias to_alias(tba::Team_key const& a){
		for(size_t i=0;i<data.size();i++){
			if(data[i]==a){
				return i;
			}
		}
		auto r=data.size();
		data|=a;
		return r;
	}

	tba::Team_key to_key(Team_alias a){
		assert(a<data.size());
		return data[a];
	}

	int convert(int)const;

	tba::Year convert(tba::Year a){
		return a;
	}

	static short convert(short a){
		return a;
	}

	static unsigned convert(unsigned a){
		return a;
	}

	template<long long MIN,long long MAX>
	Int_limited<MIN,MAX> convert(Int_limited<MIN,MAX> a){
		return a;
	}

	template<typename T>
	auto convert(Interval<T> const& a){
		return Interval{convert(a.min),convert(a.max)};
	}

	#define CONVERT(A,B) convert(a.B),

	Team_alias convert(tba::Team_key const& a){
		return to_alias(a);
	}

	tba::Team_key convert(Team_alias a){
		return to_key(a);
	}

	template<typename Team>
	Alliance<Team_alias> convert(Alliance<Team> const& a){
		return MAP(convert,a);
	}

	template<typename Team>
	Match<Team_alias> convert(Match<Team> const& a){
		return mapf([&](auto const& x){ return convert(x); },a);
	}

	template<typename A,typename B>
	auto convert(std::pair<A,B> const& p){
		return MAP(convert,p);
	}

	template<typename K,typename V>
	auto convert(std::map<K,V> const& a){
		//needs to convert back to a dictionary...
		return dict(MAP(convert,a));
	}

	template<typename K,typename V>
	auto convert(map_auto<K,V> const& a){
		return dict_auto(MAP(convert,a));
	}

	template<typename T>
	auto convert(std::vector<T> const& a){
		return MAP(convert,a);
	}

	template<typename T,size_t N>
	auto convert(vector_fixed<T,N> const& a){
		return MAP(convert,a);
	}

	Ranking_match_status<Team_alias> convert(Ranking_match_status<tba::Team_key> const& a){
		(void)a;
		return Ranking_match_status<Team_alias>{
			RANKING_MATCH_STATUS(CONVERT)
		};
	}

	Rank_results<tba::Team_key> convert(Rank_results<Team_alias> const& a){
		return Rank_results{
			RANK_RESULTS(CONVERT)
		};
	}
};

std::ostream& operator<<(std::ostream& o,Team_namer const& a){
	o<<"Team_namer("<<a.get()<<")";
	return o;
}

bool nonempty_alliances(TBA_fetcher &f,tba::Event_key const& event){
	auto e=tba::event_alliances(f,event);
	if(!e){
		return 0;
	}
	auto t=teams(*e);
	return !t.empty();
}

int rank_pts2(int event_size,int rank){
	if(event_size>=80 || event_size<10){
		return 0;
	}
	return rank_pts(event_size,rank);
}

Rank_results<tba::Team_key> rank_limits(TBA_fetcher &f,tba::Event_key const& event){
	
	/*auto g=get(f,event);
	Rank_results r;
	r.ranks=rank_limits_m(g);
	auto event_size=g.standings.size();
	r.points=points(r.ranks,event_size);
	auto total_points=rank_pts(event_size);
	size_t min_pts_taken=sum(MAP(min,values(r.points)));
	assert(min_pts_taken<=total_points);
	r.unclaimed_points=total_points-min_pts_taken;
	return r;*/

	auto info=ranking_match_status(f,event);

	if(info.schedule.empty() || nonempty_alliances(f,event) || playoffs_started(f,event)){
		auto a=listed_ranks(f,event);
		//the listed version is better if no matches are left because then you get all the tiebreakers

		//also, there are events that are finished that don't have the data written down for all of
		//the qual matches.  For example, 2016mdbet.
		if(a){
			Rank_results<tba::Team_key> r;
			for(auto [team,rank]:*a){
				r.ranks[team]=rank;
			}
			for(auto [team,rank]:*a){
				//PRINT(team);
				//PRINT(rank);
				//PRINT(r.ranks.size());
				r.points[team]=rank_pts2(r.ranks.size(),rank);
			}
			r.unclaimed_points=0;

			//pick up random teams that aren't actually listed.
			for(auto team:keys(info.standings)){
				auto f=r.ranks.find(team);
				if(f==r.ranks.end()){
					r.ranks[team]=r.ranks.size()+1;
					r.points[team]=0;
				}
			}

			return r;
		}

	}

	if(
		nonempty_alliances(f,event) || 
		playoffs_started(f,event) || 
		awards_done(f,event) ||
		event_timed_out(f,event)
	){
		Rank_results<tba::Team_key> r;
		auto teams=teams_keys(f,event);
		//arbitrarily assign ranks to the teams, but give them all 0 points.
		//This case is here for events like 2020mdow.
		for(auto [rank,team]:enumerate_from(1,teams)){
			r.ranks[team]=rank;
		}
		for(auto team:teams){
			r.points[team]=0;
		}
		r.unclaimed_points=0;
		return r;
	}

	Team_namer namer;
	auto g=namer.convert(info);
	Rank_results<Team_alias> r;
	r.ranks=rank_limits_m(g);
	auto event_size=g.standings.size();
	r.points=points(r.ranks,event_size);
	auto total_points=rank_pts(event_size);
	size_t min_pts_taken=sum(MAP(min,values(r.points)));
	assert(min_pts_taken<=total_points);
	r.unclaimed_points=total_points-min_pts_taken;
	return namer.convert(r);
}

template<typename Team,typename T>
bool valid_outcome(map_auto<Team,T> const& a,map_auto<Team,Interval<T>> const& b){
	if(keys(a)!=keys(b)){
		return 0;
	}
	for(auto k:keys(a)){
		if(!subset(a[k],b[k])){
			return 0;
		}
	}
	return 1;
}

std::optional<bool> rankings_consistent(TBA_fetcher &f,tba::Event_key const& event){
	auto a1=listed_ranks(f,event);
	if(!a1){
		return nullopt;
	}

	auto r=rank_limits(f,event);

	for(auto [k,v]:join(*a1,r.ranks)){
		auto [a,b]=v;
		if(a && b){
			if(!subset(*a,*b)){
				return 0;
			}
		}else{
			return 0;
		}
	}
	return 1;
}

bool normal_ranking_expected(tba::Event_type a){
	switch(a){
		case tba::Event_type::OFFSEASON:
		case tba::Event_type::PRESEASON:
		case tba::Event_type::CMP_FINALS:
		case tba::Event_type::FOC:
		case tba::Event_type::REMOTE:
			return 0;
		case tba::Event_type::REGIONAL:
		case tba::Event_type::CMP_DIVISION:
		case tba::Event_type::DISTRICT_CMP:
		case tba::Event_type::DISTRICT:
		case tba::Event_type::DISTRICT_CMP_DIVISION:
			return 1;
		default:
			PRINT(a);
			assert(0);
	}
}

bool normal_ranking_expected(tba::Event const& a){
	return normal_ranking_expected(a.event_type);
}

void debug(TBA_fetcher &f){
	auto found=filter(
		[&](auto x){
			return 1//year(x)>=2016 
				//&& x.event_type!=tba::Event_type::OFFSEASON
				//&& x.event_type!=tba::Event_type::PRESEASON
				//&& x.event_type!=tba::Event_type::CMP_FINALS
				&& normal_ranking_expected(x)
				&& !rankings_consistent(f,x.key);
		},
		events(f)
	);
	for(auto f1:found){
		auto l=listed_ranks(f,f1.key);
		if(!l){
			cout<<"No ranks at: "<<f1.key<<"\n";
			continue;
		}
		print_r(f1);
		cout<<"listed ranks:";
		print_r(l);

		auto status=ranking_match_status(f,f1.key);
		cout<<"from matches: ";
		print_r(status);
		return;
	}

}

void rank_limits_demo(TBA_fetcher &f){
	debug(f);
	return;


	//return test_2015(f);

	//print_r(ranking_match_status(f,tba::Event_key("2025orwil")));
	//rp_distribution(f);
	//return;

	/*Things that would be interesting to know from the outside:
	 *1) Get current state: event key -> status item
	 2) Given current state:
	 	-what are the ranges of rank and of points for each team
		-how many points are unclaimed
	 * */
	//rank_limits(f,tba::Event_key("2025orwil"));
	//for(auto event:reversed(take(50,events(f)))){
	//for(auto event:take(550,reversed(events(f)))){

	#if 0
	auto event_keys=mapf(
		[](auto x){ return x.key; },
		filter(
			[](auto const& x){
				//skip because some offseasons are known to have weird ranking systems.
				return x.event_type!=tba::Event_type::OFFSEASON;
			},
			events(f)
		)
	);
	auto g=group(
		[&](auto x){
			auto error=check_rank_limits(listed_ranks(f,x));
			return std::make_pair(year(x),error);
		},
		event_keys
	);
	//print_r(g);
	for(auto [k,v]:g){
		cout<<k<<"\t"<<v.size()<<"\t"<<take(25,v)<<"\n";
	}
	return;
	#endif
	//	auto a1=listed_ranks(f,event.key);
	//	check_

	auto interesting_events=filter(
		[](auto x){ return x.event_type==tba::Event_type::DISTRICT; },
		events(f)
	);

	for(auto [g,v]:group([](auto x){ return x.year; },interesting_events)){
		auto m=mapf(
			[&](auto x){
				return rankings_consistent(f,x.key);
			},
			v
		);
		PRINT(g);
		PRINT(count(m));
	}
	return;

	for(auto event:events(f)){
		if(event.event_type==tba::Event_type::CMP_FINALS){
			//then rankings are not meaningful.
			continue;
		}
		if(event.event_type==tba::Event_type::OFFSEASON){
			//then rankings are not meaningful.
			continue;
		}

		//print_r(event);

		auto r=rank_limits(f,event.key);
		//print_r(r);
		auto a1=listed_ranks(f,event.key);
		if(a1){
			auto const& a=to_map_auto(*a1);

			auto b=r.ranks;

			if(check_rank_limits(b)){
				nyi
				continue;
			}
			if(check_rank_limits(a)){
				nyi
				continue;
			}
	
			if(valid_outcome(a,b)){
				//cout<<"\tok\n";
			}else{
				PRINT(event.key);
				//print_r(a);
				//print_r(b);
				//diff(a,b);
				auto g=group([&](auto k){ return subset(a[k],b[k]); },keys(a));
				print_r(g);

				size_t ok=0;
				for(auto [k,v]:join(a,b)){
					auto [a1,b1]=v;
					assert(a1 || b1);
					if(!a1 || !b1){
						if(a1){
							cout<<"listed only:"<<k<<" "<<a1<<"\n";
						}else{
							cout<<"matches only:"<<k<<" "<<b1<<"\n";
						}
					}else{
						if(!subset(*a1,*b1)){
							cout<<k<<"\t"<<v<<"\n";
						}else{
							ok++;
						}
					}
				}
				PRINT(ok);
				/*for(auto k:keys(a)){
					assert(subset(a[k],b[k]));
				}*/
				//nyi
			}
		}

		/*
		auto g=get(f,event.key);
		//print_r(g);
		g.schedule=take(5,g.schedule);
		//print_r(g);
		auto r=rank_limits_m(g);
		//print_r(r);
		*/
	}
	return;

	using Team=tba::Team_key;
	std::map<Team,RP> existing_standings;//=rand((std::map<Team,RP>*)0);
	for(auto _:range(10)){
		auto team=rand((Team*)0);
		existing_standings[team]=rand()%10;
	}

	Schedule<Team> schedule=rand((Schedule<Team>*)0);
	while(schedule.size()<10){
		auto teams=keys(existing_standings);
		assert(teams.size()>=6);
		auto a1=[=]()->Alliance<Team>{
			auto picked=choose(3,teams);
			return picked;
			//auto c=to_vec(choose(3,teams));
			//return mapf([=](auto i){ return c[i]; },range_st<3>());
		}();
		auto a2=choose(3,teams-a1);

		schedule|=Match<Team>({a1,a2});
	}
	//PRINT(schedule.size());
	Ranking_match_status<Team> status(tba::Year(2016));
	status.standings=existing_standings;
	status.schedule=schedule;
	nyi//status.fill_standings();

	auto r=rank_limits_basic(status);
	auto r2=rank_limits_m(status);

	for(auto k:keys(r)|keys(r2)){
		auto a=r[k];
		auto b=r2[k];
		cout<<k<<"\t"<<a<<"\t"<<b<<"\t"<<subset(a,b)<<"\t"<<subset(b,a)<<"\n";
	}

	//for each team, look at the results in r and r2
	//and should find that the results for r2 are a subset of r.
	nyi
	//print_r(r);
}

