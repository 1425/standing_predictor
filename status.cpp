#include "status.h"
#include<iomanip>
#include "outline.h"
#include<optional>
#include<set>
#include "../tba/tba.h"
#include "map.h"
#include "tba.h"
#include "util.h"
#include "set.h"
#include "print_r.h"
#include "multiset_flat.h"
#include "event.h"

template<typename K,typename V>
std::map<V,std::vector<K>> invert(std::map<K,V> const& a){
	std::map<V,std::vector<K>> r;
	for(auto [k,v]:a){
		r[v]|=k;
	}
	return r;
}

template<typename T>
std::set<T>& operator-=(std::set<T> &a,std::set<T> const& b){
	return a=a-b;
}

template<typename T>
std::set<T>& operator|=(std::set<T>& a,std::vector<T> const& b){
	for(auto const& elem:b){
		a|=elem;
	}
	return a;
}

template<typename T>
std::set<T> operator|(std::set<T> a,std::vector<T> const& b){
	return a|=b;
}

template<typename T>
std::set<T> or_all(std::vector<std::vector<T>> const& a){
	std::set<T> r;
	for(auto elem:a){
		r|=elem;
	}
	return r;
}

template<typename T>
auto keys(T const& t){
	return mapf([](auto x){ return x.key; },t);
}

template<typename T>
std::vector<T> operator|(std::vector<T>,std::optional<T>);

template<size_t N>
std::array<size_t,N> range_st(){
	std::array<size_t,N> r;
	for(size_t i=0;i<N;i++){
		r[i]=i;
	}
	return r;
}

template<typename T>
std::vector<T> cdr(std::vector<T> a){
	if(a.empty()) return a;
	return std::vector<T>{a.begin()+1,a.end()};
}

template<typename Func,typename T,size_t N>
auto mapf(Func f,std::array<T,N> const& a){
	using E=decltype(f(*begin(a)));
	std::array<E,N> r;
	for(auto i:range_st<N>()){
		r[i]=f(a[i]);
	}
	return r;
}

template<typename T,size_t N>
std::array<T,N> as_array(std::vector<T> const& a){
	assert(a.size()==N);
	return mapf([&](auto x){ return a[x]; },range_st<N>());
}

template<typename T,size_t N>
std::ostream& operator<<(std::ostream& o,std::array<T,N> const& a){
	o<<"[ ";
	for(auto elem:a){
		o<<elem<<" ";
	}
	return o<<"]";
}

using namespace std;

//Stuff to deal w/ probability distributions

template<template<typename,typename> typename MAP>
double sum(MAP<int,double> const& p){
	double r=0;
	for(auto [k,v]:p){
		r+=k*v;
	}
	return r;
}

template<template<typename,typename> typename MAP>
double mean(MAP<int,double> const& p){
	return sum(p)/sum(values(p));
}

template<template<typename,typename> typename MAP>
std::array<int,3> quartiles(MAP<int,double> const& p){
	std::vector<int> r;
	double total=0;
	std::vector<double> thresholds{.25,.5,.75};
	//std::vector<double> thresholds{.125,.25,.5,.75,.875};
	for(auto [k,v]:p){
		total+=v;
		while(thresholds.size() && total>=thresholds[0]){
			r|=k;
			thresholds=cdr(thresholds);
		}
		if(thresholds.empty()){
			break;
		}
	}
	return as_array<int,3>(r);
}

template<template<typename,typename> typename MAP>
void show_dist(MAP<int,double> const& a){
	if(a.empty()) return;
	cout<<"min:"<<min(keys(a))<<"\n";
	cout<<"max:"<<max(keys(a))<<"\n";
	cout<<"mean:"<<mean(a)<<"\n";
	cout<<"q:"<<quartiles(a)<<"\n";
}

template<typename A,typename B>
void compare(A a,B b){
	//this is designed to compare two different probability distributions.
	std::cout<<std::setprecision(2);
	for(auto k:keys(a)|keys(b)){
		cout<<k<<"\t"<<a[k]<<"\t"<<b[k]<<"\t"<<a[k]-b[k]<<"\n";
	}
}

