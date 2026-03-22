#include "query.h"
#include "../tba/tba.h"
#include "dates.h"
#include "tba.h"

using namespace std;

set<tba::Team_key> chairmans_winners(TBA_fetcher& f,tba::District_key const& district){
	set<tba::Team_key> r;
	for(auto event:district_events(f,district)){
		auto k=event.key;
		auto aw=event_awards(f,k);
		if(aw.empty()) continue;
		auto f1=filter([](auto const& a){ return a.award_type==tba::Award_type::CHAIRMANS; },aw);
		if(f1.empty()){
			continue;
		}
		if(f1.size()!=1){
			PRINT(district);
			PRINT(k);
			PRINT(aw);
			PRINT(f1);
			nyi

		}

		//There is more than one recipient at the dcmp events.
		for(auto x:f1[0].recipient_list){
			auto team=x.team_key;
			if(!team){
				throw "Error: Chairmans award given with no team as a recipient.";
			}
			r|=*team;
		}
	}
	return r;
}

map<Point,Pr> dcmp_distribution(TBA_fetcher &f){
	vector<tba::District_key> old_districts{
		tba::District_key{"2022pnw"},
		tba::District_key{"2019pnw"},
		tba::District_key{"2018pnw"},
		tba::District_key{"2017pnw"},
		tba::District_key{"2016pnw"},
		tba::District_key{"2015pnw"},
		//tba::District_key{"2014pnw"},
	};
	multiset<Point> v;
	for(auto district:old_districts){
		auto a=district_rankings(f,district);
		if(!a) continue;
		for(auto team_data:*a){
			for(auto event_points:team_data.event_points){
				if(event_points.district_cmp){
					v|=Point(event_points.total);
				}
			}
		}
	}
	map<Point,Pr> r;
	for(auto x:v){
		r[x]=v.count(x)/double(v.size());
	}
	return r;
}

std::optional<multiset<Point>> point_results(TBA_fetcher& fetcher,tba::District_key dk){
	//district-level point totals from events.
	auto d=district_rankings(fetcher,dk);
	if(!d){
		return std::nullopt;
	}
	multiset<Point> r;
	for(auto team_result:*d){
		for(auto event: ::take<2>(team_result.event_points)){
			r|=Point(event.total);
		}
	}
	return r;
}

map<Point,Pr> historical_event_pts(TBA_fetcher &f){
	vector<tba::District_key> old_keys{
		//excluding 2014 since point system for quals was different.
		tba::District_key{"2015pnw"},
		tba::District_key{"2016pnw"},
		tba::District_key{"2017pnw"},
		tba::District_key{"2018pnw"},
		tba::District_key{"2019pnw"},
		tba::District_key{"2022pnw"}
	};

	multiset<Point> old_results;
	for(auto key:old_keys){
		auto p=point_results(f,key);
		if(p){
			old_results|=*p;
		}
	}
	map<Point,unsigned> occurrances;
	for(auto value:old_results){
		occurrances[value]=old_results.count(value); //slow
	}
	map<Point,Pr> pr;
	for(auto [pts,count]:occurrances){
		pr[pts]=double(count)/old_results.size();
	}
	//print_lines(pr);
	//PRINT(sum(seconds(pr)));
	//PRINT(old_results.size())
	return pr;
}

tba::Year year(tba::District_key const& a){
	auto s=a.get().substr(0,4);
	return tba::Year(stoi(s));
}

tba::Year year(tba::Event_key const& a){
	auto s=a.get().substr(0,4);
	return tba::Year(stoi(std::string(s)));
}

tba::Year year(tba::Event const& a){
	return year(a.key);
}

tba::Team_key rand(tba::Team_key const*){
	std::stringstream ss;
	ss<<"frc"<<rand()%1000;
	return tba::Team_key(ss.str());
}

tba::Event_key rand(tba::Event_key const*){
	std::stringstream ss;
	ss<<"2026";
	for(auto _:range_st<5>()){
		(void)_;
		ss<<char('a'+rand()%26);
	}
	return tba::Event_key(ss.str());
}

bool chairmans_expected(tba::Event_type a){
	#define X(NAME,RESULT) if(a==tba::Event_type::NAME) return RESULT;
	X(DISTRICT,1)
	X(DISTRICT_CMP_DIVISION,0)
	X(DISTRICT_CMP,1)
	#undef X

	PRINT(a);
	nyi
}

bool chairmans_expected(TBA_fetcher &f,tba::Event_key const& a){
	auto e=tba::event(f,a);
	return chairmans_expected(e.event_type);
}

