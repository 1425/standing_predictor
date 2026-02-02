#include "dates.h"
#include<chrono>

#ifdef __unix__
#include<cxxabi.h>
#endif

#include "skill_opr.h"
#include "../tba/tba.h"
#include "tba.h"
#include "set.h"
#include "print_r.h"
#include "declines.h"
#include "vector_void.h"
#include "optional.h"
#include "array.h"
#include "util.h"
#include "event.h"

using namespace std;
using Team_key=tba::Team_key;
using Event_key=tba::Event_key;
using District_key=tba::District_key;

template<typename K,typename V>
auto adjacent_pairs(std::map<K,V> const& a){
	return adjacent_pairs(to_vec(a));
}

template<typename A,typename B,typename C,typename D,typename E,typename F>
auto zip(std::tuple<A,B,C,D,E,F> const& a,std::tuple<A,B,C,D,E,F> const& b){
	return std::make_tuple(
		#define X(N) make_pair(get<N>(a),get<N>(b))
		X(0),X(1),X(2),X(3),X(4),X(5)
		#undef X
	);
}

template<typename A,typename B,typename C,typename D,typename E,typename F>
auto sum(std::tuple<A,B,C,D,E,F> const& a){
	return get<0>(a)+get<1>(a)+get<2>(a)+get<3>(a)+get<4>(a)+get<5>(a);
}

template<typename A,typename B,typename C,typename D,typename E,typename F>
size_t match_degree(std::tuple<A,B,C,D,E,F> const& a,std::tuple<A,B,C,D,E,F> const& b){
	return sum(mapf([](auto p){ return p.first==p.second; },zip(a,b)));
}

template<typename A,typename B,typename C,typename D,typename E,typename F>
std::ostream& operator<<(std::ostream& o,std::tuple<A,B,C,D,E,F> const& a){
	o<<"t(";
	#define X(N) o<<get<N>(a)<<"/";
	X(0) X(1) X(2) X(3) X(4) X(5)
	#undef X
	return o<<")";
}

std::string demangle(const char *s){
	assert(s);
	#ifdef __unix__
	int status;
	char *ret=abi::__cxa_demangle(s,0,0,&status);
	assert(ret);
	return std::string(ret);
	#else
	return s;
	#endif
}

template<typename T>
string type_string(T const& x){
	return demangle(typeid(x).name());
}

template<typename T>
struct Interval{
	T min,max;//both inclusive

	/*auto size()const{
		if an integer-like type should give max-min+1
		if a float-like type should give max-min+std::numeric_limits::lowest()
	}*/
};

template<typename T>
std::ostream& operator<<(std::ostream& o,Interval<T> const& a){
	o<<"("<<a.min<<","<<a.max<<")";
	return o;
}

std::ostream& operator<<(std::ostream& o,Interval<tba::Date> const& i){
	auto [a,b]=i;
	if(a.year()!=b.year()){
		return o<<"("<<a<<"-"<<b<<")";
	}
	unsigned ad=static_cast<unsigned>(a.day()),bd=static_cast<unsigned>(b.day());
	if(a.month()==b.month()){
		return o<<a.month()<<" "<<ad<<"-"<<bd;
	}
	return o<<a.month()<<" "<<ad<<" - "<<b.month()<<" "<<bd;
}

//std::ostream& operator<<(std::ostream& o,Interval<std::chrono::year_month_day>);

auto operator-(std::chrono::year_month_day a,std::chrono::year_month_day b){
	return std::chrono::sys_days(a)-std::chrono::sys_days(b);
}

std::ostream& operator<<(std::ostream& o,std::chrono::time_zone const& a){
	o<<"timezone(";
	o<<a.name();//<<" "<<a.get_info();
	return o<<")";
}

std::ostream& operator<<(std::ostream& o,std::chrono::time_zone const * const x){
	if(!x){
		return o<<"NULL";
	}
	return o<<*x;
}

using Year=tba::Year;

std::string link(tba::Event_key const& event,std::string const& body){
	return link("https://www.thebluealliance.com/event/"+event.get(),body);
}

std::string link(tba::Event const& event,std::string const& body){
	return link(event.key,body);
}

struct Country{
	std::string s;

	bool operator==(const char *s1){
		if(!s1){
			return 0;
		}
		return s==s1;
	}

	bool operator==(Country const& a)const{
		return s==a.s;
	}

	auto operator<=>(Country const&)const=default;
};

std::ostream& operator<<(std::ostream& o,Country const& a){
	return o<<a.s;
}

struct State_prov{
	std::string s;

	auto operator<=>(State_prov const&)const=default;
};

std::ostream& operator<<(std::ostream& o,State_prov const& a){
	return o<<a.s;
}

static const std::set<std::string> STATE_CODES{
	"AL", "AK", "AZ", "AR", "CA", "CO", "CT", "DE", "FL", "GA", 
	"HI", "ID", "IL", "IN", "IA", "KS", "KY", "LA", "ME", "MD", 
	"MA", "MI", "MN", "MS", "MO", "MT", "NE", "NV", "NH", "NJ", 
	"NM", "NY", "NC", "ND", "OH", "OK", "OR", "PA", "RI", "SC", 
	"SD", "TN", "TX", "UT", "VT", "VA", "WA", "WV", "WI", "WY",
	"DC" //omitting outlying territories, etc.
};