/*
 * Per FIRST's website in March 2024:
 * 
Pre-Qualifying Teams

The following teams will be pre-qualified for the 2024 FIRST Championship based on their performance at the 2023 FIRST Championship:

    2023 FIRST Championship Winners: 1323, 2609, 4096, and 4414
    2023 FIRST Impact Award Finalists: 118, 3284, 5665, 5985, and 6865
    2023 Engineering Inspiration Award Winners: 4, 1156, 1676, 2096, 2486, 3937, 5166, and 7565
    2023 FIRST Impact Award Winner: 321

The FIRST Impact Award, formerly the Chairman’s Award, is the most prestigious award in the program and is the only way FIRST Robotics Competition teams can enter the Hall of Fame. To honor this achievement, Hall of Fame teams that earned their Chairman's / FIRST Impact Award at the last ten FIRST Championships prior to 2023 are also pre-qualified teams. These teams are: 27, 359, 503, 597, 987, 1114, 1538, 1629, 1816, 1902, 2614, 2834, 3132, and 4613.
 *
 *
 * Author's note: RIP 1311.
 * */

#define EVENT_STATUS_ITEMS(X)\
	X(FUTURE)\
	X(IN_PROGRESS)\
	X(COMPLETE)

enum class Event_status{
	#define X(A) A,
	EVENT_STATUS_ITEMS(X)
	#undef X
};

std::ostream& operator<<(std::ostream& o,Event_status a){
	#define X(A) if(a==Event_status::A) return o<<""#A;
	EVENT_STATUS_ITEMS(X)
	#undef X
	assert(0);
}

static Event_status event_status(TBA_fetcher& tba_fetcher,tba::Event_key event);

static std::optional<tba::Team_key> ei(TBA_fetcher &tba_fetcher,tba::Event_key event_key){
	auto a=tba::event_awards(tba_fetcher,event_key);
	//PRINT(event_key);
	auto f=filter([](auto x){ return x.award_type==tba::Award_type::ENGINEERING_INSPIRATION; },a);
	if(f.empty()){
		return std::nullopt;
	}
	assert(f.size()==1);
	auto x=f[0].recipient_list;
	assert(x.size()==1);
	auto x2=x[0].team_key;
	assert(x2);
	return *x2;
}

static std::set<tba::Team_key> cmp_ei(TBA_fetcher &tba_fetcher,tba::Year year){
	auto events=events_year(tba_fetcher,year);
	auto f=filter([](auto x){ return x.event_type==tba::Event_type::CMP_DIVISION; },events);
	auto event_keys=keys(f);
	//TODO: if nothign from the fields, then look for something from the main field.
	auto found=to_set(nonempty(mapf([&](auto x){ return ei(tba_fetcher,x); },event_keys)));
	if(found.size()){
		return found;
	}
	auto f2=filter([](auto x){ return x.event_type==tba::Event_type::CMP_FINALS; },events);
	auto m=mapf([&](auto x){ return ei(tba_fetcher,x.key); },f2);
	return to_set(flatten(m));
}

static std::set<tba::Team_key> chairmans(TBA_fetcher &tba_fetcher,tba::Event_key event_key){
	auto a=tba::event_awards(tba_fetcher,event_key);
	auto f1=filter([](auto x){ return x.award_type==tba::Award_type::CHAIRMANS; },a);
	if(f1.empty()){
		return {};
	}
	assert(f1.size()==1);
	auto f=f1[0];
	auto x=f.recipient_list;
	return to_set(mapf([](auto x){ return *x.team_key; },f.recipient_list));
}

static std::set<tba::Team_key> cmp_chairmans(TBA_fetcher &tba_fetcher,tba::Year year){
	auto events=events_year(tba_fetcher,year);
	auto f=filter([](auto x){ return x.event_type==tba::Event_type::CMP_FINALS; },events);
	auto event_keys=keys(f);
	auto m=mapf([&](auto x){ return chairmans(tba_fetcher,x); },event_keys);
	return to_set(flatten(m));
}

static std::set<tba::Team_key> hof(TBA_fetcher &tba_fetcher,tba::Year current_year){
	//teams that were in the hall of fame at the beginning of the specified year
	auto years=range(tba::Year{1992},current_year);
	return or_all(mapf(
		[&](auto x){ return cmp_chairmans(tba_fetcher,x); },
		years
	));
}

static set<tba::Team_key> chairmans_pre_qualifying(TBA_fetcher &tba_fetcher,tba::Year current_year){
	//the ones besides last year's.
	set<tba::Team_key> r;
	auto at=current_year-2;
	for(int years_found=0;years_found<=10 && at>tba::Year{1992};--at){
		auto found=cmp_chairmans(tba_fetcher,at);
		if(found.size()){
			//cout<<at<<":"<<found<<"\n";
			r|=found;
			years_found++;
		}
	}
	return r;
}

