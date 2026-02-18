#include "tba.h"
#include<fstream>
#include "../tba/tba.h"
#include "set.h"
#include "util.h"
#include "arguments.h"
#include "vector_void.h"
#include "print_r.h"
#include "dates.h"

using namespace std;

TBA_fetcher_base::~TBA_fetcher_base(){}

tba::Cached_fetcher get_tba_fetcher(std::string const& auth_key_path,std::string const& cache_path){
	ifstream ifs(auth_key_path);
	string tba_key;
	getline(ifs,tba_key);
	return tba::Cached_fetcher{tba::Fetcher{tba::Nonempty_string{tba_key}},tba::Cache{cache_path.c_str()}};
}

set<tba::Team_key> chairmans_winners(TBA_fetcher& f,tba::District_key const& district){
	set<tba::Team_key> r;
	for(auto event:district_events(f,district)){
		auto k=event.key;
		auto aw=event_awards(f,k);
		if(aw.empty()) continue;
		auto f1=filter([](auto a){ return a.award_type==tba::Award_type::CHAIRMANS; },aw);
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
			assert(team);
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

multiset<Point> point_results(TBA_fetcher& fetcher,tba::District_key dk){
	//district-level point totals from events.
	auto d=district_rankings(fetcher,dk);
	assert(d);
	multiset<Point> r;
	for(auto team_result:*d){
		for(auto event:take(2,team_result.event_points)){
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
		old_results|=point_results(f,key);
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
class Local_fetcher_tba{
	vector<unique_ptr<tba::Cache>> cache;

	public:
	Local_fetcher_tba(){
		for(auto path:find("..","cache.db")){
			try{
				cache.emplace_back(new tba::Cache(path.c_str()));
			}catch(std::runtime_error const&){
			}
		}
	}

	std::pair<tba::HTTP_Date,tba::Data> fetch(tba::URL const& url)const{
		for(auto &c:cache){
			assert(c);
			auto f=c->fetch(url);
			if(f) return *f;
		}
		throw No_data{url};
	}
};

std::pair<tba::HTTP_Date,tba::Data> TBA_fetcher::fetch(tba::URL const& url)const{
	return data->fetch(url);
}

TBA_fetcher_config::TBA_fetcher_config():
	auth_key_path("../tba/auth_key"),
	cache_path("../tba/cache.db"),
	local_only(0),
	log(0)
{}

void TBA_fetcher_config::add(Argument_parser &f){
	f.add(
		"--tba_auth_key",{"PATH"},
		"Path to auth_key for The Blue Alliance",
		auth_key_path
	);
	f.add(
		"--tba_cache",{"PATH"},
		"Path to cache for data from The Blue Alliance",
		cache_path
	);
	f.add(
		"--tba_local",{},
		"Do not attempt to talk to The Blue Alliance; use only what is in cache.",
		local_only
	);
	f.add(
		"--tba_log",{},
		"Print which tba pages are used, even if cached",
		log
	);
}

struct TBA_fetcher_log{
	TBA_fetcher inner;
	std::vector<tba::URL> data;

	std::pair<tba::HTTP_Date,tba::Data> fetch(tba::URL url){
		//data|=url;
		PRINT(url);
		return inner.fetch(url);
	}
};

TBA_fetcher TBA_fetcher_config::get()const{
	TBA_fetcher r=[&]()->TBA_fetcher{
		if(local_only){
			return new Local_fetcher_tba{};
		}
		return new tba::Cached_fetcher{get_tba_fetcher(auth_key_path,cache_path)};
	}();
	if(log){
		return new TBA_fetcher_log(std::move(r),{});
	}
	return r;
}

TBA_fetcher get_tba_fetcher(
	bool local_only,
	string auth_key_path="../tba/auth_key",
	string cache_path="../tba/cache.db"
){
	if(local_only){
		return new Local_fetcher_tba{};
	}
	return new tba::Cached_fetcher{get_tba_fetcher(auth_key_path,cache_path)};
}

std::ostream& operator<<(std::ostream& o,No_data const& a){
	return o<<"No_data("<<a.url<<")";
}

tba::Year year(tba::District_key const& a){
	auto s=a.get().substr(0,4);
	return tba::Year(stoi(s));
}

tba::Year year(tba::Event_key const& a){
	auto s=a.get().substr(0,4);
	return tba::Year(stoi(s));
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
	for(auto _:range(5)){
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
	assert(s0==s1);
	if(s0){
		return 1;
	}
	//if you uncomment the next line you get linker problems (!)
	//print_r(a);
	nyi
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

std::vector<tba::Team_key> teams_keys(TBA_fetcher& f,tba::Event_key a){
	return tba::event_teams_keys(f,a);
}
