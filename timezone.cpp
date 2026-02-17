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
#include "address.h"

using namespace std;

template<typename T>
bool operator==(set<optional<T>> a,set<T> b){
	for(auto elem:a){
		if(!elem){
			return 0;
		}
		if(!b.count(*elem)){
			return 0;
		}
	}
	for(auto x:b){
		if(!a.count(x)){
			return 0;
		}
	}
	return 1;
}

bool has_matches(TBA_fetcher &f,tba::Event const& e){
	//asking for the keys because they are faster to parse.
	return tba::event_matches_keys(f,e.key).size()!=0;
	//return tba::event_matches(f,e.key).size()!=0;
}

bool has_times(tba::Match const& a){
	return a.time || a.actual_time || a.predicted_time || a.post_result_time;
}

bool has_matches_with_times(TBA_fetcher &f,tba::Event const& e){
	auto m=tba::event_matches(f,e.key);
	return any(MAP(has_times,m));
}

std::optional<std::chrono::hours> offset(std::chrono::time_zone const* a){
	if(!a){
		return std::nullopt;
	}
	return offset(*a);
}

std::chrono::hours get_timezone(State_prov const& a,std::optional<City> const& city){
	//This is very approximate.
	//also ignoring timezones.

	std::set<string> pacific{
		"CA","NV","OR","WA",
		"AZ",//sometimes
		
		//Canada
		"BC"
	};
	std::set<string> mountain{
		"AZ","CO","ID","KS","MT","NE",
		"NV","NM","ND","SD","UT","WY",
		"TX",//small section
		//some of Oregon

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
		"TN","VT","VA","WV","KY",

		//Not a state
		"DC",

		//Canada
		"ON","QC"
	};
	std::set<string> alaska{"AK"};//most of it, anyway
	std::set<string> hawaii{"HI"};

	vector<std::chrono::hours> r;
	#define X(A,B) if(A.count(a.s)) r|=std::chrono::hours(B);
	X(hawaii,-10)
	X(alaska,-9)
	X(pacific,-8)
	X(mountain,-7)
	X(central,-6)
	X(eastern,-5)
	#undef X

	if(r.empty()){
		PRINT(a);
		assert(0);
	}

	if(r.size()==1){
		return r[0];
	}

	if(a=="NV" && city=="Las Vegas"){
		return std::chrono::hours(-8);
	}
	if(a=="TN" && city=="Knoxville"){
		return std::chrono::hours(-5);
	}
	#define X(A,B,C) if(a==""#B && city==A) return std::chrono::hours(C);
	X("Lawrence",KS,-6)
	X("Grand Forks",ND,-6)
	X("Oak Ridge",TN,-5)
	X("Salina",KS,-6)
	X("Olathe",KS,-6)
	X("Collierville",TN,-6)
	X("West Fargo",ND,-6)
	X("Shawnee",KS,-6)
	X("Fargo",ND,-6)
	X("Sevierville",TN,-5)
	X("Corbin",KY,-5)
	X("Louisville",KY,-5)
	X("Phoenix",AZ,-7)
	X("Chandler",AZ,-7)
	X("Azusa",AZ,-7)
	X("Prescott Valley",AZ,-7)
	X("Flagstaff",AZ,-7)
	X("Tempe",AZ,-7)
	X("Glendale",AZ,-7)
	X("Scottsdale",AZ,-7)
	X("El Paso",TX,-7)
	X("Houston",TX,-6)
	X("Katy",TX,-6)
	X("Dallas",TX,-6)
	X("San Antonio",TX,-6)
	X("Austin",TX,-6)
	X("Irving",TX,-6)
	X("Lubbock",TX,-6)
	X("Texas",TX,-6) //going to assume this means Texas City.
	X("Conroe",TX,-6)
	X("Fort Worth",TX,-6)
	X("Oak Ridge",TX,-6)
	X("The Woodlands",TX,-6)
	X("Plano",TX,-6)
	X("Waco",TX,-6)
	X("Allen",TX,-6)
	X("Pasadena",TX,-6)
	X("Amarillo",TX,-6)
	X("Channelview",TX,-6)
	X("Del Rio",TX,-6)
	X("Greenville",TX,-6)
	X("Rockwall",TX,-6)
	X("Dripping Springs",TX,-6)
	X("New Braunfels",TX,-6)
	X("Nome",TX,-6)
	X("Belton",TX,-6)
	X("League City",TX,-6)
	X("Frisco",TX,-6)
	X("Friendswood",TX,-6)
	X("Corpus Christi",TX,-6)
	X("Manor",TX,-6)
	X("Tomball",TX,-6)
	X("Victoria",TX,-6)
	X("Abilene",TX,-6)
	X("Farmersville",TX,-6)
	X("McAllen",TX,-6)
	#undef X

	PRINT(a)
	PRINT(r)
	PRINT(city);
	nyi
}