static set<tba::Team_key> pre_qualifying(TBA_fetcher &tba_fetcher,tba::Year current_year){
	tba::Year year=current_year-1;
	//PRINT(ei(tba_fetcher,year));

	auto e1=tba::events_year(tba_fetcher,year);
	//print_r(e1);
	auto m=mapf([](auto x){ return x.event_type; },e1);
	//PRINT(to_set(m));

	auto f1=filter(
		[](auto x){
			return x.event_type==tba::Event_type::CMP_FINALS;
		},
		e1
	);
	/*if(f1.size()!=1){
		PRINT(year);
		print_r(f1);
		assert(0);
	}*/
	assert(f1.size()==1 || f1.size()==2);
	//auto f=f1[0];
	std::set<tba::Team_key> teams;
	for(auto f:f1){
		std::set<tba::Award_type> prequalified_award_types{
			tba::Award_type::WINNER,
			tba::Award_type::CHAIRMANS,
			tba::Award_type::CHAIRMANS_FINALIST
		};

		auto key=f.key;
		//PRINT(key);
		auto e=tba::event_awards(tba_fetcher,key);
		auto e2=filter([=](auto x){ return prequalified_award_types.count(x.award_type); },e);
		//print_r(e2);
		auto m2=to_set(nonempty(flatten(mapf(
			[](auto x){
				return mapf([](auto y){ return y.team_key; },x.recipient_list);
			},
			e2
		))));
		teams|=m2;
	}
	auto c=chairmans_pre_qualifying(tba_fetcher,current_year);
	return c|teams|cmp_ei(tba_fetcher,year);
	//print_r(m2);
	//want to find:
	//-championship winners
	//-impact award finalists
	//-EI award winners
	//-impact award winner
	//
	//chairmans from the last 10 FRC championships
	//nyi
}

#define CMP_REASON_ITEMS(X)\
	X(PRE_QUALIFIED)\
	X(REGIONAL)\
	X(DISTRICT)\
	X(PRIORITY_WAITLIST)\
	X(WAITLIST)

enum class Cmp_reason{
	#define X(A) A,
	CMP_REASON_ITEMS(X)
	#undef X
};

std::ostream& operator<<(std::ostream& o,Cmp_reason a){
	#define X(A) if(a==Cmp_reason::A) return o<<""#A;
	CMP_REASON_ITEMS(X)
	#undef X
	assert(0);
}

static std::optional<tba::Team_key> team_winners(tba::Award_Recipient const& a){
	return a.team_key;
}

static std::vector<tba::Team_key> team_winners(std::vector<tba::Award_Recipient> const& a){
	return flatten(MAP(team_winners,a));
}

static std::vector<tba::Team_key> team_winners(tba::Award const& a){
	return team_winners(a.recipient_list);
}

static std::vector<tba::Team_key> team_winners(std::vector<tba::Award> const& a){
	return flatten(MAP(team_winners,a));
}

static std::set<tba::Team_key> regional_quals(TBA_fetcher &tba_fetcher,tba::Year year){
	auto e=events(tba_fetcher,year);
	auto f=filter([](auto x){ return x.event_type==tba::Event_type::REGIONAL; },e);
	auto m=sorted(mapf([](auto x){ return make_pair(x.week,x.key); },f));
	auto m2=seconds(m);
	//print_r(m2);
	std::set<tba::Team_key> r;
	for(auto event:m2){
		auto aw=event_awards(tba_fetcher,event);
		//print_r(mapf([](auto x){ return x.award_type; },aw));
		std::set<tba::Award_type> advancing{
			tba::Award_type::CHAIRMANS,
			tba::Award_type::WINNER,
			tba::Award_type::WILDCARD,
			tba::Award_type::ENGINEERING_INSPIRATION
		};
		auto f=filter([=](auto x){ return advancing.count(x.award_type); },aw);
		r|=team_winners(f);
	}
	return r;
}

static std::vector<tba::Team> teams(TBA_fetcher &tba_fetcher){
	std::vector<tba::Team> r;
	for(int i=0;;++i){
		auto t=tba::teams(tba_fetcher,i);
		if(t.empty()){
			break;
		}
		r|=t;
	}
	return r;
}

static std::set<tba::Team_key> original_teams(TBA_fetcher &tba_fetcher){
	auto f=filter(
		[](auto const& team){ return team.rookie_year==tba::Year{1992}; },
		teams(tba_fetcher)
	);
	return to_set(keys(f));
}

static auto regionals(TBA_fetcher &tba_fetcher,tba::Year year){
	auto f=filter(
		[](auto x){ return x.event_type==tba::Event_type::REGIONAL; },
		events_year(tba_fetcher,year)
	);
	return keys(f);
}

