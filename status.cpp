#include "status.h"
#include "event_status.h"
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
#include "cmp_reason.h"
#include "run.h"
#include "vector_void.h"
#include "array.h"
#include "skill_opr.h"
#include "optional.h"

using namespace std;

template<typename T>
std::multiset<T> operator&(std::multiset<T> a,std::multiset<T> b){
	std::multiset<T> r;
	for(auto k:to_set(a)|to_set(b)){
		auto n=std::min(a.count(k),b.count(k));
		for(auto _:range(n)){
			(void)_;
			r|=k;
		}
	}
	return r;
}

template<typename T>
std::multiset<T>& operator&=(std::multiset<T>& a,std::multiset<T> const& b){
	return a=(a&b);
}

template<typename T>
auto and_all(std::set<T> a){
	assert(!a.empty());
	auto r=*std::begin(a);
	for(auto const& elem:a){
		r&=elem;
	}
	return r;
}

template<typename T>
auto or_all(std::set<T> const& a){
	assert(!a.empty());
	auto r=*begin(a);
	for(auto const& elem:a){
		r|=elem;
	}
	return r;
}

template<typename T>
auto operator-(std::multiset<T> a,std::multiset<T> b){
	std::multiset<T> r;
	for(auto k:to_set(a)){
		auto c1=a.count(k);
		auto c2=b.count(k);
		if(c1>c2){
			auto n=c1-c2;
			for(auto _:range(n)){
				(void)_;
				r|=k;
			}
		}
	}
	return r;
}

template<typename Func,typename T>
auto count_if(Func f,T const& t){
	auto f1=filter(f,t);
	return f1.size();
}

template<typename T>
void print_lines(size_t n,T const& t){
	for(auto const& elem:t){
		indent(n);
		std::cout<<elem<<"\n";
	}
}

template<typename T,typename T2>
std::vector<T> operator|(std::vector<T> a,T2 t){
	a|=t;
	return a;
}

auto as_doubles(auto a){
	return MAP(double,a);
}

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

