#include "dates.h"
#include<chrono>
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
#include "timezone.h"
#include "names.h"
#include "interval.h"


std::optional<std::chrono::days> operator-(std::chrono::year_month_day a,std::optional<std::chrono::year_month_day> const& b){
	if(!b){
		return std::nullopt;
	}
	return a-*b;
}

using namespace std;
using Team_key=tba::Team_key;
using Event_key=tba::Event_key;
using District_key=tba::District_key;
using Year=tba::Year;

void show_duration(std::chrono::duration<long int,std::ratio<1,1000*1000*1000>> a){
	std::chrono::hh_mm_ss hms{a};
	cout<<hms;
}

std::ostream& operator<<(std::ostream& o,Time_ns const& a){
	std::chrono::hh_mm_ss hms{a};
	return o<<hms;
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

std::chrono::year_month_day as_date(Time a){
	return std::chrono::floor<std::chrono::days>(a);
	//auto days=std::chrono::duration_cast<std::chrono::days>(a);
	//return std::chrono::year_month_day(std::chrono::sys_days(days));
	PRINT(type_string(a));
	nyi
}

std::chrono::year_month_day operator+(std::chrono::year_month_day a,int b){
	//interpreting b as a number of days.
	return a+std::chrono::days(b);
}

template<typename T>
auto consolidate(std::vector<T> );

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

//should always return between 1-7
int days(tba::Event const& a){
	assert(a.start_date);
	assert(a.end_date);
	return (*a.end_date-*a.start_date).count()+1;
}

bool cmp(tba::Event_type a){
	return a==tba::Event_type::CMP_DIVISION || a==tba::Event_type::CMP_FINALS;
}


std::chrono::year_month_day current_date(){
	// Get the current time point from the system clock
	const std::chrono::time_point now{std::chrono::system_clock::now()};

	// Floor the time point to the nearest day to get a date without time components
	const std::chrono::year_month_day ymd{std::chrono::floor<std::chrono::days>(now)};

	return ymd;
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

void match_timings(TBA_fetcher &f){
	//First, look at which of the times get listed
	#if 0
	using T=tuple<bool,bool,bool,bool>;
	std::vector<T> v;
	for(auto const& event:events(f)){
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
		for(auto const& event:events(f)){
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
		for(auto const& event:events(f)){
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

auto find_hosts(TBA_fetcher &f){
	//See if we can figure out if there is a specific team that might be hosting the event.
	map<tba::Event_key,std::set<Team_key>> r;
	for(auto event:reversed(events(f))){

		/*auto get_loc=[&](auto x){
			return make_tuple(
				x.city,
				normalize_state(x.state_prov),
				normalize_country(x.country),
				x.address,
				x.postal_code,
				x.location_name
			);
		};
		(void)get_loc;*/

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