static bool winning(std::optional<tba::M_Elimination_Alliance_status> const& a){
	//This is not being used at the moment because there are too many 
	//events where the last match is missing, making it look like 
	//there was no winner.
	
	if(!a) return 0;
	auto b=*a;
	if(std::holds_alternative<tba::Unknown>(b)){
		return 0;
	}
	assert(std::holds_alternative<tba::Elimination_Alliance_status>(b));
	auto c=std::get<tba::Elimination_Alliance_status>(b);

	if(c.level=="f" && c.current_level_record && c.current_level_record->wins==2){
		return 1;
	}

	if(c.level=="f" && c.current_level_record && c.current_level_record->losses==2){
		return 0;
	}

	if(c.level=="sf"){
		return 0;
	}

	print_r(c);
	nyi
}

static std::vector<tba::Team_key> winners(TBA_fetcher &tba_fetcher,tba::Event_key event){
	auto aw=event_awards(tba_fetcher,event);
	auto f=filter([](auto x){ return x.award_type==tba::Award_type::WINNER; },aw);
	return team_winners(f);
}

/*auto teams(tba::Elimination_Alliance const& a){
	return a.picks|a.backup;
}*/

static std::optional<tba::Team_key> winning_alliance_second_pick(TBA_fetcher &tba_fetcher,tba::Event_key event){
	auto w=winners(tba_fetcher,event);
	if(w.empty()){
		return std::nullopt;
	}

	auto ea=event_alliances(tba_fetcher,event);
	
	if(!ea || ea->empty()){
		//no data about alliances, hope that the order of the awards preserves pick order!

		if(w.size()<3){
			//happened w/ the data for 2005fl.
			return std::nullopt;
		}
		return w[2];
	}

	auto f=filter(
		[=](auto x){ return (to_set(x.picks)&to_set(w)).size(); },
		*ea
	);

	if(f.size()!=1){
		PRINT(event);
		PRINT(w);
		print_r(*ea);
		print_r(f);
		nyi
	}

	auto fu=f[0];
	assert(fu.picks.size()>=3);
	return fu.picks[2];

	#if 0
	PRINT(w);
	nyi

	auto aw=event_awards(tba_fetcher,event);
	if(aw.empty()){
		return std::nullopt;
	}
	PRINT(event);
	print_r(aw);
	nyi

	//can't just do it this way because sometimes the last match result is missing
	//even though the event is over, so need to look at awards to figure out who won.
	auto e=event_alliances(tba_fetcher,event);

	//probably because the event has not yet happened.
	if(!e) return std::nullopt;

	auto f1=filter([](auto x){ return winning(x.status); },*e);
	if(f1.empty()){
		return std::nullopt;
	}
	assert(f1.size()==1);
	auto f=f1[0];
	/*if(f.picks.size()!=3){
		PRINT(event);
		print_r(f);
	}*/
	assert(f.picks.size()>=3);
	return f.picks[2];
#endif
}

static std::set<tba::Team_key> regional_winning_alliance_second_pick(TBA_fetcher &tba_fetcher,tba::Year year){
	auto m=mapf(
		[&](auto x){ return winning_alliance_second_pick(tba_fetcher,x); },
		regionals(tba_fetcher,year)
	);
	return to_set(nonempty(m));
}

static std::set<tba::Team_key> rookie_all_star(TBA_fetcher &tba_fetcher,tba::Event_key const& event){
	auto aw=event_awards(tba_fetcher,event);
	auto f=filter([](auto x){ return x.award_type==tba::Award_type::ROOKIE_ALL_STAR; },aw);
	//This can be >1 team.  Example: 2004on
	return to_set(team_winners(f));
}

static std::set<tba::Team_key> regional_rookie_all_stars(TBA_fetcher &tba_fetcher,tba::Year year){
	return or_all(mapf(
		[&](auto x){ return rookie_all_star(tba_fetcher,x); },
		regionals(tba_fetcher,year)
	));
}

static std::optional<tba::Team_key> winning_backups(TBA_fetcher &tba_fetcher,tba::Event_key event){
	auto w=winners(tba_fetcher,event);
	if(w.size()<4){
		return std::nullopt;
	}
	
	auto ea=event_alliances(tba_fetcher,event);

	if(!ea || ea->empty()){
		//in case don't have the alliance directly recorded.
		return w[3];
	}

	assert(ea);
	auto e=*ea;
	assert(e.size());
	auto f=filter_unique([=](auto x){ return (to_set(w)&to_set(x.picks)).size(); },e);

	if(f.picks.size()==3){
		auto r=to_set(w)-to_set(f.picks);
		assert(r.size()==1);
		return *r.begin();
	}

	//PRINT(event);
	//PRINT(f.picks);
	assert(f.picks.size()>=4);
	return f.picks[3];
}