std::set<tba::Team_key> chairmans(TBA_fetcher &tba_fetcher,tba::Event_key event_key){
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

/*auto teams(tba::Elimination_Alliance const& a){
	return a.picks|a.backup;
}*/

/*static std::set<tba::Team_key> teams(TBA_fetcher &tba_fetcher,tba::Event_key const& a){
	return to_set(tba::event_teams_keys(tba_fetcher,a));
}*/

/*static auto teams(TBA_fetcher &tba_fetcher,tba::Event const& a){
	return teams(tba_fetcher,a.key);
}*/

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

static std::set<tba::Team_key> district_teams_at_cmp(TBA_fetcher& tba_fetcher,tba::Year year){
	auto teams=to_set(flatten(values(district_teams(tba_fetcher,year))));
	return teams&cmp_teams(tba_fetcher,year);
}

static optional<vector<Point>> elimination_points(TBA_fetcher &tba_fetcher,tba::Event_key const& event){
	auto e=tba::event_district_points(tba_fetcher,event);
	if(!e){
		return std::nullopt;
	}
	return mapf([](auto x){ return Point(x.elim_points); },values(e->points));
}

static std::vector<tba::Event_key> local_district_events(TBA_fetcher &tba_fetcher,tba::Year year){
	auto events=tba::events(tba_fetcher,year);
	events=filter(
		[](auto x){ return x.event_type==tba::Event_type::DISTRICT; },
		events
	);
	return keys(events);
}

static map<Point,double> elimination_points(TBA_fetcher &tba_fetcher,tba::Year year){
	auto m=nonempty(mapf(
		[&](auto x){ return elimination_points(tba_fetcher,x); },
		local_district_events(tba_fetcher,year)
	));
	auto m2=flatten(m);
	auto c=count(flatten(m));
	return to_map(mapf([=](auto x){ return make_pair(x.first,double(x.second)/m2.size()); },c));
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

static std::optional<std::vector<Point>> award_points(TBA_fetcher &tba_fetcher,tba::Event_key const& event){
	auto e=tba::event_district_points(tba_fetcher,event);
	if(!e){
		return std::nullopt;
	}
	return mapf([](auto x){ return Point(x.award_points); },values(e->points));
}

static map<Point,double> award_points(TBA_fetcher &tba_fetcher,tba::Year const& year){
	auto m=nonempty(mapf(
		[&](auto x){ return award_points(tba_fetcher,x); },
		local_district_events(tba_fetcher,year)
	));
	//if(m.empty()) return;
	auto m2=flatten(m);
	auto c=count(m2);
	return to_map(mapf([=](auto x){ return make_pair(x.first,double(x.second)/m2.size()); },c));
}

static void award_points(TBA_fetcher &tba_fetcher){
	for(auto year:years()){
		auto x=award_points(tba_fetcher,year);
		PRINT(year);
		PRINT(x);
		//Note: Distribution is noticably different depending on the ruleset for the year
		//Like post 2022, seems like winning EI/Chairmans+anything else is not possible
		//but it does not seem super jumpy on a year to year basis.
	}
}

//would be interesting to see how much correlation there is between award points & elimination points

static optional<std::vector<Point>> award_plus_elim(TBA_fetcher &tba_fetcher,tba::Event_key event){
	auto e=tba::event_district_points(tba_fetcher,event);
	if(!e){
		return std::nullopt;
	}
	assert(e);
	return mapf([](auto x){ return Point(x.award_points+x.elim_points); },values(e->points));
}

static map<Point,double> award_plus_elim(TBA_fetcher &tba_fetcher,tba::Year year){
	auto m=nonempty(mapf(
		[&](auto x){ return award_plus_elim(tba_fetcher,x); },
		local_district_events(tba_fetcher,year)
	));
	auto m2=flatten(m);
	auto c=count(m2);
	return to_map(mapf([=](auto x){ return make_pair(x.first,double(x.second)/m2.size()); },c));
}

static void award_plus_elim(TBA_fetcher &tba_fetcher){
	for(auto year:years()){
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
		m3|=std::array<int,2>{int(pre),int(post)};
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
	(void)distance;

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

std::multiset<tba::Award_type> award_types(TBA_fetcher &f,tba::Event_key event){
	return to_multiset(mapf([](auto x){ return x.award_type; },event_awards(f,event)));
}

void by_event_size(TBA_fetcher &tba_fetcher){
	for(auto year:years()){
		using T=tuple<int,int,int,double,double>;
		map<int,vector<T>> m;
		vector<multiset<tba::Award_type>> award_types_x;
		for(auto event:local_district_events(tba_fetcher,year)){
			auto e=event_district_points(tba_fetcher,event);
			if(!e){
				cout<<"No district points: "<<event<<"\n";
				continue;
				//PRINT(year);
				//PRINT(event);
			}
			assert(e);
			auto v=mapf(
				[](auto x){
					return make_tuple(
						x.alliance_points,
						x.award_points,
						x.qual_points,
						x.elim_points,
						x.total
					);
				},
				values(e->points)
			);
			m[v.size()]|=sum(v);
			award_types_x|=award_types(tba_fetcher,event);
		}

		PRINT(year);
		//print_r(m);
	
		bool show_awards=0;
		if(show_awards && !all_equal(award_types_x)){
			//auto s=to_set(award_types_x);
			/*auto always=and_all(s);
			PRINT(always);
			auto sometimes=or_all(s)-always;
			PRINT(sometimes);*/
			auto f=to_set(flatten(award_types_x));
			vector<tuple<tba::Award_type,size_t,size_t>> v;
			for(auto type:f){
				auto used=mapf([=](auto x){ return x.count(type); },award_types_x);
				if(!all_equal(used|1)){
					if(max(used)>1){
						cout<<"\t\t"<<type<<":"<<count(used)<<"\n";
					}else{
						cout<<"\t\t"<<type<<":"<<as_pct(mean(as_doubles(used)))<<"\n";
					}
				}
				/*auto used=count_if([=](auto x){ return x.count(type); },award_types_x);
				if(used!=award_types_x.size()){
					v|=make_tuple(type,used,award_types_x.size());
					//cout<<type<<":"<<used<<"\n";
				}*/
			}
			print_lines(1,v);
		}
		
		for(auto [k,t]:m){
			auto v=mapf([](auto x){ return std::get<4>(x); },t);
			auto m1=min(v);
			auto m2=max(v);
			auto d=m2-m1;
			cout<<k<<"\t"<<v.size();
			cout<<'\t'<<mean(v)/k;
			cout<<'\t'<<d<<'\t';
			if(d){
				cout<<min(v)<<'\t'<<quartiles(v)<<'\t'<<max(v);
			}else{
				cout<<m1;
			}
			cout<<'\n';

			bool show_diff=0;
			if(show_diff && d){
				//mapf([](auto x){ return ; });
				//look through each of the component items
				#define X(NAME,N) {\
					auto m=mapf([](auto x){ return get<N>(x); },t);\
					if(min(m)!=max(m)){\
						auto d=max(m)-min(m);\
						cout<<"\t\t"<<NAME<<":"<<d<<"\t"<<min(m)<<" "<<max(m)<<"\n";\
					}\
				}
				X("alliance",0)
				X("award_points",1)
				X("qual_points",2)
				X("elim_points",3)
				#undef X
			}
		}
	}
}

int demo(TBA_fetcher& tba_fetcher){
	by_event_size(tba_fetcher);

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

	for(auto year:years()){
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
