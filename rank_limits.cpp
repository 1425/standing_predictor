#include "rank_limits.h"
#include "rand.h"
#include "../tba/data.h"
#include "set_fixed.h"
#include "util.h"
#include "pick_points.h"
#include "../tba/tba.h"
#include "output.h"
#include "interval.h"
#include "rank_pts.h"
#include "tba.h"
#include "util.h"
#include "print_r.h"
#include "dates.h"
#include "multiset_flat.h"
#include "set_limited.h"
#include "multiset_fixed.h"
#include<type_traits>
#include<bitset>
#include "map_small_int.h"
#include "map_fixed.h"
#include "map_auto.h"

template<typename K,typename V>
auto dict(std::vector<std::pair<K,V>> const& a){
	std::map<K,V> r;
	for(auto [k,v]:a){
		r[k]=v;
	}
	return r;
}

template<typename A,typename B,size_t N>
std::array<A,N>& operator+=(std::array<A,N> &a,std::array<B,N> const& b){
	for(auto i:range_st<N>()){
		a[i]+=b[i];
	}
	return a;
}

using namespace std;

template<typename A,typename B>
auto teams(std::pair<A,B> const& p){
	auto t=teams(p.first);
	//t|=teams(p.second);
	return t;
}

using Team_alias=Int_limited<0,250>;

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

/*template<typename V>
std::set<Team_alias> teams(map_small_int<Team_alias,V> const& a){
	return keys(a);
}*/

template<typename V>
std::set<Team_alias> teams(map_fixed<Team_alias,V> const& a){
	return keys(a);
}

/*template<typename K,typename V>
auto teams(map_fixed<K,V> const& a){
	return keys(a);
}*/

template<size_t N>
set_limited<tba::Team_key,N> teams(set_limited<tba::Team_key,N> const& a){
	return a;
}

//using Team=tba::Team_key;

template<typename Team>
using Alliance=set_limited<Team,3>;

//using Match=std::array<Alliance,2>;//all teams contained should be unique

template<typename Team>
class Match{
	using Data=std::array<Alliance<Team>,2>;
	Data data;

	public:

	Match(Data a):data(a){
		assert( (a[0]&a[1]).empty());
	}

	auto const& get()const{
		return data;
	}

	//operator Data()const;

	using const_iterator=Data::const_iterator;

	const_iterator begin()const{
		return data.begin();
	}

	const_iterator end()const{
		return data.end();
	}

	Alliance<Team> const& operator[](size_t i){
		assert(i<2);
		return data[i];
	}
};

template<typename Team>
std::ostream& operator<<(std::ostream& o,Match<Team> const& a){
	return o<<a.get();
}

template<typename Team>
Match<Team> rand(Match<Team> const*);

template<typename Team,typename T,size_t N>
auto zip(Match<Team> const& a,std::array<T,N> const& b){
	return zip(a.get(),b);
}

template<typename Team>
auto enumerate_from(size_t i,Match<Team> const& a){
	return enumerate_from(i,a.get());
}

template<typename Func,typename Team>
auto mapf(Func f,Match<Team> const& a){
	return mapf(f,a.get());
}

template<typename Team>
std::set<Team> teams(Match<Team> const& a){
	return to_set(teams(a.get()));
}

template<typename Team>
using Schedule=std::vector<Match<Team>>;

using RP=Int_limited<0,200>;

template<typename Team>
using Standings=map_auto<Team,RP>;

template<typename Team>
auto teams(Standings<Team> const& a){
	return keys(a);
}

#define RANKING_MATCH_STATUS(X)\
	X(Standings<Team>,standings)\
	X(Schedule<Team>,schedule)

template<typename Team>
struct Ranking_match_status{
	RANKING_MATCH_STATUS(INST)

	void fill_standings(){
		for(auto team:teams(schedule)){
			auto f=standings.find(team);
			if(f==standings.end()){
				standings[team]=0;
			}
		}
	}
};

template<typename Team>
std::ostream& operator<<(std::ostream& o,Ranking_match_status<Team> const& a){
	o<<"Ranking_match_status( ";
	#define X(A,B) o<<""#B<<":"<<a.B<<" ";
	RANKING_MATCH_STATUS(X)
	#undef X
	return o<<")";
}