static std::set<tba::Team_key> regional_winning_backups(TBA_fetcher& tba_fetcher,tba::Year year){
	return to_set(nonempty(mapf(
		[&](auto x){ return winning_backups(tba_fetcher,x); },
		regionals(tba_fetcher,year)
	)));
}

static std::set<tba::Team_key> priority_waitlist(TBA_fetcher &tba_fetcher,tba::Year year){
	std::set<tba::Team_key> r;
	
	//Criteria for the priority waitlist:
	//1) HOF teams not from the last 10 years
	r|=hof(tba_fetcher,year)-chairmans_pre_qualifying(tba_fetcher,year);

	//2) original and sustaining teams
	r|=original_teams(tba_fetcher);

	//3) regional winning alliance second pick
	r|=regional_winning_alliance_second_pick(tba_fetcher,year);

	//4) regional regional rookie all star
	r|=regional_rookie_all_stars(tba_fetcher,year);

	//5) regional winning alliance backup teams
	r|=regional_winning_backups(tba_fetcher,year);

	return r;
}

static bool cmp_event(tba::Event_type e){
	switch(e){
		case tba::Event_type::CMP_FINALS:
		case tba::Event_type::CMP_DIVISION:
			return 1;
		case tba::Event_type::REGIONAL:
		case tba::Event_type::PRESEASON:
		case tba::Event_type::DISTRICT_CMP:
		case tba::Event_type::DISTRICT:
		case tba::Event_type::DISTRICT_CMP_DIVISION:
		case tba::Event_type::OFFSEASON:
			return 0;
		default:
			PRINT(e);
			nyi
	}
}

static bool cmp_event(tba::Event const& a){
	return cmp_event(a.event_type);
}

/*static std::set<tba::Team_key> teams(TBA_fetcher &tba_fetcher,tba::Event_key const& a){
	return to_set(tba::event_teams_keys(tba_fetcher,a));
}*/

/*static auto teams(TBA_fetcher &tba_fetcher,tba::Event const& a){
	return teams(tba_fetcher,a.key);
}*/

static std::set<tba::Team_key> cmp_teams(TBA_fetcher &tba_fetcher,tba::Year year){
	auto f=filter(
		[](auto const& x){ return cmp_event(x); },
		tba::events(tba_fetcher,year)
	);
	auto m=mapf([&](auto x){ return event_teams_keys(tba_fetcher,x.key); },f);
	return to_set(flatten(m));
}

static auto district_keys(TBA_fetcher &tba_fetcher,tba::Year year){
	return keys(districts(tba_fetcher,year));
}

static bool dcmp(tba::Event_type a){
	using enum tba::Event_type;
	switch(a){
		case DISTRICT:
			return 0;
		case DISTRICT_CMP:
		case DISTRICT_CMP_DIVISION:
			return 1;
		default:
			PRINT(a);
			nyi
	}
}

static std::set<tba::Team_key> district_quals(
	TBA_fetcher &tba_fetcher,
	tba::District_key district,
	std::set<tba::Team_key> const& attending_teams
){
	//first, see if the district is done
	auto x=district_events(tba_fetcher,district);
	auto f=filter([](auto x){ return dcmp(x.event_type); },x);
	assert(f.size());
	auto cmp_events=keys(f);
	auto a=to_set(mapf([&](auto x){ return event_status(tba_fetcher,x); },cmp_events));
	if(a!=set{Event_status::COMPLETE}){
		return {};
	}
	auto rank=district_rankings(tba_fetcher,district);
	assert(rank);
	auto r=*rank;

	size_t slots=worlds_slots(district);

	auto aw=flatten(mapf([&](auto x){ return event_awards(tba_fetcher,x); },cmp_events));
	std::set<tba::Award_type> auto_qual{
		tba::Award_type::CHAIRMANS,
		tba::Award_type::ENGINEERING_INSPIRATION,
		tba::Award_type::ROOKIE_ALL_STAR
	};
	auto f2=filter([=](auto x){ return auto_qual.count(x.award_type); },aw);
	auto by_award=to_set(team_winners(f2));

	//if some of the award winners aren't attending, then more slots for teams by points
	auto extra_by_award=(by_award-attending_teams).size();
	slots+=extra_by_award;

	std::set<tba::Team_key> by_pts;
	for(size_t i=0;i<r.size() && by_pts.size()<slots;i++){
		auto team=r[i].team_key;
		if(!by_award.count(team)){
			if(!attending_teams.count(team)){
				//if not going, don't count it against the total.
				slots++;
			}
			by_pts|=team;
		}
	}
	return by_award|by_pts;
}