std::optional<Country> normalize_country(std::optional<std::string> s1){
	if(!s1){
		return std::nullopt;
	}
	auto s=*s1;
	if(s=="US" || s=="United States" || s=="Usa" || s=="usa" || s=="YSA" || s=="San Jose"){
		s="USA";
	}
	if(s=="Turkiye" || s=="Tu00fcrkiye" || s=="Türkiye"){
		s="Turkey";
	}
	if(s=="CN"){
		s="China";
	}
	if(s=="AU" || s=="Austrialia" || s=="Australia "){
		s="Australia";
	}
	if(s=="Chinese Taipei" || s=="XX"){
		s="Taiwan";
	}
	if(s=="CA"){
		s="Canada";
	}
	if(s=="Northern Israel"){
		s="Israel";
	}
	std::set<std::string> known{
		"USA","Canada","Israel","Mexico",
		"China","Brazil","Australia","Turkey",
		"Taiwan"
	};
	if(known.count(s)){
		return Country(s);
	}
	try{
		//looks like someone put a zipcode in instead.
		stoi(s);
		return std::nullopt;
	}catch(...){}

	//obviously invalid; replace with correct value
	if(s=="San Jose" || s=="Suffolk "){
		return Country("USA");
	}

	//PRINT(s1);
	//throw "what?";
	assert(s1);
	return Country(*s1);
}

std::optional<State_prov> normalize_state(std::string s){
	using R=std::optional<State_prov>;
	if(s=="Florida" || s=="Fl"){
		return R("FL");
	}
		if(s=="Oregon"){
			return R("OR");
		}
		if(s=="California" || s=="CA " || s=="California "){
			return R("CA");
		}
		if(s=="New York"){
			return R("NY");
		}
		if(s=="Maryland"){
			return R("MD");
		}
		if(s=="Minnesota" || s=="mn" || s=="Mn"){
			return R("MN");
		}
		if(s=="Illinois"){
			return R("IL");
		}
		if(s=="Colorado"){
			return R("CO");
		}
		if(s=="Ohio" || s=="OHI"){
			return R("OH");
		}
		if(s=="MS "){
			return R("MS");
		}
		if(s=="Missouri"){
			return R("MO");
		}
		if(s=="Louisiana"){
			return R("LA");
		}
		if(s=="Michigan"){
			return R("MI");
		}
		if(s=="North Carolina"){
			return R("NC");
		}
		if(s=="Arkansas"){
			return R("AR");
		}
		if(s=="Wisconsin"){
			return R("WI");
		}
		if(s=="St. Louis"){
			return R("MO");
		}

	//Canada
	if(s=="Ontario"){
		return R("ON");
	}

	if(s=="XX"){
		return std::nullopt;
	}

	return State_prov(s);
}

std::optional<State_prov> normalize_state(std::optional<std::string> const& a){
	if(!a){
		return std::nullopt;
	}
	return normalize_state(*a);
}

Country get_country(tba::Event x){
			if(x.key=="2007br"){
				return Country("Brazil");
			}
			if(x.key=="2006ca"){
				return Country("USA");
			}
			if(x.key=="2007az"){
				return Country("USA");
			}
			if(x.key=="2007ct"){
				return Country("USA");
			}
			if(x.key=="2007co"){
				return Country("USA");
			}
			if(x.key=="2007fl"){
				return Country("USA");
			}
			if(x.key=="2007ca"){
				return Country("USA");
			}
			if(x.key=="2013mhsl"){
				return Country("USA");
			}
			if(x.key=="2014bfbg"){
				return Country("USA");
			}
			if(x.key=="2013rsr"){
				return Country("USA");
			}
			if(x.key=="2014mshsl"){
				return Country("USA");
			}
			if(x.key=="2013mshsl"){
				return Country("USA");
			}
			if(x.key=="2013mm"){
				return Country("USA");
			}
			if(x.key=="2014audd"){
				return Country("Australia");
			}
			if(x.key=="2017onsc" || x.key=="2022oncne" || x.key=="2022onsc" || x.key=="2022onsc2"){
				return Country("Canada");
			}
			if(x.key=="2022xxma2" || x.key=="2022xxmac"){
				return Country("Australia");
			}
			if(x.key=="2022xxrio" || x.key=="2023xxrio"){
				return Country("Brazil");
			}
			if(x.key=="2023mexas"){
				return Country("Mexico");
			}
			if(x.key=="2023oncne" || x.key=="2023onsc" || x.key=="2023onsc3"){
				return Country("Canada");
			}
			if(x.key=="2024isios" || x.key=="2024isos2"){
				return Country("Israel");
			}
			if(x.key=="2024tacy"){
				return Country("Taiwan");
			}
			//print_r(x);
			auto n=normalize_country(x.country);
			if(!n){
				if(!x.state_prov){
					print_r(x);
					return Country("");
				}
				assert(x.state_prov);
				if(STATE_CODES.count(*x.state_prov)){
					return Country("USA");
				}else{
					nyi
				}
			}

			/*try{
				return make_tuple(normalize_country(x.country),*x.state_prov,*x.city);
			}catch(...){
				cout<<"country: \""<<x.country<<"\"\n";
				print_r(x);
				//assert(0);
				return nullopt;
			}*/
			return *n;
}