template<typename Team>
void print_r(int n,Ranking_match_status<Team> const& a){
	indent(n);
	cout<<"Ranking_match_status\n";
	n++;
	#define X(A,B) indent(n); cout<<""#B<<"\n"; print_r(n+1,a.B);
	RANKING_MATCH_STATUS(X)
	#undef X
}

template<typename Team>
std::set<Team> teams(Ranking_match_status<Team> const& a){
	auto t1=teams(a.standings);
	auto t2=teams(a.schedule);
	assert( (t2-t1).empty());
	return t1;
}

/*RP rp(tba::Ranking const& a){
	//this is obviously incomplete because it doesn't know about any bonus RP.
	//and probably have to look at the match details to see that...
	print_r(a);
	nyi
}*/
RP rp(int);

template<typename T>
RP rp(T const& a){
	print_r(a);
	nyi
}

using Bonus_rp=Int_limited<0,3>;//depends on year

RP rp(tba::Match_Score_Breakdown_2025_Alliance const& a){
	return a.rp;
}

RP rp(tba::Match_Score_Breakdown_2024_Alliance const& a){
	return a.rp;
}

RP rp(tba::Match_Score_Breakdown_2023_Alliance const& a){
	return a.rp;
}

RP rp(tba::Match_Score_Breakdown_2022_Alliance const& a){
	return a.rp;
}

RP rp(tba::Match_Score_Breakdown_2020_Alliance const& a){
	return a.rp;
}

auto rp(tba::Match_Score_Breakdown_2017_Alliance const& a){
	//warning: this doesn't include the win/tie pts!
	Bonus_rp r=0;
	if(a.kPaRankingPointArchieved){
		//sic ,int
	}
	if(a.rotorRankingPointAchieved){
		r++;
	}
	return r;
}

auto rp(tba::Match_Score_Breakdown_2014_Alliance const&){
	//no bonus RP and not w/l/t info in here!
	return Bonus_rp(0);
}

auto rp(tba::Match_Score_Breakdown_2016_Alliance const& a){
	//warning: no w/l/t in here!
	Bonus_rp r=0;
	if(a.teleopTowerCaptured){
		r++;
	}
	if(a.teleopDefensesBreached){
		r++;
	}
	return r;
}

RP rp(tba::Match_Score_Breakdown_2015_Alliance const&){
	return 0;
}

#define X(NAME) auto rp(tba::Match_Score_Breakdown_##NAME const& a){\
	using T=decltype(rp(a.red));\
	return std::array<T,2>{{rp(a.red),rp(a.blue)}};\
}
X(2026)
X(2025)
X(2024)
X(2023)
X(2022)
X(2020)
X(2017)
X(2016)
X(2015)
X(2014)
#undef X

using RP_count=std::variant<std::array<RP,2>,std::array<Bonus_rp,2>>;

template<typename... Ts>
auto rp(std::variant<Ts...> const& a){
	return std::visit([](auto const& x){ return RP_count(rp(x)); },a);
}

template<typename T>
auto rp(std::optional<T> const& a){
	using N=decltype(rp(*a));
	using R=std::optional<N>;
	if(!a){
		return R(std::nullopt);
	}
	return R(rp(*a));
}

optional<array<RP,2>> rp(tba::Match const& a){
	//print_r(a);
	auto r1=rp(a.score_breakdown);
	if(!r1){
		return std::nullopt;
	}
	auto r=*r1;
	if(std::holds_alternative<std::array<RP,2>>(r)){
		return std::get<std::array<RP,2>>(r);
	}
	array<RP,2> total;
	auto bonus=std::get<std::array<Bonus_rp,2>>(r);
	total+=bonus;

	switch(a.winning_alliance){
		case tba::Winning_alliance::red:
			total[0]+=2;
			break;
		case tba::Winning_alliance::blue:
			total[1]+=2;
			break;
		case tba::Winning_alliance::NONE:
			//assuming that this means a tie.
			total[0]++;
			total[1]++;
			break;
		default:
			assert(0);
	}
	return total;
}