static std::set<tba::Team_key> district_quals(
	TBA_fetcher &tba_fetcher,
	tba::Year year,
	std::set<tba::Team_key> const& attending_teams //so that can fill in for declines
){
	return or_all(mapf(
		[&](auto x){ return district_quals(tba_fetcher,x,attending_teams); },
		district_keys(tba_fetcher,year)
	));
}

static map<tba::Team_key,Cmp_reason> cmp_reasons(TBA_fetcher &tba_fetcher,tba::Year year){
	auto teams=cmp_teams(tba_fetcher,year);
	map<tba::Team_key,Cmp_reason> r;
	
	auto f=[&](Cmp_reason reason,std::set<tba::Team_key> team_list){
		for(auto elem:team_list){
			r[elem]=reason;
		}
		teams-=team_list;
	};

	f(Cmp_reason::PRE_QUALIFIED,pre_qualifying(tba_fetcher,year));
	f(Cmp_reason::REGIONAL,regional_quals(tba_fetcher,year));
	f(Cmp_reason::DISTRICT,district_quals(tba_fetcher,year,teams));
	f(Cmp_reason::PRIORITY_WAITLIST,priority_waitlist(tba_fetcher,year));

	for(auto t:teams){
		r[t]=Cmp_reason::WAITLIST;
	}

	return r;
#if 0

	auto f=filter(
		//[](auto x){ return x.event_type==tba::Event_type::CMP_FINALS; },
		//[](auto x){ return cmp_event(x.event_type==tba::Event_type::CMP_FINALS; },
		[](auto x){ return cmp_event(x); },
		tba::events(tba_fetcher,year)
	);
	auto teams=flatten(mapf([&](auto x){ return teams(tba_fetcher,x); },f));
	print_r(m);
	/*for(auto event:f){
		auto t=to_set(event_teams_keys(tba_fetcher,event.key));
		nyi
	}*/
#endif
}

static auto districts_keys(TBA_fetcher &fetcher,tba::Year year){
	return keys(districts(fetcher,year));
}

static map<tba::District_key,std::vector<tba::Team_key>> district_teams(TBA_fetcher& tba_fetcher,tba::Year year){
	map<tba::District_key,std::vector<tba::Team_key>> r;
	for(auto district:districts_keys(tba_fetcher,year)){
		r[district]=district_teams_keys(tba_fetcher,district);
	}
	return r;
}

static unsigned int playoff_size(TBA_fetcher& tba_fetcher,tba::Event_key event){
	if(event=="2022waspo"){
		//the fact that this special case has to be here is total insanity.
		//who ran that event, anyway?
		return 6;
	}
	auto event_info=tba::event(tba_fetcher,event);
	if(event_info.playoff_type==std::nullopt){
		return 8;
	}
	auto t=*event_info.playoff_type;
	using enum tba::Playoff_type;
	switch(t){
		case BRACKET_8_TEAM: return 8;
		case AVG_SCORE_8_TEAM: return 8;
		case BRACKET_4_TEAM: return 4;
		case DOUBLE_ELIM_8_TEAM: return 8;
		case DE8: return 8;
		default:
			print_r(t);
			nyi
	}
}

static int event_points_multiplier(TBA_fetcher &tba_fetcher,tba::Event_key event){
	auto e=tba::event(tba_fetcher,event);
	using enum tba::Event_type;
	switch(e.event_type){
		case DISTRICT:
			return 1;
		case DISTRICT_CMP_DIVISION:
		case DISTRICT_CMP:
			return 3;
		default:
			PRINT(e.event_type);
			assert(0);
	}
}

