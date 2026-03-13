#include "fly.h"
#include "history.h"
#include "../tba/tba.h"
#include "tba.h"
#include "print_r.h"
#include "dates.h"

using namespace std;

/*
Options trading as a model for FRC team management

CD seems to be alergic to prediction markets, but have you considered the other fastest way to lose money, options trading?

Assume that if your team qualifies for the championship then they will go and that you will buy plane tickets.  You could buy tickets now and lock in the price.  But you could also wait and see both whether prices might change and how the odds of your team making the championship change.  

If you know that your team is going, your financial position is equivalent to having sold a European-style call option on plane tickets at a strike price of $0 and a date of when the championship is.  In other words, this is like being short the asset (in this case plane tickets) with a known date by which it must be covered.  

If you don't know whether or not your team is going yet, then this is equivalent to the option above, except that you know that the counterparty will exercise the call if and only if the team qualifies.  

Now that we know have outlined the problem, we can design a trading strategy.  Here are the basic parts:

1) Model the cost of flights at different times before the championship event
2) Model the probability of a team qualifying for the championship event, and how its uncertainty changes over time.  
3) Assume that expected utility is proportional to expected cost.  
4) Combine these models to calculate the best time to buy tickets cost for each team.

For the model of flight costs, I am using the conventional wisdom about US domestic flights:

1) Assign the expected cost 3+ months out 1.0 as a baseline.
2) Flights 1-3 months out are about 0.75
3) Flights 1 month to 21 days out are 1.0
4) Fligts 21 days to 7 days out ramp from 1.0 to 1.59
5) Flights 7 or fewer days are 1.59

For the model of team qualification probabilities, I have previously built one of those:

https://github.com/1425/standing_predictor/

To turn that into how team probabilities change over time, I ran that model on previous seasons and built a Markov model whose states for a team are (time left, upcoming scheduled events, quantized probability).  

 * */
using Week=int; //number of weeks before dcmp
using Year=tba::Year;
using Schedule=vector_fixed<Week,2>;

/*#define TEAM_STATUS(X)\
	X(Pr,current_p)\
	X(int,weeks_to_dcmp)\
	X(Schedule,scheduled)

STRUCT_DECLARE(Team_status,TEAM_STATUS)*/

using Local_schedule=vector_fixed<std::chrono::days,2>;

#define SEASON_SCHEDULE(X)\
	X(Local_schedule,local)\
	X(std::chrono::days,dcmp)

STRUCT_DECLARE(Season_schedule,SEASON_SCHEDULE)
PRINT_STRUCT(Season_schedule,SEASON_SCHEDULE)

std::optional<tba::District_key> district(tba::Event const& a){
	if(!a.district){
		return std::nullopt;
	}
	return a.district->key;
}

auto counting_locals(TBA_fetcher &f,tba::Team_key team,Year year){
	auto d=district(f,team,year);
	auto t=team_events_year(f,team,year);
	auto found=filter(
		[&](auto x)->bool{
			if(district(x)!=d){
				return 0;
			}
			return x.event_type==tba::Event_type::DISTRICT;
		},
		t
	);
	found=sort_by(found,[](auto x){ return x.end_date; });
	return ::take<2>(found);
}

auto end_date(tba::Event const& a){
	assert(a.end_date);
	return *a.end_date;
}

std::optional<Season_schedule> schedule(TBA_fetcher &f,tba::Team_key team,Year year){
	auto d=district(f,team,year);
	if(!d){
		//Team not in a district in that year.
		return std::nullopt;
	}
	auto ds=dcmp_end(f,*d);
	auto cs=cmp_start(f,year);
	auto local=counting_locals(f,team,year);
	return Season_schedule{
		::mapf([=](auto x){ return cs-end_date(x); },local),
		cs-ds
	};
}

auto schedule(TBA_fetcher& f,tba::Team const& a,Year year){
	return schedule(f,a.key,year);
}

/*Team_status status(TBA_fetcher &f,tba::Team_key team){
	(void)f;
	(void)team;
	nyi
}*/

using Box=int;

#define TEAM_STATE(X)\
	X(Box,p)\
	X(Week,week)\
	X(Schedule,schedule)

STRUCT_DECLARE(Team_state,TEAM_STATE)