int get_timezone(State_prov const& a){
	//This is very approximate.
	//also ignoring timezones.
	
	std::set<string> pacific{
		"CA","NV","OR","WA",

		//Canada
		"BC"
	};
	std::set<string> mountain{
		"AZ","CO","ID","KS","MT","NE",
		"NV","NM","ND","SD","UT","WY",
		//some of Oregon, TX

		//Canada
		"AB"
	};
	std::set<string> central{
		"AL","AR","IL","IA","KS","KY",
		"LA","MN","MS","MO","NE","ND",
		"OK","SD","TN","TX","WI"
	};
	std::set<string> eastern{
		"CT","DE","FL","GA","IN","KT",
		"ME","MD","MA","MI","NH","NJ",
		"NY","NC","OH","PA","RI","SC",
		"TN","VT","VA","WV",

		//Not a state
		"DC",

		//Canada
		"ON","QC"
	};
	std::set<string> alaska{"AK"};//most of it, anyway
	std::set<string> hawaii{"HI"};

	#define X(A,B) if(A.count(a.s)) return B;
	X(hawaii,-10)
	X(alaska,-9)
	X(pacific,-8)
	X(mountain,-7)
	X(central,-6)
	X(eastern,-5)
	#undef X

	PRINT(a);
	assert(0);
}

int get_timezone(Country a,std::optional<State_prov> state){
	/* This only needs to be very roughly correct so that events that
	 * happen between like 8am and 8pm end up on the correct date. */

	if(a=="USA" || a=="Canada"){
		if(state){
			return get_timezone(*state);
		}
		//Pacific
		return -8;
	}
	if(a=="Australia"){
		return 11; //Sydney summer
	}
	if(a=="China" || a=="Taiwan"){
		return 8;
	}
	if(a=="Mexico"){
		//Mexico City
		return -6;
	}
	if(a=="Brazil"){
		return -3;
	}
	if(a=="Israel"){
		return 2;
	}
	if(a=="Turkey"){
		return 3;
	}
	PRINT(a);
	return 0;
}

int get_timezone(tba::Event const& event){
	auto country=get_country(event);
	auto state=[=]()->std::optional<State_prov>{
		if(event.state_prov){
			return normalize_state(*event.state_prov);
		}
		return std::nullopt;
	}();
	return get_timezone(country,state);
}

//returns whether the dates look ok; 0=ok
bool examine_event(TBA_fetcher& f,tba::Event event){
		bool shown=0;
		auto show=[&](){
			if(shown){
				return;
			}
			PRINT(event.key);
			cout<<event.key<<"\t"<<event.start_date<<"\t"<<event.end_date<<"\n";
			shown=1;
		};

		assert(event.start_date);
		assert(event.end_date);

		auto scheduled_dates=range_inclusive(*event.start_date,*event.end_date);

		using YMD=std::chrono::year_month_day;
		std::set<YMD> dates;
		auto add_date=[&](std::chrono::year_month_day a){
			dates|=a;
		};

		for(auto m:tba::event_matches(f,event.key)){
			std::vector<time_t> times;
			times|=m.time;
			times|=m.actual_time;
			times|=m.predicted_time;
			times|=m.post_result_time;

			//if(!m.time && !m.actual_time && !m.predicted_time && !m.post_result_time){
			if(times.empty()){
				continue;
			}

			for(auto time:times){
				//show();
				auto x=std::chrono::system_clock::from_time_t(time);
				auto days_since_epoch = std::chrono::floor<std::chrono::days>(x);

				std::chrono::year_month_day ymd{days_since_epoch};
				//cout<<ymd<<"\n";
				add_date(ymd);
				/*auto year = static_cast<int>(ymd.year());
				auto month = static_cast<unsigned>(ymd.month());
				auto day = static_cast<unsigned>(ymd.day());
				PRINT(year) PRINT(month) PRINT(day)*/
			}
			if(!times.empty()){
				//delay in seconds
				auto diff=max(times)-min(times);
				if(diff>3600){
					/*show();
					PRINT(diff);
					print_lines(times);
					nyi*/
				}
				//PRINT(diff);
			}

			//print_r(m);
			//nyi
		}

		//remove obvious nonsense.
		//dates-=std::chrono::year_month_day{std::chrono::year{1900}/1/1};

		auto out_of_bounds=dates-scheduled_dates;
		if(!out_of_bounds.empty()){
			return 1;
			show();
			PRINT(event);
			cout<<"schedule:\t"<<scheduled_dates<<"\n";
			cout<<"matches:\t"<<dates<<"\n";
		}
		//assert(out_of_bounds.empty());
		//if(shown) PRINT(dates);
	return 0;
}