static Event_status event_status(TBA_fetcher& tba_fetcher,tba::Event_key event){
	if(event.get().substr(0,6)=="202121"){
		//The "participation" events for 2021 are not in the future, despite no play having happened.
		return Event_status::COMPLETE;
	}

	//Doing this check early because a bunch of 2020 events did nothing but award the Chairman's award
	//but they are obviously not still in progress.
	{
		auto c=chairmans(tba_fetcher,event);
		if(!c.empty()){
			return Event_status::COMPLETE;
		}
	}

	auto e=event_district_points(tba_fetcher,event);
	assert(e);
	auto v=values(e->points);
	if(v.empty()){
		return Event_status::FUTURE;
	}

	auto f=[](auto x){
		cout<<x<<"\n";
		cout<<"min:"<<min(x)<<"\n";
		cout<<"max:"<<max(x)<<"\n";
		cout<<"mean:"<<mean(x)<<"\n";
		cout<<"median:"<<median(x)<<"\n";
		cout<<"sum:"<<sum(x)<<"\n";
		cout<<"size:"<<x.size()<<"\n";
	};
	(void)f;

	#define X(A,B) \
		auto B=multiset_flat(mapf([](auto x){ return x.B; },v));
	TBA_POINTS(X)
	#undef X

	if(sum(qual_points)==0){
		return Event_status::FUTURE;
	}

	auto x1=sum(alliance_points)/event_points_multiplier(tba_fetcher,event);
	auto p1=playoff_size(tba_fetcher,event);
	int expected_selection_pts;
	switch(p1){
		case 8:
			expected_selection_pts=236;
			break;
		case 4:
			expected_selection_pts=124;
			break;
		case 6:
			expected_selection_pts=183;
			break;
		default:
			assert(0);
	}

	if(x1==expected_selection_pts){
		//alliances are complete
	}else if(x1==0){
		//alliance selection has not begun or no data
	}else{
		if(x1>expected_selection_pts){
			PRINT(event);
			PRINT(x1);
			PRINT(expected_selection_pts);
		}
		assert(x1<expected_selection_pts);
		//alliance selection in progress
		//or it's 202wasp and they ran with 6 alliances.
		return Event_status::IN_PROGRESS;
	}

	auto x3=sum(elim_points);
	if(x3==0){
		//then no playoffs yet
		//cout<<"no playoffs\n";
		return Event_status::IN_PROGRESS;
	}

	//because a bunch of districts in the Chesapeake district
	//in 2022 didn't give out a Chairman's Award???
	vector<string> no_chairmans={
		"2022dc305",
		"2022dc313",
		"2022dc320",
		"2022dc327",
		"2022on204",
		"2022on273",
		"2022on305",
		"2022on325",
		"2022on409",
		"2022va306",
		"2022va319"
	};

	for(auto x:no_chairmans){
		if(event==tba::Event_key(x)){
			return Event_status::COMPLETE;
		}
	}

	return Event_status::IN_PROGRESS;
}

static std::map<tba::Event_key,Event_status> event_status(TBA_fetcher& tba_fetcher,tba::Year year){
	auto a=events(tba_fetcher,year);
	auto b=filter([](auto x){ return x.event_type==tba::Event_type::DISTRICT; },a);
	return to_map(mapf(
		[&](auto x){ return make_pair(x,event_status(tba_fetcher,x)); },
		keys(b)
	));
}

static std::set<tba::Team_key> district_teams_at_cmp(TBA_fetcher& tba_fetcher,tba::Year year){
	auto teams=to_set(flatten(values(district_teams(tba_fetcher,year))));
	return teams&cmp_teams(tba_fetcher,year);
}

static auto elimination_points(TBA_fetcher &tba_fetcher,tba::Event_key const& event){
	auto e=tba::event_district_points(tba_fetcher,event);
	assert(e);
	return mapf([](auto x){ return x.elim_points; },values(e->points));
}

static std::vector<tba::Event_key> local_district_events(TBA_fetcher &tba_fetcher,tba::Year year){
	auto events=tba::events(tba_fetcher,year);
	events=filter(
		[](auto x){ return x.event_type==tba::Event_type::DISTRICT; },
		events
	);
	return keys(events);
}

static map<int,double> elimination_points(TBA_fetcher &tba_fetcher,tba::Year year){
	auto m=mapf(
		[&](auto x){ return elimination_points(tba_fetcher,x); },
		local_district_events(tba_fetcher,year)
	);
	auto m2=flatten(m);
	auto c=count(flatten(m));
	return to_map(mapf([=](auto x){ return make_pair(int(x.first),double(x.second)/m2.size()); },c));
}

static void elimination_points(TBA_fetcher &tba_fetcher){
	//try to come up with a baseline for how many points are given out during eliminations
	//see how it changes per year -> definitely different distribution when playoff pt rules change
	//see how the distribution changes
	//see if this changes between DCMP and regular district events
	for(auto year:range(tba::Year{1992},tba::Year{2025})){
		auto c2=elimination_points(tba_fetcher,year);
		PRINT(year);
		for(auto [a,b]:c2){
			int x=b*100;
			cout<<a<<":"<<x<<" ";
		}
		cout<<"\n";
		//PRINT(sum(values(c2)));
	}
}

