#include "status.h"
#include<iomanip>
#include "outline.h"
#include<optional>
#include<cmath>
#include "../tba/tba.h"
#include "set.h"
#include "util.h"
#include "map.h"
#include "tba.h"
#include "print_r.h"
#include "multiset_flat.h"
#include "event.h"

auto head(auto x){
	return take(10,x);
}

template<typename T>
auto seconds(std::vector<std::vector<T>> const& a){
	return mapf(
		[](auto x){
			assert(x.size()>=2);
			return x[1];
		},
		a
	);
}

template<typename T>
auto second(std::vector<T> const& a){
	assert(a.size()>=2);
	return a[1];
}

template<typename T,size_t N>
auto second(std::array<T,N> const& a){
	static_assert(N>=2);
	return a[1];
}

template<typename T>
auto seconds(std::vector<T> const& a){
	return MAP(second,a);
}

std::string as_pct(double d){
	std::stringstream ss;
	ss<<int(d*100)<<'%';
	return ss.str();
}

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

auto swap_pairs(auto a){
	return mapf([](auto x){ return make_pair(x.second,x.first); },a);
}

template<typename T>
std::set<T> operator-(std::set<T> a,T t){
	a.erase(t);
	return a;
}

template<typename T>
auto first(T const& t){
	assert(!t.empty());
	return *begin(t);
}

template<typename T>
auto firsts(T const& t){
	return MAP(first,t);
}

template<typename T>
auto to_vec(std::multiset<T> const& a){
	return std::vector<T>{a.begin(),a.end()};
}

auto square(auto x){
	return x*x;
}

auto variance(std::vector<int> v)->double{
	auto mu=mean(v);
	return mean(mapf(
		[mu](auto x){ return square(x-mu); },
		v
	));
}

auto std_dev(std::vector<int> v){
	return sqrt(variance(v));
}

template<typename T>
auto std_dev(std::multiset<T> const& a){
	return std_dev(to_vec(a));
}

auto mad(std::vector<int> v)->double{
	//mean absolute devaition
	auto mu=mean(v);
	return mean(mapf(
		[mu](auto x){ return fabs(x-mu); },
		v
	));
}

template<typename T>
auto mad(std::multiset<T> a){
	return mad(to_vec(a));
}

static auto consolidate_inner(std::vector<int> in){
	auto s=to_set(in);
	std::vector<std::pair<int,int>> v;
	if(s.empty()){
		return v;
	}
	int start=*begin(s);
	for(int i=start;i<=max(s);++i){
		if(!s.count(i)){
			v|=std::make_pair(start,i-1);
			do{
				i++;
			}while(i<=max(s) && s.count(i)==0);
			start=i;
		}
	}
	v|=std::make_pair(start,max(s));
	return v;
}

auto consolidate(std::vector<int> in){
	auto v=consolidate_inner(in);
	std::stringstream ss;
	for(auto x:v){
		if(x.first==x.second){
			ss<<x.first;
		}else{
			ss<<x;
		}
		ss<<' ';
	}
	return ss.str();
}

#define RM_CONST(X) typename std::remove_cv<X>::type
#define RM_REF(X) typename std::remove_reference<X>::type
#define ELEM(X) RM_CONST(RM_REF(decltype(*std::begin(X))))

auto cross(auto a,auto b){
	using A=ELEM(a);
	using B=ELEM(b);
	std::vector<std::pair<A,B>> r;
	for(auto a1:a){
		for(auto b1:b){
			r|=std::make_pair(a1,b1);
		}
	}
	return r;
}

template<typename T>
auto to_array(std::pair<T,T> a){
	return std::array<T,2>{a.first,a.second};
}

template<typename T>
auto mean(std::multiset<T> const& a){
	return mean(to_vec(a));
}

auto quartiles(auto a){
	assert(!a.empty());
	auto b=sorted(a);
	return std::array{b[b.size()/4],b[b.size()/2],b[b.size()*3/4]};
}