/*Team_state simplify(Team_status){
	nyi
}*/

using Dist=std::map<int,Pr>;

using Transition_table=std::map<pair<int,bool>,std::map<int,Dist>>;

#if 0
std::map<Team_status,Pr> step(Transition_table table,Team_state a){
	(void)table;
	/*
	 * A better way to divide the boxes:
	 * 0-50
	 * 50-75
	 * 75-87.2
	 * 87.5-...
	 *
	 * odds of miss:
	 * 1 - 1/2
	 * 1/2 - 1/4
	 * 1/8
	 * 1/16
	 * 1/32
	 * 1/64
	 * 1/64 or less ?
	 *
	 * odds of cmp make ...
	 * there is going to be 
	 * really want cmp odds, not dcmp
	 * */
	assert(a.week>=0);
	for(auto elem:a.schedule){
		assert(elem<=a.week);
	}
	
	bool event_now=contains(a.schedule,a.week);
	(void)event_now;
	nyi
	#if 0
	auto next=table[(a.week,event_now)][a.p];
	return map_keys(
		[](auto x){
			Team_status t2;
			t2.p=x;
			t2.week=a.week-1;
			t2.schedule=filter([](auto x){ return x<a.week; },a.schedule);
			return t2;
		},
		next
	);
	#endif
}
#endif

using Days=std::chrono::days;
using Event=std::pair<Days,bool>;//bool=team is playing

#define TEAM_SETUP(X)\
	X(std::vector<Event>,pre_dcmp)\
	X(Days,dcmp)

STRUCT_DECLARE(Team_setup,TEAM_SETUP)
PRINT_STRUCT(Team_setup,TEAM_SETUP)

using Date=tba::Date;

auto local_events(TBA_fetcher &f,tba::District_key district){
	return filter(
		[](auto x){ return x.event_type==tba::Event_type::DISTRICT; },
		events(f,district)
	);
}

auto interesting_dates(TBA_fetcher& f,tba::District_key district){
	return to_set(mapf(end_date,local_events(f,district)));
}

auto interesting_dates(TBA_fetcher &f,std::optional<tba::District_key> const& a){
	if(!a){
		return std::set<Date>();
	}
	return interesting_dates(f,*a);
}

auto district_throw(TBA_fetcher &f,tba::Team_key team,Year year){
	auto r=district(f,team,year);
	if(!r) throw "district not found";
	return *r;
}

std::optional<Team_setup> team_setup(TBA_fetcher &f,tba::Team_key team,Year year){
	auto d=::district(f,team,year);
	if(!d){
		return std::nullopt;
	}
	auto district=*d;

	auto s=[&](){
		auto r=schedule(f,team,year);
		assert(r);
		return *r;
	}();

	auto dates=interesting_dates(f,district);
	auto c=cmp_start(f,year);
	return Team_setup{
		mapf(
			[=](auto x){
				auto offset=c-x;
				return make_pair(
					offset,
					to_set(s.local).count(offset)
				);
			},
			dates
		),
		c-dcmp_end(f,district)
	};
}

auto team_setup(TBA_fetcher &f,tba::Team const& team,Year year){
	return team_setup(f,team.key,year);
}

//using Team_status=std::tuple<Team_setup,Days,Box>;

#define TEAM_SEASON_STATUS(X)\
	X(Team_setup,setup)\
	X(Days,days)\
	X(Box,box)

STRUCT_DECLARE(Team_season_status,TEAM_SEASON_STATUS)
PRINT_STRUCT(Team_season_status,TEAM_SEASON_STATUS)

//template<typename K,typename V>
//V find_close(std::map<K,V>,K);

template<typename K,typename V>
auto max_key(std::map<K,V> const& a){
	return max(keys(a));
}

template<typename Func,typename T>
auto argmin(Func f,std::vector<T> const& v){
	auto m=min(mapf([&](auto x){ return make_pair(f(x),x); },v));
	return m.second;
}

template<typename Func,typename T>
auto argmin(Func f,set_flat<T> const& v){
	auto m=min(mapf([&](auto x){ return make_pair(f(x),x); },v));
	return m.second;
}

auto distance(int a,long b){
	return abs(a-b);
}