Ranking_match_status<tba::Team_key> get(TBA_fetcher &f,tba::Event_key const& event){
	//Note that at the moment this doesn't have incomplete events to look at
	//so it's only half tested.

	/* Also, it seems like there are quite a few matches where it doesn't understand 
	 * what is going on already - so that would be good to work out.
	 *
	 * */

	Ranking_match_status<tba::Team_key> r;
	for(auto match:tba::event_matches(f,event)){
		auto f=[](auto x){
			auto found=to_set(x.team_keys)-to_set(x.surrogate_team_keys);
			if(found.size()>3){
				//this is invalid; skip the match
				//might want to note that this happened somehow
				//does actually occur in the data.  Specifically at
				//2024gaalb
			}
			return Alliance(take(3,found));
		};
		//match.alliances.red.team_keys/surrogate_team_keys
		auto m1=f(match.alliances.red);
		auto m2=f(match.alliances.blue);
		if( (m1&m2).size() ){
			//invalid match; skipping
			//might want to note this to caller or something.
			continue;
		}
		Match<tba::Team_key> match1({m1,m2});
		//r.schedule|=match1;

		auto rp_totals_m=rp(match);
		if(!rp_totals_m){
			r.schedule|=match1;
		}else{
			auto rp_totals=*rp_totals_m;
			if(sum(rp_totals)==0){
				r.schedule|=match1;
			}else{
				for(auto [teams,pts]:zip(match1,rp_totals)){
					for(auto team:teams){
						r.standings[team]+=pts;
					}
				}
			}
		}
	}
	r.fill_standings();
	return r;
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

static const RP MAX_RP_PER_MATCH=5;

template<typename Team>
Ranking_match_status<Team> assume_wins(Ranking_match_status<Team> a,Team const& team){
	/*for(auto [match_index,match]:enumerate(a.schedule)){
		for(auto [i,alliance]:enumerate(match)){
			if(contains(alliance,team)){
				Match_result match_result;
				if(i==0){
					match_result={MAX_RP_PER_MATCH,0};
				}else{
					match_result={0,MAX_RP_PER_MATCH};
				}
				auto b=apply(a,match_index,match_result);
				return assume_wins(b,team);
			}
		}
	}
	return a;*/
	Schedule<Team> &s=a.schedule;
	auto out=s.begin();
	for(auto match:a.schedule){
		if(match[0].count(team)){
			for(auto team:match[0]){
				a.standings[team]+=MAX_RP_PER_MATCH;
			}
		}else if(match[1].count(team)){
			for(auto team:match[1]){
				a.standings[team]+=MAX_RP_PER_MATCH;
			}
		}else{
			*out=match;
			++out;
		}
	}

	//now remove the last items beyond what's been written
	s.erase(out,s.end());

	return a;
}

template<typename Team>
Ranking_match_status<Team> assume_losses(Ranking_match_status<Team> a,Team const& team){
	/*for(auto [match_index,match]:enumerate(a.schedule)){
		for(auto [i,alliance]:enumerate(match)){
			if(contains(alliance,team)){
				Match_result match_result;
				if(i==1){
					match_result={MAX_RP_PER_MATCH,0};
				}else{
					match_result={0,MAX_RP_PER_MATCH};
				}
				auto b=apply(a,match_index,match_result);
				return assume_losses(b,team);
			}
		}
	}*/
	Schedule<Team> &s=a.schedule;
	auto out=s.begin();
	for(auto match:a.schedule){
		if(match[0].count(team)){
			for(auto team:match[1]){
				a.standings[team]+=MAX_RP_PER_MATCH;
			}
		}else if(match[1].count(team)){
			for(auto team:match[0]){
				a.standings[team]+=MAX_RP_PER_MATCH;
			}
		}else{
			*out=match;
			++out;
		}
	}

	//now remove the last items beyond what's been written
	s.erase(out,s.end());

	return a;
}

template<typename Team>
map_auto<Team,Interval<Rank>> rank_limits_basic(Ranking_match_status<Team> const& status);

//std::map<Team,Interval<Point>> rank_limits_m(std::map<Team,RP> const& existing_standings,Schedule const& schedule){
template<typename Team>
map_auto<Team,Interval<Rank>> rank_limits_m(Ranking_match_status<Team> const& status){
	//this will attempt to narrow down based on assuming the results for that team

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

	auto [existing_standings,schedule]=status;

	//going to start by assuming that there is only win or loss, no ties
	//and no bonus RP

	//const auto event_size=existing_standings.size();

	//PRINT(event_size);
	auto matches_left=[=](){
		map<Team,unsigned> r;
		for(auto match:schedule){
			for(auto alliance:match){
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
		for(auto [team,rp]:existing_standings){
			r[team]=Interval<RP>(rp,rp+MAX_RP_PER_MATCH*matches_left[team]);
		}
		return r;
	}();

	//cout<<"RP ranges:\n";
	//print_r(rp_ranges);

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

	//cout<<"Rank ranges:\n";
	//print_r(rank_ranges);

	/*const auto point_ranges=map_values(
		[=](auto x){
			return apply_monotonic(
				[=](auto y){ return Point(rank_pts(event_size,y)); },
				x
			);
		},
		rank_ranges
	);

	return point_ranges;*/
}

/*template<typename Team>
using Rank_range=map_auto<Team,Interval<Rank>>;

template<typename Team>
using Point_range=map_auto<Team,Interval<Point>>;

#define RANK_RESULTS(X)\
	X(Rank_range<Team>,ranks)\
	X(Point_range<Team>,points)\
	X(unsigned,unclaimed_points)

template<typename Team>
struct Rank_results{
	RANK_RESULTS(INST)
};

template<typename Team>
std::ostream& operator<<(std::ostream& o,Rank_results<Team> const& a){
	o<<"Rank_result( ";
	#define X(A,B) o<<""#B<<":"<<a.B<<" ";
	RANK_RESULTS(X)
	#undef X
	return o<<")";
}

template<typename Team>
void print_r(int n,Rank_results<Team> const& a){
	indent(n);
	cout<<"Rank_results\n";
	n++;
	#define X(A,B) indent(n); cout<<""#B<<"\n"; print_r(n+1,a.B);
	RANK_RESULTS(X)
	#undef X
}*/

auto teams(std::set<tba::Team_key> const& a){
	return a;
}

auto teams(std::set<Team_alias> const& a){
	return a;
}

template<long long MIN,long long MAX>
auto teams(Interval<Int_limited<MIN,MAX>> const& a){
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

	Team_alias convert(tba::Team_key a){
		return to_alias(a);
	}

	tba::Team_key convert(Team_alias a){
		return to_key(a);
	}

	template<typename Team>
	Alliance<Team_alias> convert(Alliance<Team> a){
		return MAP(convert,a);
	}

	template<typename Team>
	Match<Team_alias> convert(Match<Team> const& a){
		return mapf([&](auto x){ return convert(x); },a);
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
	Team_namer namer;
	auto g=namer.convert(get(f,event));
	Rank_results<Team_alias> r;
	r.ranks=rank_limits_m(g);
	auto event_size=g.standings.size();
	r.points=points(r.ranks,event_size);
	auto total_points=rank_pts(event_size);
	size_t min_pts_taken=sum(MAP(min,values(r.points)));
	assert(min_pts_taken<=total_points);
	r.unclaimed_points=total_points-min_pts_taken;
	PRINT(namer);
	return namer.convert(r);
}

void rank_limits_demo(TBA_fetcher &f){
	/*Things that would be interesting to know from the outside:
	 *1) Get current state: event key -> status item
	 2) Given current state:
	 	-what are the ranges of rank and of points for each team
		-how many points are unclaimed
	 * */
	//rank_limits(f,tba::Event_key("2025orwil"));
	//for(auto event:reversed(take(50,all_events(f)))){
	//for(auto event:take(550,reversed(all_events(f)))){

	for(auto event:all_events(f)){
		PRINT(event.key);
		auto r=rank_limits(f,event.key);
		print_r(r);
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
	Ranking_match_status<Team> status;
	status.standings=existing_standings;
	status.schedule=schedule;
	status.fill_standings();

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