auto all_events(TBA_fetcher &f){
	return flatten(mapf([&](auto year){ return tba::events(f,year); },years()));
}

void show_duration(std::chrono::duration<long int,std::ratio<1,1000*1000*1000>> a){
	std::chrono::hh_mm_ss hms{a};
	cout<<hms;
}

using Time_ns=std::chrono::duration<long int,std::ratio<1,1000*1000*1000>>;

std::ostream& operator<<(std::ostream& o,Time_ns const& a){
	std::chrono::hh_mm_ss hms{a};
	return o<<hms;
}

void match_timings(TBA_fetcher &f){
	//First, look at which of the times get listed
	#if 0
	using T=tuple<bool,bool,bool,bool>;
	std::vector<T> v;
	for(auto const& event:all_events(f)){
		auto matches=event_matches(f,event.key);
		for(auto const& m:matches){
			v|=T(m.time,m.actual_time,m.predicted_time,m.post_result_time);
		}
	}
	PRINT(count(v));
	//if there are any times, m.time exists.
	#endif

	/*auto example_matches=[&](){
		std::vector<tba::Match> r;
		for(auto const& event:all_events(f)){
			auto matches=event_matches(f,event.key);
			r|=matches;
			if(r.size()>10000){
				return r;
			}
		}
		return r;
	}();

	PRINT(example_matches.size());

	auto times=mapf([](auto const& x){ return x.time; },example_matches);
	PRINT(times.size());
	*/

	auto times=[&](){
		vector<time_t> r;
		for(auto const& event:all_events(f)){
			for(auto const& match:event_matches(f,event.key)){
				r|=match.actual_time;
				if(r.size()>10000){
					return r;
				}
			}
		}
		return r;
	}();

	auto m=mapf([](auto x){ return std::chrono::system_clock::from_time_t(x); },to_set(times));

	print_lines(take(5,m));

	//TODO: For each event, look at each day of competition and look at the earliest and latest matches

	vector<Time_ns> offsets;

	for(auto x:m){
		//std::chrono::hh_mm_ss hms(x.time_since_epoch());
		auto d=x.time_since_epoch();
		auto days=std::chrono::duration_cast<std::chrono::days>(d);
		auto left=d-days;

		//PRINT(hms);
		//PRINT(days);
		std::chrono::hh_mm_ss hms(left);
		//PRINT(left);
		//PRINT(hms);
		//PRINT(type_string(x));
		//
		offsets|=left;
	}

	cout<<"Deciles:\n";
	for(auto x:deciles(offsets)){
		cout<<"\t";
		show_duration(x);
		cout<<"\n";
	}

	map<int,unsigned> density;
	for(auto x:offsets){
		density[std::chrono::duration_cast<std::chrono::hours>(x).count()]++;
	}
	print_lines(density);

	/*auto elapsed=mapf([](auto x){ return x.second-x.first; },adjacent_pairs(m));

	for(auto x:elapsed){
		show_duration(x);
		cout<<"\n";
	}*/

	/*for(auto [a,b]:adjacent_pairs(m)){
		auto elapsed=b-a;
		//PRINT(elapsed);
		//auto d=std::chrono::duration_cast<std::chrono::minutes>(elapsed);
		//PRINT(d);
		show_duration(elapsed);
		cout<<"\n";
	}*/

	nyi
}

struct Match_positions{
	int days_scheduled;
	using Day=long int;
	struct Day_info{
		using Time=Time_ns;
		Time start,end;
		int matches;
	};
	std::map<Day,Day_info> days;
};

std::ostream& operator<<(std::ostream& o,Match_positions::Day_info const& a){
	o<<"Day_info(";
	o<<a.start<<" "<<a.end<<" "<<a.matches;
	return o<<")";
}

std::ostream& operator<<(std::ostream& o,Match_positions const& a){
	o<<"Match_positions(";
	o<<a.days_scheduled<<" "<<a.days;
	return o<<")";
}