bool complete(tba::Match const& a){
	if(a.post_result_time){
		return 1;
	}
	auto s0=a.alliances.red.score.valid();
	auto s1=a.alliances.blue.score.valid();

	//assert(s0==s1);//not true for 2014txri_sf1m3
	if(s0 || s1){
		return 1;
	}

	if(a.videos.size()){
		//lol! that's a funny way to know that a match is done.
		//but this really happens.  For example, 2016onsc_qf1m1
		return 1;
	}

	//could decide to look at the time for it and make them time out if it was too long ago.
	return 0;
}

bool matches_complete(TBA_fetcher &f,tba::Event_key const& event){
	//note that this isn't thinking too hard about whether or not this event ought to have matches.
	auto e=tba::event_matches(f,event);
	if(e.empty()){
		return 0;
	}
	return all(MAP(complete,e));
}

bool won_chairmans(TBA_fetcher &f,tba::Year year,tba::Team_key const& team){
	auto t=tba::team_awards_year(f,team,year);
	auto found=count_if([](auto x){ return x.award_type==tba::Award_type::CHAIRMANS; },t);
	return found!=0;
}

bool event_timed_out(TBA_fetcher &f,tba::Event_key const& event){
	auto e=tba::event(f,event);
	assert(e.end_date);
	auto since_end=current_date()-*e.end_date;
	return since_end>std::chrono::days(3);
}

std::vector<tba::Event> events(TBA_fetcher &f){
	return flatten(mapf([&](auto year){ return tba::events(f,year); },years()));
}

std::vector<tba::Event_key> events_keys(TBA_fetcher &f){
	return mapf([](auto x){ return x.key; },events(f));
}

std::vector<tba::Year> years(){
	return range(tba::Year(1992),tba::Year(2027));
}