template<typename A,typename B,typename C,typename D>
auto distance(std::pair<A,B> a,std::pair<C,D> b){
	return make_pair(distance(a.first,b.first),distance(a.second,b.second));
}

template<typename K,typename V,typename T>
V find_close(std::map<K,V> const& a,T const& b){
	auto f=a.find(b);
	if(f!=a.end()){
		return f->second;
	}
	if(b>max_key(a)){
		return a.at(max_key(a));
	}
	//auto new_key=argmin([=](auto x){ return abs(x-b); },keys(a));
	auto new_key=argmin([=](auto x){ return distance(x,b); },keys(a));
	return a.at(new_key);
}

map<Team_season_status,Pr> step(Transition_table const& table,Team_season_status const& team_status){
	//using Transition_table=std::map<pair<int,bool>,std::map<int,Dist>>;
	auto [team_setup,days,box]=team_status;

	//if not an interesting day, then just return the same thing back out except the number of days left
	//goes down.
	
	for(auto [event_days,playing]:team_setup.pre_dcmp){
		if(days==event_days){
			auto k=make_pair(event_days.count(),playing);
			if(!table.contains(k)){
				PRINT(team_status);
				PRINT(k);
				PRINT(keys(table));
				nyi
			}
			auto found=find_close(table,k);
			/*auto found=table.at(k);
			PRINT(found);
			PRINT(box);*/
			//auto dist=found.at(box);
			auto dist=find_close(found,box);
			map<Team_season_status,Pr> r;
			for(auto [box2,p]:dist){
				Team_season_status next{team_setup,days-std::chrono::days(1),box2};
				r[next]=p;
			}
			return r;
		}
	}

	if(team_setup.dcmp==days){
		//force the probability to either 0 or 100%.
		//PRINT(team_status);

		Team_season_status a{
			team_status.setup,
			team_status.days-std::chrono::days(1),
			10
		};

		Team_season_status b=a;
		b.box=0;

		map<Team_season_status,Pr> r;
		Pr p=double(box)/10;
		r[a]=p;
		r[b]=1-p;
		return r;
	}

	//nothing happening on this day, give the same results back out
	//except that 1 day has passed.
	assert(days>=std::chrono::days(0));
	Team_season_status next{
		team_setup,
		days-std::chrono::days(1),
		box
	};
	map<Team_season_status,Pr> r;
	r[next]=1;
	return r;
	/*
	auto found=table.at(make_pair(days,box));
	assert(f!=table.end());
	auto d=f->second;
	map<Team_status,Pr> r;
	for(auto [k,v]:d){
		nyi
	}
	return r;*/
}

Year get_year(tba::Date a){
	int i=static_cast<int>(a.year());
	return Year(i);
}

template<typename Func,typename T>
auto max_by(Func f,std::vector<T> const& a){
	auto m=sorted(mapf([&](auto x){ return make_pair(f(x),x); },a));
	return last(m).second;
}

std::optional<Team_season_status> team_season_status(TBA_fetcher& f,tba::Team_key team,std::vector<History_item> const& history){
	auto now=current_date(); //this should get passed in
	auto year=get_year(now);
	//using Team_status=std::tuple<Team_setup,Days,Box>;
	auto s2=team_setup(f,team,year);
	if(!s2){
		return std::nullopt;
	}

	auto days=cmp_start(f,year)-now;
	//should probably check that days >=0

	auto f1=filter([&](auto x){ return x.team==team && x.year==year; },history);
	assert(!f1.empty());
	auto m=max_by([](auto x){ return x.date; },f1);
	auto p=m.cmp_pr;
	auto box=int(p*10);
	return Team_season_status(*s2,days,box);
}

auto team_season_status(TBA_fetcher &f,tba::Team const& team,auto const& c){
	return team_season_status(f,team.key,c);
}

bool operator==(std::chrono::days a,int b){
	return a==std::chrono::days(b);
}

bool operator!=(std::chrono::days a,int b){
	return !(a==b);
}

using Value=double;
using Result=std::pair<Days,Value>;
using Cache=std::map<Team_season_status,Result>;