std::optional<Match_positions> match_schedule(TBA_fetcher &f,tba::Event_key event){
	auto event_data=tba::event(f,event);
	auto timezone=get_timezone(event_data);

	auto m=event_matches(f,event);
	auto times=sorted(nonempty(mapf([](auto x){ return x.time; },m)));
	if(times.empty()){
		return std::nullopt;
	}

	auto from=[=](auto x){
		return std::chrono::system_clock::from_time_t(x+3600*timezone);
	};

	assert(event_data.start_date);
	assert(event_data.end_date);

	const auto playing=to_set(range_inclusive(*event_data.start_date,*event_data.end_date));
	auto days_scheduled=playing.size();
	//PRINT(days_scheduled);

	auto to_date_time=[=](time_t x)->std::pair<tba::Date,Time_ns>{
		auto f=from(x);
		//PRINT(f);
		//PRINT(type_string(f));
		//auto date=std::chrono::year_month_day(f);
		auto date=std::chrono::floor<std::chrono::days>(f);
		//PRINT(date);
		//auto days=std::chrono::duration_cast<std::chrono::days>(f);
		auto residual=f-date;
		//PRINT(date);
		//PRINT(residual);
		//nyi
		return make_pair(date,residual);
	};

	auto to_day_number_time=[=](time_t x){
		auto y=to_date_time(x);
		auto day=y.first-*event_data.start_date;
		return make_pair(day.count(),y.second);
	};

	auto g=group(
		[](auto x){ return x.first; },
		mapf(to_day_number_time,times)
	);

	auto r=map_values(
		[](auto x){
			auto times_of_day=seconds(x);
			return Match_positions::Day_info(min(times_of_day),max(times_of_day),x.size());
		},
		g
	);
	return Match_positions(days_scheduled,r);
	/*print_r(r);
	nyi*/
	#if 0
	auto check_date=[&](auto x){
		auto days_since_epoch = std::chrono::floor<std::chrono::days>(x);
		std::chrono::year_month_day ymd{days_since_epoch};
		//std::chrono::year_month_day ymd{std::chrono::sys_days(x)};
		if(!playing.count(ymd)){
			PRINT(playing);
			PRINT(ymd);
		}
		return playing.count(ymd);
	};

	return check_date(from(min(times))) && check_date(from(max(times)));
	#endif

	//PRINT(from(min(times)));
	//PRINT(from(max(times)));

	/*for(auto y:adjacent_pairs(times)){
		PRINT(y.second-y.first);
	}*/

	//exit(0);
}

struct Name_contents{
	std::set<string> sponsors,orgs;

	auto operator<=>(Name_contents const&)const=default;
};

std::ostream& operator<<(std::ostream& o,Name_contents const& a){
	o<<"Name_contents(";
	o<<a.sponsors;
	o<<a.orgs;
	return o<<")";
}

Name_contents parse_name(std::string const& name){
	//Note that this is very approximate.
	//because the delimiters are chars that can appear in the names of
	//financial sponsors and organizations.
	
	auto sp=split(name,'/');
	assert(sp.size()>=1);

	auto sponsors=take(sp.size()-1,sp);

	auto sp2=split(last(sp),'&');
	assert(sp2.size()>=1);

	vector<string> orgs;
	if(sp2.size()==1){
		orgs=sp2;
	}else{
		sponsors|=sp2[0];
		orgs=skip(1,sp2);
	}

	return Name_contents(to_set(sponsors),to_set(orgs));
}

auto parse_name(optional<string> const& a){
	assert(a);
	return parse_name(*a);
}

auto parse_name(tba::Team const& t){
	return parse_name(t.name);
}

std::vector<tba::Team> teams_year_all(TBA_fetcher &f,Year year){
	std::vector<tba::Team> r;
	size_t page=0;
	while(1){
		auto found=teams_year(f,year,page);
		r|=found;
		page++;

		if(found.empty()){
			break;
		}
	}
	return r;
}

std::vector<tba::Team> all_teams(TBA_fetcher &f){
	//just asking for one year because it actually doesn't give different results for different years.
	return teams_year_all(f,Year(2026));
}

void check_sponsors(TBA_fetcher& f){
	//This is useless because it just gives all the teams all the time 
	//with the same info.
	map<Team_key,std::map<Year,Name_contents>> r;
	for(auto year:years()){
		auto teams=teams_year_all(f,Year(2025));
		for(auto const& team:teams){
			r[team.key][year]=parse_name(team.name);
		}
	}
	for(auto [team,info]:r){
		PRINT(team);
		PRINT(info.size());
		for(auto [a,b]:adjacent_pairs(info)){
			if(a.second!=b.second){
				diff(a.second,b.second);
			}
		}

		//print_lines(info);
	}
}

auto find_hosts(TBA_fetcher &f){
	//See if we can figure out if there is a specific team that might be hosting the event.
	map<tba::Event_key,std::set<Team_key>> r;
	for(auto event:reversed(all_events(f))){

		auto get_loc=[&](auto x){
			return make_tuple(
				x.city,
				normalize_state(x.state_prov),
				normalize_country(x.country),
				x.address,
				x.postal_code,
				x.location_name
			);
		};
		(void)get_loc;

		auto teams=event_teams(f,event.key);

		/*auto matched=reversed(sorted(mapf(
			[&](auto x){ return make_pair(match_degree(get_loc(x),get_loc(event)),x); },
			teams
		)));

		matched=filter([](auto x){ return x.first>2; },matched);*/

		if(event.location_name){
			auto f2=filter([=](auto x){ return parse_name(x).orgs.count(*event.location_name); },teams);

			r[event.key]=to_set(mapf([](auto x){ return x.key; },f2));
		}

	}
	return r;
}

void common_sponsors(TBA_fetcher &f){
	//Figure out what the most common sponsors are for teams.
	std::multiset<string> sponsors,orgs;
	std::multiset<int> num_sponsors,num_orgs;

	for(auto team:all_teams(f)){
		auto p=parse_name(team);
		sponsors|=p.sponsors;
		orgs|=p.orgs;

		num_sponsors|=p.sponsors.size();
		num_orgs|=p.orgs.size();
	}

	PRINT(quartiles(num_sponsors));
	PRINT(quartiles(num_orgs));

	auto show=[](string label,auto x){
		auto c=reversed(sorted(swap_pairs(count(x))));
		cout<<label<<"\n";
		for(auto x:take(20,c)){
			cout<<"\t"<<x<<"\n";
		}
	};
	show("sponsors",sponsors);
	show("orgs",orgs);
}