std::vector<tba::Team> teams(TBA_fetcher &f,tba::Year year){
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

std::vector<tba::Team> teams(TBA_fetcher &f){
	//just asking for one year because it actually doesn't give different results for different years.
	return teams(f,tba::Year(2026));
}

std::vector<tba::Team_key> teams_keys(TBA_fetcher& f,tba::Event_key const& a){
	return tba::event_teams_keys(f,a);
}

std::vector<tba::Team_key> teams_keys(TBA_fetcher& f,tba::Event const& a){
	return teams_keys(f,a.key);
}

static std::vector<tba::District_key> districts_inner(TBA_fetcher &f){
	auto found=flatten(mapf([&](auto year){ return tba::districts(f,year); },years()));
	return mapf([](auto x){ return x.key; },found);
}

std::vector<tba::District_key> const& districts(TBA_fetcher &f){
	static std::vector<tba::District_key> cache=districts_inner(f);
	return cache;
}

std::vector<tba::Event> events(TBA_fetcher &f,tba::District_key const& district){
	return tba::district_events(f,district);
}

std::vector<tba::Event_key> events_keys(TBA_fetcher &f,tba::District_key const& district){
	static map<tba::District_key,std::vector<tba::Event_key>> cache;
	static std::mutex lock;
	std::lock_guard<std::mutex> locked(lock);
	auto it=cache.find(district);
	if(it!=cache.end()){
		return it->second;
	}
	return cache[district]=tba::district_events_keys(f,district);
}

bool playoff(tba::Competition_level a){
	return a!=tba::Competition_level::qm;
}

std::vector<tba::Match> playoff_matches(TBA_fetcher &f,tba::Event_key const& event){
	//auto m=tba::event_matches(f,event);
	//PRINT(count(mapf([](auto x){ return x.comp_level; },m)));
	return filter(
		[](auto const& x){ return playoff(x.comp_level); },
		tba::event_matches(f,event)
	);
}

std::vector<tba::Match_Simple> playoff_matches_simple(TBA_fetcher &f,tba::Event_key const& event){
	return filter(
		[](auto const& x){ return playoff(x.comp_level); },
		tba::event_matches_simple(f,event)
	);
}

bool playoffs_started(TBA_fetcher &f,tba::Event_key const& event){
	/*PRINT(event);
	auto p1=playoff_matches(f,event);
	PRINT(p1.size());
	nyi*/
	auto p=filter([](auto const& x){ return complete(x); },playoff_matches(f,event));
	return !p.empty();
}

bool awards_done(TBA_fetcher &f,tba::Event_key const& event){
	auto aw=event_awards(f,event);
	if(aw.empty()) return 0;
	auto f1=filter([](auto const& a){ return a.award_type==tba::Award_type::CHAIRMANS; },aw);
	return !f1.empty();
}

std::optional<tba::District_key> district(TBA_fetcher &f,tba::Event_key const& event){
	auto found=filter(
		[&](auto const& x){ return contains(events_keys(f,x),event); },
		districts(f)
	);
	if(found.size()==1){
		return found[0];
	}
	if(found.empty()){
		return std::nullopt;
	}
	PRINT(event);
	PRINT(found);
	assert(0);
}

bool complete(TBA_fetcher &f,tba::Event_key const& a){
	return awards_done(f,a) || event_timed_out(f,a);
}

tba::Event_type event_type(TBA_fetcher &f,tba::Event_key const& event){
	auto x=tba::event(f,event);
	return x.event_type;
}

std::string link(tba::Event_key const& event,std::string const& body){
	return link("https://www.thebluealliance.com/event/"+std::string(event.get()),body);
}

std::string link(tba::Event const& event,std::string const& body){
	return link(event.key,body);
}

tba::Event_type event_type(tba::Event const& a){
	return a.event_type;
}

tba::Event_type event_type(TBA_fetcher &f,tba::Event_points const& a){
	return event_type(f,a.event_key);
}

tba::Year current_season(TBA_fetcher& f){
	return tba::status(f).current_season;
}

using Date=tba::Date;
using Year=tba::Year;
using Team_key=tba::Team_key;

Date dcmp_end_calc(TBA_fetcher &f,tba::District_key const& district){
	auto found=filter(
		[](auto x){ return x.event_type==tba::Event_type::DISTRICT_CMP; },
		events(f,district)
	);
	auto m=mapf([](auto x){ assert(x.end_date); return *x.end_date; },found);
	assert(!m.empty());
	assert(all_equal(m));
	return m[0];
}

Date dcmp_end(TBA_fetcher &f,tba::District_key const& district){
	static std::map<tba::District_key,Date> cache;
	static std::mutex lock;
	std::lock_guard<std::mutex> locked(lock);
	{
		auto f1=cache.find(district);
		if(f1!=cache.end()){
			return f1->second;
		}
	}
	return cache[district]=dcmp_end_calc(f,district);
}

Date dcmp_start(TBA_fetcher &f,tba::District_key const& district){
	auto found=filter(
		[](auto x){ return x.event_type==tba::Event_type::DISTRICT_CMP; },
		events(f,district)
	);
	auto m=mapf([](auto x){ assert(x.start_date); return *x.start_date; },found);
	assert(!m.empty());
	assert(all_equal(m));
	return m[0];
}

std::optional<Date> dcmp_start(TBA_fetcher& f,std::optional<tba::District_key> const& a){
	if(!a){
		return std::nullopt;
	}
	return dcmp_start(f,*a);
}

Date cmp_start_inner(TBA_fetcher &f,tba::Year year){
	auto found=filter(
		[](auto x){ return x.event_type==tba::Event_type::CMP_DIVISION || x.event_type==tba::Event_type::CMP_FINALS; },
		tba::events(f,year)
	);
	auto m=mapf([](auto x){ assert(x.start_date); return *x.start_date; },found);
	assert(!m.empty());
	//PRINT(count(m));
	//assert(all_equal(m));
	return min(m);
}

Date cmp_start(TBA_fetcher &f,tba::Year year){
	static std::map<Year,Date> cache;
	static std::mutex lock;
	std::lock_guard<std::mutex> locked(lock);
	auto found=cache.find(year);
	if(found!=cache.end()){
		return found->second;
	}
	return cache[year]=cmp_start_inner(f,year);
}

std::vector<tba::District_key> districts_keys(TBA_fetcher &f,Year year){
	return mapf([](auto x){ return x.key; },tba::districts(f,year));
}

auto calc_districts(TBA_fetcher &f){
	std::map<std::pair<Team_key,Year>,tba::District_key> r;

	for(auto year:years()){
		for(auto district:districts_keys(f,year)){
			for(auto team:tba::district_teams_keys(f,district)){
				r.insert(make_pair(make_pair(team,year),district));
			}
		}
	}

	return r;
}

std::optional<tba::District_key> district(TBA_fetcher& f,Team_key const& team,Year const& year){
	/*auto f1=filter(
		[&](auto x){
			//return to_set(tba::district_teams_keys(f,x)).count(a);
			return contains(tba::district_teams_keys(f,x),a);
		},
		districts_keys(f,year)
	);
	if(f1.empty()){
		return std::nullopt;
	}
	if(f1.size()!=1){
		PRINT(a);
		PRINT(year);
		print_r(f1);
	}
	assert(f1.size()==1);
	return f1[0];*/

	static auto cache=calc_districts(f);

	auto it=cache.find(make_pair(team,year));

	if(it==cache.end()){
		return std::nullopt;
	}
	return it->second;
}