double cost_function(int days_out){
	if(days_out<=7){
		return 1.59;
	}
	//at 21 days out start to spike
	if(days_out<=21){
		auto taper_width=21-7;
		auto slope=-.59/taper_width;
		//PRINT(slope);
		return 1.59+slope*(days_out-7);
	}
	//1-3 months out 25% lower (for domestic
	double months=days_out/30.0;
	if(months>1 && months<3){
		return .75;
	}
	return 1;
}

Value cost(std::chrono::days a){
	//OBVIOUSLY WANT TO MAKE THIS DIFFERENT LATER
	//return (a<std::chrono::days(20))?2:1;
	return cost_function(a.count());
}

template<typename T>
auto weighted_average(std::map<T,double> a){
	T sum{};
	double weight=0;
	for(auto [k,v]:a){
		sum+=(k*v);
		weight+=v;
	}
	return sum/weight;
}

template<typename T>
auto weighted_average(std::vector<std::pair<T,double>> a){
	T sum{};
	double weight=0;
	for(auto [k,v]:a){
		sum+=(k*v);
		weight+=v;
	}
	return sum/weight;
}

Result run_cached(Cache &cache,Transition_table const& transition_table,Team_season_status const& here){
	{
		auto f=cache.find(here);
		if(f!=cache.end()){
			return f->second;
		}
	}

	if(here.days>std::chrono::days(0)){
		auto after=step(transition_table,here);
		auto m=mapf(
			[&](auto x){
				auto [k,v]=x;
				return make_pair(
					run_cached(cache,transition_table,k),
					v
				);
			},
			after
		);
		auto m2=mapf([](auto x){ return make_pair(x.first.second,x.second); },m);
		auto cost_if_wait=weighted_average(m2);
		auto cost_if_now=cost(here.days);
		if(cost_if_now<=cost_if_wait){
			return Result{here.days,cost_if_now};
		}
		auto buy_times=mapf([](auto x){ return x.first.first; },m);
		return Result{max(buy_times),cost_if_wait};
	}

	if(here.box==0){
		return Result{0,0.0};
	}
	if(here.box==10){
		return Result{0,cost(Days(0))};
	}
	PRINT(here);
	nyi
}

using Team_key=tba::Team_key;

void run_demo(TBA_fetcher &f,Year year,std::vector<std::pair<Team_key,Team_season_status>> team_status){
	Transition_table transition_table=read_dist();
	
	Cache cache;
	for(auto [team,info]:team_status){
		auto x=run_cached(cache,transition_table,info);
		auto date=cmp_start(f,year)-x.first;
		cout<<team<<"\t"<<date<<"\t"<<x<<"\n";
	}
	return;

	/*while(team_status.days!=0){
		PRINT(team_status);
		auto after=step(transition_table,team_status);
		//PRINT(after);

		team_status=choose(keys(after));
	}
	cout<<"End state:"<<team_status<<"\n";*/
}

void run_demo(std::optional<Team_season_status> const& a){
	if(!a) return;
	return run_demo(*a);
}

auto as_int(Team_key a){
	return stoi(a.str().c_str()+3);
}

int fly_demo(TBA_fetcher &f){
	auto x=read_dist();
	Year year(2026);
	auto h=read_history();

	cout<<"Reading team status\n";

	auto e=nonempty(mapf(
		[&](auto x)->optional<pair<Team_key,Team_season_status>>{
			auto s=team_season_status(f,x,h);
			if(s){
				return make_pair(x.key,*s);
			}else{
				return std::nullopt;
			}
		},
		//filter([](auto x){ return as_int(x.key)>5000; },teams(f))
		teams(f)
	));

	cout<<"Pricing model\n";
	run_demo(f,year,e);
	cout<<"Done\n";

	if(0){
		for(auto team:teams(f)){
			auto s=schedule(f,team,year);
			auto s2=team_setup(f,team,year);
			auto s3=team_season_status(f,team,h);
			cout<<team.key<<"\t"<<s3<<"\n";

			run_demo(s3);
		}
	}

	for(auto district:districts(f,year)){
		//could print interesting dates
	}

	//1) just run through a few paths and see what happens in the end
	//2) look at current path for all the teams
	//3) run through outcomes for all of them
	//4) combine that with the ticket cost calculator
	//would be good to do some sanity checking of what the team distributions looks like 
	//after X number of weeks
	return 0;
}