using Day=int;

using Dates_result=std::map<
	Day,
	map<Day,pair<Time_ns,Time_ns>>
>;

Dates_result event_times_inner(TBA_fetcher&);

Dates_result event_times(TBA_fetcher &f){
	static auto r=event_times_inner(f);
	return r;
}

Dates_result event_times_inner(TBA_fetcher &f){
	/*common_sponsors(f);
	return 0;*/

	/*auto h=find_hosts(f);
	print_r(h);*/

	/*Interesting questions to ask about schedules
	 * 1) typical lengths scheduled (overall dist)
	 * 2) for each length, what's a typical distribution of # of matches on each day
	 * 3) categorize events by which days there are matches
	 * 4) for each of those categories: look at the typical beginning and ending times for each day
	 *
	 * */

	std::map<int,std::vector<Match_positions>> found;

	for(auto const& event:all_events(f)){
		auto b=match_schedule(f,event.key);
		if(b){
			auto c=*b;
			found[c.days_scheduled]|=c;
		}
	}

	#if 0
	for(auto [length,events]:found){
		cout<<"Events of length "<<length<<"\n";
		multiset<long int> days_played;
		for(auto x:events){
			days_played|=keys(x.days);
		}
		PRINT(count(days_played));

		for(auto day:to_set(days_played)){
			vector<Match_positions::Day_info> here;
			for(auto x:events){
				auto f=x.days.find(day);
				if(f!=x.days.end()){
					here|=f->second;
				}
			}
			cout<<day<<" "<<here.size()<<"\n";
			auto starts=mapf([](auto x){ return x.start; },here);
			auto ends=mapf([](auto x){ return x.end; },here);
			auto matches=mapf([](auto x){ return x.matches; },here);
			cout<<"\t"<<quartiles(starts)<<"\n";
			cout<<"\t"<<quartiles(ends)<<"\n";
			cout<<"\t"<<quartiles(matches)<<"\n";
		}
	}
	#endif

	return map_values(
		[=](auto events){
			multiset<long int> days_played;
			for(auto event:events){
				days_played|=keys(event.days);
			}
			set<long int> days_to_care;
			for(auto [day,n]:count(days_played)){
				if(n>events.size()/3){
					days_to_care|=day;
				}
			}
			map<Day,pair<Time_ns,Time_ns>> r;
			for(auto day:days_to_care){
				vector<Match_positions::Day_info> here;
				for(auto x:events){
					auto f=x.days.find(day);
					if(f!=x.days.end()){
						here|=f->second;
					}
				}
				auto starts=mapf([](auto x){ return x.start; },here);
				auto ends=mapf([](auto x){ return x.end; },here);
				r[day]=make_pair(median(starts),median(ends));
			}
			//PRINT(r);
			return r;
		},
		found
	);

	//match_timings(f);
	//choose some event and then for each of the days that it says it's going on 
	//figure out which matches are that day
	//also, would be interesting to know when the awards are given out, but there may not be data for that

	//The interesting thing about this is that we get times in with UTC, but we don't actually know the timezone for 
	//the places that each of the events are
	//so we don't actually know what day things are happening
	//but we can probably approximate this be just having some sort of timezone for each country that participates
	//because we have a pretty good idea that matches are not going to be played in the hours near midnight.
	//In fact, it may be possible to infer a timezone of a location based on the hours that matches are played.
	//
	/*
	 * Step 1:
	 *   Look at all the events ever and find the country/state/city tuple
	 *
	 * look at the time of day that each event finishes? -> last match?
	 *
	 *
	 * Where would like to end up:
	 * For each date range before cmp, what do the predictions look like?
	 * 
	 * For each event:
	 * have a model what is its current status:
	 * 1) not started
	 * 2) in progress
	 * 3) complete
	 * over time
	 *
	 * And to know when to expect these to change in the future would want to know when matches happen in whatever
	 * time zone and how long the awards ceremony tends to be
	 *
	 * this will allow having a good idea at what specific times to look for events to end
	 * and therefore when the odds for different teams are expected to update.
	 *   */

	const auto all_events1=all_events(f);

	auto m=mapf(
		//[&](auto x)->std::optional<tuple<string,string,string>>{
		[&](auto const& x){
			auto c=get_country(x);
			if(c=="USA"){
				if(x.state_prov){
					auto n=normalize_state(*x.state_prov);
					if(!n || !STATE_CODES.count(n->s)){
						print_r(x);
						cout<<"state \""<<x.state_prov<<"\"\n";
					}
				}
			}
			return c;
		},
		all_events1
	);

	PRINT(m.size());
	PRINT(to_set(m).size());

	//auto m1=mapf(get_timezone,to_set(m));
	//PRINT(m1);

	//print_lines(sorted(m));
	//PRINT(countries);

	//now examine the matches of an event and try to figure out what time of day they were.
	for(auto const& event:all_events1){
		(void)event;
		//examine_event(f,event);
	}
	#if 0
	{
		auto m=mapf([&](auto x){ return examine_event(f,x); },all_events1);
		PRINT(count(m));
	}
	for(auto [k,v]:group([](auto x){ return get_country(x); },all_events1)){
		auto m=mapf([&](auto x){ return examine_event(f,x); },v);
		cout<<k<<"\t";
		PRINT(count(m));
	}
	#endif

	// Prints UTC and local time.
	const auto tp_utc{std::chrono::system_clock::now()};
	std::cout << "Current time 'UTC' is: " << tp_utc << "\n"
                 "Current time 'Local' is: "
              << std::chrono::current_zone()->to_local(tp_utc) << '\n';

	auto z=std::chrono::current_zone();
	
	PRINT(z);

	nyi//return 0;
}

