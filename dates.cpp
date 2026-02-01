#include "dates.h"
#include<chrono>
#include "skill_opr.h"
#include "../tba/tba.h"
#include "tba.h"
#include "set.h"
#include "print_r.h"
#include "declines.h"
#include "vector_void.h"
#include "optional.h"
#include "array.h"

using namespace std;
using Team_key=tba::Team_key;

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

template<typename T>
string type_string(T const& x){
	return typeid(x).name();
}

template<typename T>
class Interval{
	T min,max;//both inclusive
};

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
	(void)o;
	(void)a;
	nyi
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
	using Day=int;
	struct Day_info{
		using Time=double;//TODO: Change this.
		Time start,end;
		int matches;
	};
	std::map<Day,Day_info> days;
};

//1=ok, 0=error
bool demo2(TBA_fetcher &f,tba::Event_key event){
	//tba::Event_key event("2016orwil");
	//PRINT(event);

	auto event_data=tba::event(f,event);
	//print_r(event_data);

	//PRINT(event_data.start_date);
	//PRINT(event_data.end_date);

	auto timezone=get_timezone(event_data);
	//PRINT(timezone);

	auto m=event_matches(f,event);
	//print_r(m);
	auto times=sorted(nonempty(mapf([](auto x){ return x.time; },m)));
	if(times.empty()){
		return 1;
	}

	//PRINT(min(times));
	//PRINT(max(times));

	auto from=[=](auto x){
		return std::chrono::system_clock::from_time_t(x+3600*timezone);
	};

	/*for(auto x:take(20,times)){
		PRINT(from(x));
	}*/
	assert(event_data.start_date);
	assert(event_data.end_date);
	const auto playing=to_set(range_inclusive(*event_data.start_date,*event_data.end_date));
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
	//PRINT(name);
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

std::vector<string> get_location_names(int){
	nyi
}

/*
 * It would be interesting to see how sponsors, etc. change over the years for each team
 * */

using Year=tba::Year;

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

/*
 * Would be interesting to find matches of teams and events
 * to find the hosts, if there is one.
 *
 * would be interesting to parse out the official names of teams
 * */
void find_hosts(TBA_fetcher &f){
	//check_sponsors(f);
	//exit(0);

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

		/*event.city
			state_prov
			country
			address
			postal_code
			location_name*/

		auto teams=event_teams(f,event.key);
		/*assert(!x.empty());
		auto a=x[0];
		print_r(a);*/

		mapf(
			[](auto x){
				/*if(x.key==tba::Team_key("frc1425")){
					PRINT(parse_name(x.name));
				}*/
				return parse_name(x.name);
			},
			teams
		);

		auto matched=reversed(sorted(mapf(
			[&](auto x){ return make_pair(match_degree(get_loc(x),get_loc(event)),x); },
			teams
		)));

		matched=filter([](auto x){ return x.first>2; },matched);

		//print_r(event.key);
		//cout<<event.key<<"\t"<<get_loc(event)<<"\n";
		//PRINT(take(5,matched));
		/*for(auto [n,team]:take(5,matched)){
			cout<<"\t"<<n<<"\t"<<team.key<<"\t"<<get_loc(team)<<"\n";
		}*/

		if(event.location_name){
			auto f2=filter([=](auto x){ return parse_name(x).orgs.count(*event.location_name); },teams);

			if(f2.size()>1){
				cout<<event.key<<"";
				cout<<"\t"<<get_loc(event)<<"\n";
				//for(auto t:mapf(get_loc,f2)){
				for(auto t:f2){
					//cout<<"\t"<<t<<"\n";
					print_r(1,t);
				}
				//assert(f2.size()==1);
			}
		}

		/*a.city
			state_prov
			country
			address
			postal_code
			location_name*/

		//nyi
	}
}

int dates_demo(TBA_fetcher &f){
	find_hosts(f);
	/*Interesting questions to ask about schedules
	 * 1) typical lengths scheduled (overall dist)
	 * 2) for each length, what's a typical distribution of # of matches on each day
	 * 3) categorize events by which days there are matches
	 * 4) for each of those categories: look at the typical beginning and ending times for each day
	 *
	 * */

	for(auto const& event:all_events(f)){
		auto b=demo2(f,event.key);
		if(!b){
			cout<<"fail in:"<<event.key<<"\n\n";
		}
	}

	return 0;

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

	return 0;
}