static auto award_points(TBA_fetcher &tba_fetcher,tba::Event_key const& event){
	auto e=tba::event_district_points(tba_fetcher,event);
	assert(e);
	return mapf([](auto x){ return x.award_points; },values(e->points));
}

static map<int,double> award_points(TBA_fetcher &tba_fetcher,tba::Year const& year){
	auto m=mapf(
		[&](auto x){ return award_points(tba_fetcher,x); },
		local_district_events(tba_fetcher,year)
	);
	//if(m.empty()) return;
	auto m2=flatten(m);
	auto c=count(m2);
	return to_map(mapf([=](auto x){ return make_pair(x.first,double(x.second)/m2.size()); },c));
}

auto years=range(tba::Year{1992},tba::Year{2025});

static void award_points(TBA_fetcher &tba_fetcher){
	for(auto year:years){
		auto x=award_points(tba_fetcher,year);
		PRINT(year);
		PRINT(x);
		//Note: Distribution is noticably different depending on the ruleset for the year
		//Like post 2022, seems like winning EI/Chairmans+anything else is not possible
		//but it does not seem super jumpy on a year to year basis.
	}
}

//would be interesting to see how much correlation there is between award points & elimination points

static auto award_plus_elim(TBA_fetcher &tba_fetcher,tba::Event_key event){
	auto e=tba::event_district_points(tba_fetcher,event);
	assert(e);
	return mapf([](auto x){ return x.award_points+x.elim_points; },values(e->points));
}

static map<int,double> award_plus_elim(TBA_fetcher &tba_fetcher,tba::Year year){
	auto m=mapf(
		[&](auto x){ return award_plus_elim(tba_fetcher,x); },
		local_district_events(tba_fetcher,year)
	);
	auto m2=flatten(m);
	auto c=count(m2);
	return to_map(mapf([=](auto x){ return make_pair(int(x.first),double(x.second)/m2.size()); },c));
}

static void award_plus_elim(TBA_fetcher &tba_fetcher){
	for(auto year:years){
		auto a=award_plus_elim(tba_fetcher,year);
		cout<<year<<" ";
		for(auto [k,v]:a){
			cout<<k<<":"<<int(v*100)<<" ";
		}
		cout<<"\n";

		auto aw=award_points(tba_fetcher,year);
		auto el=elimination_points(tba_fetcher,year);
		auto alt=convolve(aw,el);
		compare(a,alt);
	}
}

int demo(TBA_fetcher& tba_fetcher){
	//might eventually want to have a score distribution for in-progress events
	//that is different
	//or could just discount all the numbers from events that are currently in progress
	//approaches to making the distribution of points:
	//1) split into sections of how much is complete, and give points for those + dist for remainder
	//	-would need to have some idea of what the distribution looks like for remaining parts
	//2) for quals, could put in ranges or where the rank can be at the end for each team, etc.
	//if have a partial total of pts for a team at an event, need to come up with a distribution
	//for the event total; could just lop off the part of the distribution where the total is
	//below what's been calculated as the minimum.
	//could also try to calculate a maximum number for a team remaining
	
	cout<<"Award+\n";
	award_plus_elim(tba_fetcher);

	cout<<"Award\n";
	award_points(tba_fetcher);

	cout<<"Elim:\n";
	elimination_points(tba_fetcher);

	tba::Year current_year{2024};
	{
		//auto p=priority_waitlist(tba_fetcher,current_year);
		//print_r(p);
		auto c=cmp_reasons(tba_fetcher,current_year);
		//print_r(invert(c));
		//nyi
	}

	(void)district_teams_at_cmp;
	(void)winning;
	//auto x=in_progress(tba_fetcher,tba::Event_key{"2024isde1"});
	//PRINT(x);
	/*{
		tba::Year year{2024};
		auto d=district_teams_at_cmp(tba_fetcher,year);
		auto pq=pre_qualifying(tba_fetcher,year);
		
		PRINT(d&pq);
		d-=pq;
		PRINT(d.size());
		PRINT(d);
		//nyi
	}*/

	auto years=range(tba::Year(2004),tba::Year(2025));

	for(auto year:years){
		auto e=event_status(tba_fetcher,year);
		cout<<year<<count(seconds(e))<<"\n";

	}

	for(auto year:range(tba::Year{2022},tba::Year{2025})){
		auto c=cmp_reasons(tba_fetcher,year);
		PRINT(count(values(c)));
	}

	for(auto current_year:range(tba::Year(2004),tba::Year(2025))){
		auto p=pre_qualifying(tba_fetcher,current_year);
		//cout<<current_year<<":"<<p<<"\n";
	}

	return 0;
}