template<typename Func,typename T>
auto filter_first(Func f,T const& t){
	for(auto const& elem:t){
		if(f(elem)){
			return elem;
		}
	}
	assert(0);
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

static const int ROUND_AMOUNT=1;

static int round(int x){
	return x/ROUND_AMOUNT*ROUND_AMOUNT;
}

template<typename T,size_t N>
static auto round(std::array<T,N> a){
	return MAP(round,a);
}

template<typename T>
static auto round(vector<T> a){
	return MAP(round,a);
}

/*void pre_dcmp_prediction(TBA_fetcher &tba_fetcher){
	
}*/

static void skill_chart(TBA_fetcher &tba_fetcher){
	//TODO: Check how large the sample sizes are for the pre-dcmp totals

	//for each point total at first event, what is the distribution of pts at second event?
	//also would want to have some sort of measurement between first 2 events and dcmp
	//also would be interesting to see distribution of season totals after first event

	//These result in not super intuitive results unless you put in a huge amount of smoothing.
	//Don't really have a principled way to account for amount of smoothing to apply
	//Maybe try to see how many pseudo counts entered give the best approximation between 
	//different years?

	//Also TODO: figure out where some of the unusually high point totals come from
	//TODO: Deal w/ the teams that don't make it to a second event as 0.
	//TODO: Produce model of if teams ever re-appear after missing an event that they are scheduled for.

	vector<tba::District_Ranking> r;
	auto years=to_set(range(2015,2025))-2020;
	//point totals are totally messed up for 2020 because of the DCMP not having matches, but still awarding points
	//for some awards
	for(auto year:years){
		for(auto district:district_keys(tba_fetcher,tba::Year{year})){
			auto d=district_rankings(tba_fetcher,district);
			assert(d);
			r|=*d;
		}
	}

	auto attendance_type=[](auto x){
		//print_r(x.event_points);
		auto m=mapf([](auto x){ return x.district_cmp; },x.event_points);
		return to_multiset(m);
	};

	auto c=count(mapf(attendance_type,r));
	//print_lines(sorted(swap_pairs(c)));

	//there are actually 8 times teams don't have a second district event 
	//but did attend their district championship.

	/*auto f=filter([&](auto x){ return attendance_type(x)==multiset<bool>{0,1}; },r);
	print_r(f);
	nyi*/

	auto a=mapf(
		[](auto x){
			//print_r(x.event_points);
			auto g=group([](auto y){ return y.district_cmp; },x.event_points);
			//print_r(g);
			auto v=map_values(
				[](auto x){
					return mapf([](auto y){ return y.total; },x);
				},
				g
			);
			return v;
		},
		r
	);
	//print_lines(a);

	/*auto m=mapf(
		[](auto x){
			return mapf([](auto y){ return y.total; },x.event_points);
		},
		r
	);*/
	//print_r(m);

	//at some point, should run this with missing events being counted as 0.
	/*auto m3=mapf(
		[](auto x){ return take(2,x); },
		filter([](auto x){ return x.size()>=2; },m)
	);
	*/
	vector<std::array<int,2>> m3;
	/*for(auto elem:a){
		auto p=elem[0];
		switch(p.size()){
			case 0:
				continue;
			case 1:
				m3|=std::array<int,2>{int(p[0]),0};
				break;
			case 2:
				m3|=std::array<int,2>{int(p[0]),int(p[1])};
				break;
			default:
				assert(0);
		}
	}*/
	for(auto elem:filter([](auto x){ return x[1].size(); },a)){
		auto pre=sum(elem[0]);
		auto post=sum(elem[1]);
		m3|=std::array<int,2>{pre,post};
	}

	auto m2=round(m3);

	auto max_value=max(seconds(m2));
	PRINT(max_value);

	PRINT(consolidate(firsts(m2)));
	PRINT(consolidate(seconds(m2)));
	PRINT(m2.size());
	auto x=to_set(firsts(m2));
	auto y=to_set(seconds(m2));
	PRINT(x.size());
	PRINT(y.size());
	{
		auto c=x.size()*y.size();
		PRINT(c);
		PRINT(m2.size()/double(c));
	}

	auto possible_combos=cross(x,y);
	auto actual_combos=to_multiset(m2);

	//map<pair<int,int>,unsigned> density;
	auto density=count(m2);

	auto distance=[&](auto x1,auto y1){
		auto d=density[array{x1,y1}];
		if(d) return 0;

		//by_distance
		auto s=sort_by(to_vec(x),[=](auto a){ return abs(a-x1); });
		auto f=filter([&](auto x2){ return density[array{x2,y1}]; },s);
		assert(!f.empty());
		auto found=f[0];
		auto dist=abs(found-x1);
		return dist;
	};

	//e2 numbers -> e1 number -> int
	//how to weight 
	//what % of the possible items have nonzero?

	//would make sense to have it so that expand region until get at least N counts?
	//also, would make sense to divide by the size of the region that was explored
	//to that distance

	#if 0
	for(auto x1:x){ //the performance in the first event.
		vector<int> distances;
		for(auto y1:y){
			auto d=distance(x1,y1);
			//cout<<x1<<" "<<y1<<" "<<d<<"\n";
			distances|=d;
		}
		auto w=mapf([](auto x){ return (x==0)?0:(double(1)/x); },distances);
		cout<<x1<<" "<<min(distances)<<" "<<max(distances)<<" "<<quartiles(distances);
		auto n=filter([=](auto p){ return p[0]==x1; },m3).size();
		cout<<" "<<n;
		cout<<" "<<sum(w)<<"\n";
		//cout<<distances<<"\n";
	}
	nyi
	#endif

	auto m=mapf([=](auto x){ return actual_combos.count(to_array(x)); },possible_combos);
	//what the distribution of how many occurrances have appeared is.
	//turns of like half of possible things have never occurred.
	//print_lines(count(m));

	double old_mean=0;
	vector<double> means;
	for(auto [k,v]:group([](auto x){ return x[0]; },m2)){
		auto v2=to_multiset(seconds(v));
		auto m2=mean(v2);
		auto dm=m2-old_mean;
		means|=dm;
		old_mean=m2;

		cout<<k<<"\t"<<v2.size()<<"\t"<<min(v2)<<"\t"<<max(v2)<<"\t"<<mean(v2);
		cout<<"\t"<<mad(v2)<<"\t"<<std_dev(v2);
		cout<<"\t"<<dm<<"\n";
		//would be interesting to see how far away the nearest thing is with that value
		//then have the value be 1/that.  
		//
	}

	cout<<"mean differences:\t"<<min(means)<<"\t"<<mean(means)<<"\t"<<max(means)<<"\n";
	PRINT(quartiles(means));
	cout<<"\n";


	//print_lines(sorted(m2));
	for(auto [k,v]:group([](auto x){ return x[0]; },m2)){
		auto v2=to_multiset(seconds(v));
		//cout<<k<<"\t"<<v2.size()<<"\t"<<min(v2)<<"\t"<<mean(v2)<<"\t"<<max(v2)<<"\n";
		cout<<k<<"\t";
		//cout<<v.size()<<"\t";
		//cout<<map_values([=](auto x){ return as_pct(double(x)/v2.size()); },count(v2));
		double total=0;
		for(auto i:range(0,max_value+1,ROUND_AMOUNT)){
			auto value=v2.count(i)/double(v2.size());
			total+=value;
			cout<<total<<"\t";
		}
		cout<<"\n";
	}
}

int demo(TBA_fetcher& tba_fetcher){
	skill_chart(tba_fetcher);

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

	/*
	 * How to keep track of what teams distributions look like:
	 * 1) pair of distributions, 1 for after district events, 1 for after dcmp
	 * 2) distribution of pairs of totals -> can later be shrunk down to only part later if needed.
	 * */

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
