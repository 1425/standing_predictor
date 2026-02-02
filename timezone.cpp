#include "timezone.h"
#include<string>
#include<set>
#include<optional>
#include<cassert>
#include "../tba/data.h"
#include "print_r.h"
#include "tba.h"
#include "../tba/tba.h"
#include "optional.h"
#include "vector_void.h"
#include "dates.h"

using namespace std;

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