//should always return between 1-7
int days(tba::Event const& a){
	assert(a.start_date);
	assert(a.end_date);
	return (*a.end_date-*a.start_date).count()+1;
}

//returns std::chrono::time_point
auto operator+(std::chrono::year_month_day a,Time_ns b){
	return std::chrono::sys_days(a)+b;
}

std::chrono::year_month_day operator+(
	std::chrono::year_month_day a,
	std::chrono::duration<long,std::ratio<86400,1>> b
){
	return std::chrono::sys_days(a)+b;
}

using Time=std::chrono::time_point<std::chrono::_V2::system_clock, std::chrono::duration<long int, std::ratio<1, 1000000000> > >;

auto expected_running(TBA_fetcher &f,Event_key event){
	//Note: Timezone for all of these results is local to the event!

	//cout<<"expected_running("<<event<<")\n";

	//figure out timestamps for when expect the event to start and end
	//event_times(f,)
	//probably want to give some time for the awards to show up after the last match is played.
	//if looking for when all the results are going to come in.
	auto event_data=tba::event(f,event);
	auto d=days(event_data);
	auto times=event_times(f);
	auto found=times.find(d);
	if(found==times.end()){
		cout<<"days:"<<d<<"\n";
	}
	assert(found!=times.end());
	auto a=found->second;

	auto start=*event_data.start_date;
	std::vector<Interval<Time>> r;
	for(auto [day,time_range]:a){
		auto [time_start,time_end]=time_range;
		auto d2=std::chrono::days(day);
		auto s=start+d2+time_start;
		auto e=start+d2+time_end;
		using T=decltype(s);
		auto x=Interval<T>{s,e};
		r|=x;
	}
	return r;
}

int as_date(int);

std::chrono::year_month_day as_date(Time a){
	return std::chrono::floor<std::chrono::days>(a);
	//auto days=std::chrono::duration_cast<std::chrono::days>(a);
	//return std::chrono::year_month_day(std::chrono::sys_days(days));
	PRINT(type_string(a));
	nyi
}

auto expected_running(TBA_fetcher &f,Year year){
	auto e=tba::events(f,year);
	auto m=mapf([&](auto x){ return make_pair(x,expected_running(f,x)); },keys(e));
	//for now, going to ignore that all of the timezones are different for the results.
	using Date=std::chrono::year_month_day;
	map<
		Date,
		std::vector<std::pair<Event_key,Interval<Time>>>
	> by_date;
	//could also look at this as for each time interval, what are the changes that happen
	//at any time, would be interesting to know how many events are going on
	//for any interval of time, what are the things going on?
	//when are the times where no event are actively happening?


	for(auto [event,days]:m){
		for(auto interval:days){
			by_date[as_date(interval.min)]|=make_pair(event,interval);
		}
	}

	return by_date;
}

template<typename T>
auto consolidate(std::vector<T> );

std::chrono::year_month_day operator+(std::chrono::year_month_day a,int b){
	//interpreting b as a number of days.
	return a+std::chrono::days(b);
}

template<typename T>
auto consolidate(std::set<T> const& a){
	std::vector<Interval<T>> r;
	std::optional<T> start;
	std::optional<T> last;
	for(auto x:a){
		if(last){
			if(x==*last+1){
				last=x;
			}else{
				r|=Interval<T>{*start,*last};
				start=last=x;
			}
		}else{
			start=last=x;
		}
	}
	if(start){
		r|=Interval<T>(*start,*last);
	}
	return r;
}

bool cmp(tba::Event_type a){
	return a==tba::Event_type::CMP_DIVISION || a==tba::Event_type::CMP_FINALS;
}

std::optional<std::tuple<string,string,string>> parse_district_event_name(std::string const& s){
	auto sp=split(s);
	auto district_abbrev=sp[0];
	if(sp[1]!="District"){
		return std::nullopt;
	}
	vector<string> name;
	size_t i=2;
	while(i<sp.size() && sp[i]!="Event"){
		name|=sp[i];
		i++;
	}
	assert(i<sp.size());
	i++;
	vector<string> sponsor;
	if(i<sp.size()){
		assert(sp[i]=="presented");
		i++;
		assert(sp[i]=="by");
		i++;
		while(i<sp.size()){
			sponsor|=sp[i];
			i++;
		}
	}
	return std::make_tuple(district_abbrev,join(" ",name),join(" ",sponsor));
}