//std::chrono::hours get_timezone(Country a,std::optional<State_prov> state){
std::chrono::hours get_timezone(Address address){
	auto a=address.country;
	auto state=address.state;

	/* This only needs to be very roughly correct so that events that
	 * happen between like 8am and 8pm end up on the correct date. */

	if(a=="USA" || a=="Canada"){
		if(state){
			return get_timezone(*state,address.city);
		}
		//Pacific
		return std::chrono::hours(-8);
	}
	if(a=="Australia"){
		if(
			state=="New South Wales" || state=="NSW" || 
			state=="VIC" || state=="Victoria"
		){
			return std::chrono::hours(11); //Sydney summer
		}else if(state=="WA"){
			return std::chrono::hours(8);
		}else{
			cout<<a<<"\t\""<<state<<"\"\n";
			nyi
		}
	}
	if(a=="China" || a=="Taiwan"){
		return std::chrono::hours(8);
	}
	if(a=="Mexico"){
		if(
			state=="DIF" || //Districo Federal
			state=="MEX" ||
			state=="COA" //Coahuila
			||state=="NLE" //Nuevo Leon
			||state=="PUE" //Puebla
			||state=="GUA" //Guanajuato
			||state=="NL" //Nuevo Leon
		){
			//Mexico City
			return std::chrono::hours(-6);
		}
		if(
			state=="BC" //Baja California
		){
			return std::chrono::hours(-8);
		}
		if(state=="SON"){ //Sonora
			return std::chrono::hours(-7);
		}

		PRINT(a); PRINT(state);
		nyi
	}
	if(a=="Brazil"){
		if(
			state=="RS" //Rio Grande do Sul
			||state=="Rio de Janeiro"
			||state=="DF"
			||state=="SC" //Santa Catarina
			||state=="SP" //São Paulo
		){
			//Brasilia standard time
			return std::chrono::hours(-3);
		}
		if(state=="Mato Grosso"){
			return std::chrono::hours(-4);
		}
		PRINT(a);
		PRINT(state);
		nyi
	}
	if(a=="Israel"){
		return std::chrono::hours(2);
	}
	if(a=="Turkey"){
		return std::chrono::hours(3);
	}
	assert(0);
	PRINT(a);
	return std::chrono::hours(0);
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
		//return std::chrono::system_clock::from_time_t(x+3600*timezone);
		return std::chrono::system_clock::from_time_t(x)+timezone;
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
					if(!n || !state_codes().count(n->s)){
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

std::chrono::time_zone const* locate_zone(std::optional<std::string> a){
	if(!a){
		return NULL;
	}
	return std::chrono::locate_zone(*a);
}

std::chrono::hours get_timezone(tba::Event const& event){
	/*auto country=get_country(event);
	auto state=[=]()->std::optional<State_prov>{
		if(event.state_prov){
			return normalize_state(*event.state_prov);
		}
		return std::nullopt;
	}();
	return get_timezone(country,state);*/
	auto a=address(event);
	assert(a);
	//return get_timezone(a->country,a->state);
	return get_timezone(*a);
}

int timezone_demo(TBA_fetcher &f){
	cout<<"timezone demo\n";


	return check_address(f);

	/*auto g=mapf([](auto x){ return x.timezone; },all_events(f));
	print_r(count(g));

	auto f1=filter([](auto x){ return !x.timezone; },all_events(f));
	PRINT(keys(f1));*/

	//There are some really funky events like "202121reg" that don't occur in any meaningful sense.
	//Which for example says that it's in the "America/Anchorage" timezone but ...
	auto events=filter(
		[&](auto x){ return has_matches(f,x); },
		//[&](auto x){ return has_matches_with_times(f,x); },
		all_events(f)
	);

	auto m=mapf([](auto x){ return make_tuple(x.timezone,get_timezone(x),x.key); },all_events(f));
	auto m2=sorted(m);
	//print_lines(m2);

	map<optional<string>,map<std::chrono::hours,vector<tba::Event_key>>> m3;
	for(auto x:m2){
		m3[get<0>(x)][get<1>(x)]|=get<2>(x);
	}

	for(auto [k,v]:m3){
		if(!k) continue;//for now, ignore events that don't have a listed timezone.

		if(set{offset(locate_zone(k))}==keys(v)){
			//cout<<"Looks good\n";
			continue;
		}
		cout<<k<<"\t";
		cout<<locate_zone(k)<<"\n";
		for(auto [k2,v2]:v){
			cout<<"\t"<<k2<<"\t"<<v2.size();
			//cout<<take(5,v2)<<"\n";
			//print_r(2,mapf([&](auto x){ return tba::event(f,x); },take(5,v2)));
			cout<<"\n";
			for(auto t:take(5,v2)){
				cout<<"\t\t"<<t<<"\t"<<address(f,t)<<"\n";
			}
		}
	}

	/*for(auto event:all_events(f)){
		cout<<event.key<<"\t"<<event.timezone<<"\n";
	}*/
	return 0;
}