auto abbrev(District_key a){
	return a.get().substr(4,100);
}

auto display_name(TBA_fetcher& f,District_key district){
	auto a=abbrev(district);
	auto x=tba::history(f,a);
	auto found=filter([=](auto x){ return x.year==year(district); },x);
	assert(found.size()==1);
	auto f1=found[0];
	//PRINT(f1);
	return f1.display_name;
}

auto district_display_names(TBA_fetcher &f){
	//could have this go through all the years
	auto d=tba::districts(f,Year(2026));
	return mapf([](auto x){ return x.display_name; },d);
}

std::optional<tuple<string,string,string>> parse_dcmp_name(TBA_fetcher &f,std::string const& s){
	auto n=district_display_names(f);
	auto found=filter([=](auto x){ return prefix(s,x); },n);
	switch(found.size()){
		case 0:
			return std::nullopt;
		case 1:{
			auto d=found[0];
			auto sp=split(s.substr(d.size(),s.size()));
			if(sp[0]=="FIRST"){
				sp=skip(1,sp);
			}
			vector<string> name;
			size_t i=0;
			while(i<sp.size() && sp[i]!="presented"){
				name|=sp[i];
				i++;
			}
			vector<string> sponsor;
			if(i<sp.size()){
				i++;
				assert(sp[i]=="by");
				i++;
				while(i<sp.size()){
					sponsor|=sp[i++];
				}
			}
			//can also parse an optional "presented by" at the end
			//return s.substr(d.size(),s.size());
			return make_tuple(d,join(" ",name),join(" ",sponsor));
		}
		default:
			print_lines(found);
			assert(0);
	}
}

std::string parse_event_name(TBA_fetcher &f,std::string const& s){
	{
		auto x=parse_district_event_name(s);
		if(x){
			return get<1>(*x);
		}
	}

	{
		auto x=parse_dcmp_name(f,s);
		if(x){
			return get<1>(*x);
		}
	}

	if(s=="FIRST Championship - FIRST Robotics Competition"){
		return "Worlds";
	}
	return s;

	//return s;

	//"<> District ... Event [presented by ...]";
	auto sp=split(s);
	auto district_abbrev=sp[0];
	//sp[1]=="District"
}

void schedule_demo(TBA_fetcher &f){
	District_key district("2026ca");

	auto d=sort_by(
		district_events(f,district),
		[](auto x){ return x.start_date; }
	);

	{
		auto e=events(f,year(district));
		auto f=filter([](auto x){ return x.event_type==tba::Event_type::CMP_FINALS; },e);
		d|=f;
	}

	ofstream file("what.html");
	file<<"<table border>";
	file<<tr(th("Event")+th("Status")+th("Type")+th("Date")+th("Teams"));

	for(auto event:d){
		file<<"<tr>";
		file<<td(link(event,parse_event_name(f,event.name)));
		file<<td("status");
		file<<td(event.event_type);
		//file<<td(event.start_date);
		file<<td(Interval<std::chrono::year_month_day>(*event.start_date,*event.end_date));
		//file<<td(event.end_date);
		auto teams=[&]()->int{
			switch(event.event_type){
				case tba::Event_type::DISTRICT:
					return event_teams_keys(f,event.key).size();
				case tba::Event_type::DISTRICT_CMP:{
					auto x=dcmp_size(district);
					return x[0];
				}
				case tba::Event_type::CMP_FINALS:
					return worlds_slots(district);
				default:
					assert(0);
			}
		}();
		file<<td(teams);
		file<<"</tr>";
	}
	file<<"</table>";
}

int dates_demo(TBA_fetcher &f){
	schedule_demo(f);
	return 0;

	/*Would be nice to have a listing for a district of the status
	 * For each event: 
	 *  -# of teams attending
	 *  -# eligible to earn points
	 *  -status (upcoming/in progress/complete)
	 *  -what days it's scheduled for
	 * For the DCMP:
	 *  -# of team slots
	 *  -status (upcoming/in progress/complete)
	 *  -how many of the slots are taken
	 *  -how many teams are still in the running for the remaining slots
	 * For the CMP:
	 *  -# of points slots
	 *  -status (upcoming...)
	 *  -# of slots taken
	 *  -how many teams are still in the running for remaining slots
	 *
	 * name | status | type | scheduled dates | # of slots
	 *
	 * */
	//auto e=event_times(f);
	//print_r(e);
	//auto e=expected_running(f,Event_key("2026orwil"));
	auto e=expected_running(f,Year(2025));
	//print_r(e);
	auto k=keys(e);
	cout<<"Runs where there are events happening:\n";
	for(auto x:consolidate(k)){
		auto days=x.max-x.min+std::chrono::days(1);
		cout<<x<<days<<"\n";
	}
	return 0;
}
