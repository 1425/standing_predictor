#include<fstream>
#include<numeric>
#include "../tba/db.h"
#include "../tba/data.h"
#include "../tba/tba.h"
#include "../frc_api/db.h"
#include "../frc_api/query.h"
#include "../frc_api/rapidjson.h"
#include "../frc_api/curl.h"
#include "util.h"

//Start generic code

#define FILTER(A,B) filter([&](auto x){ return (A)(x); },(B))

template<typename T>
bool operator==(std::set<T> const& a,std::vector<T> const& b){
	return a==to_set(b);
}

template<typename T>
bool all_equal(std::vector<T> const& a){
	for(auto elem:a){
		if(elem!=a[0]){
			return 0;
		}
	}
	return 1;
}

template<typename T>
auto to_set(std::multiset<T> const& v){
	return std::set<T>{begin(v),end(v)};
}

template<typename T>
std::multiset<T>& operator|=(std::multiset<T> &a,std::vector<T> const& b){
	a.insert(begin(b),end(b));
	return a;
}

template<typename T>
std::vector<T> range_inclusive(T start,T lim){
	std::vector<T> r;
	for(auto i=start;i<=lim;++i){
		r|=i;
		if(i==lim){
			return r;
		}
	}
	return r;
}

//Start program-specific code

using namespace std;

optional<pair<int,vector<tba::Team_key>>> declines(
	auto &f,
	tba::Year const& year,
	tba::District_key const& district
){
	auto ranks=district_rankings(f,district);
	assert(ranks);
	//pre-dcmp points, and if got any points at dcmp, and previously won a chairman's award
	auto data=::mapf(
		[&](auto x){
			int pre_dcmp=x.rookie_bonus;
			bool dcmp=0;
			for(auto y:x.event_points){
				if(y.district_cmp){
					//looking for qual pts to avoid counting EI teams w/o robot.
					if(y.qual_points){
						dcmp=1;
					}
				}else{
					pre_dcmp+=y.total;
					//PRINT(y);
					//nyi
					//if(y.award_points>=10) chairmans=1;
				}
			}
			auto chairmans_local=filter(
				[](auto y)->bool{
					return y.award_type==tba::Award_type::CHAIRMANS;
				},
				team_awards_year(f,x.team_key,year)
			);
			bool chairmans=!chairmans_local.empty();
			return make_tuple(pre_dcmp,dcmp,chairmans,x.team_key);
		},
		*ranks
	);

	//ignore all teams whole won the chairman's award
	auto by_points=filter([](auto x){ return !get<2>(x); },data);

	//print_lines(sorted(by_points));

	//find point threshold to make it

	auto at_cmp=FILTER(get<1>,by_points);

	if(at_cmp.empty()) return std::nullopt;
	auto cutoff=min(MAP(get<0>,at_cmp));
	//PRINT(cutoff)

	//# of teams > that threshold with no dcmp points
	auto not_at_cmp=filter([](auto x){ return !get<1>(x); },data);
	auto declined=filter([=](auto x){ return get<0>(x)>cutoff || get<2>(x); },not_at_cmp);

	//PRINT(declined);
	//print_lines(declined);

	auto all_at_cmp=FILTER(get<1>,data);
	auto event_size=all_at_cmp.size();

	return make_pair(event_size,::mapf([](auto x){ return get<3>(x); },declined));
}

auto get_tba_fetcher(std::string const& auth_key_path,std::string const& cache_path){
	ifstream ifs(auth_key_path);
	string tba_key;
	getline(ifs,tba_key);
	return tba::Cached_fetcher{tba::Fetcher{tba::Nonempty_string{tba_key}},tba::Cache{cache_path.c_str()}};
}

auto get_frc_fetcher(){
	ifstream f("../frc_api/api_key");
	string s;
	getline(f,s);
	return frc_api::Cached_fetcher{frc_api::Fetcher{frc_api::Nonempty_string{s}},frc_api::Cache{}};
}

namespace frc_api{
template<typename Fetcher,typename T>
auto run(Fetcher &fetcher,frc_api::URL url,const T*){
	//PRINT(url);
	auto g=fetcher.fetch(url);
	rapidjson::Document a;
	a.Parse(g.second.c_str());
	try{
		return decode(a,(T*)nullptr);
	}catch(...){
		std::cout<<url<<"\n";
		throw;
        }
}

#define X(A,B) \
	template<typename Fetcher>\
	B run(Fetcher &fetcher,A a){\
		return run(fetcher,url(a),(B*)nullptr);\
	}
FRC_API_QUERY_TYPES(X)
#undef X
}

int points(frc_api::Cached_fetcher &f,frc_api::Season season,int awardId){
	//obviously very slow to run this every time.  Could easily make this get cached.
	auto aw=run(f,frc_api::Award_listings{season});
	assert(aw);

	auto f0=filter([=](auto x){ return x.awardId==awardId; },aw->awards);
	auto names=mapf([](auto x){ return x.description; },f0);
	if(!all_equal(names)){
		PRINT(names);
		nyi
	}
	assert(names.size());
	map<string,int> m{
		{"District Chairman's Award",10},
		{"District Engineering Inspiration Award",8},
		{"District Event Winner",0},
		{"District Event Finalist",0},
		{"Industrial Safety Award sponsored by Underwriters Laboratories",5},
		{"Industrial Design Award sponsored by General Motors",5},
		{"Highest Rookie Seed",0},
		{"Judges' Award",5},
		{"Rookie All Star Award",5},
		{"Rookie Inspiration Award",5},
		{"Entrepreneurship Award sponsored by Kleiner Perkins Caufield and Byers",5},
		{"Team Spirit Award sponsored by Chrysler",5},
		{"Excellence in Engineering Award sponsored by Delphi",5},
		{"Gracious Professionalism Award sponsored by Johnson & Johnson",5},
		{"Creativity Award sponsored by Xerox",5},
		{"Quality Award sponsored by Motorola",5},
		{"Innovation in Control Award sponsored by Rockwell Automation",5},
		{"Imagery Award in honor of Jack Kamen",5},

		//also appears at DCMP
		{"Regional Chairman's Award",0}, //auto advance, so pts don't matter
		{"Regional Engineering Inspiration Award",0},

		//not a team award
		{"FIRST Dean's List Finalist Award",0},

		{"District Championship Winner",0},//autoadv
		{"District Championship Finalist",0},
		{"Woodie Flowers Finalist Award",0},

		//obviously will end up getting multiplied by 3.
		{"District Championship Rookie All Star Award",5},

		{"Volunteer of the Year",0},
		{"Rookie Inspiration Award sponsored by National Instruments",5},
		{"Team Spirit Award sponsored by FCA Foundation",5},
		{"Quality Award sponsored by Motorola Solutions Foundation",5},

		//not really an award
		{"District Championship Points Qualifying Team",0},
		{"FIRST Championship District Points Qualifying Team",0},

		{"Safety Award sponsored by Underwriters Laboratories",5},
		{"Autonomous Award sponsored by Ford",5},
		{"Entrepreneurship Award",5},
		{"Team Spirit Award",5},
		{"Excellence in Engineering Award",5},
		{"Gracious Professionalism Award",5},
		{"Creativity Award sponsored by Rockwell Automation",5},
		{"Quality Award",5},
		{"Innovation in Control Award",5},
	};
	auto f1=m.find(names[0]);
	if(f1==m.end()){
		PRINT(names[0]);
		nyi
	}
	return f1->second;
}

void analyze_event(auto &f,frc_api::Season const& year,frc_api::Event_code const& event_code){
	//Going to have to put in the work to figure out fields relate to each other as a single event if want to know about dcmp points.
	//assert(event.divisionCode==std::nullopt);
	//divisionCode is code for the event that it is part of

	//TODO: Calculate points earned at this event.

	PRINT(event_code);
	//TODO: Figure out who was at this event
	//what were the rankings
	//awards won?
	//alliance selection results
	//playoff perf points
	auto aw=run(f,frc_api::Event_awards{year,event_code});
	//these all come up empty.
	//if(aw) PRINT(*aw);
	//PRINT(aw);

	for(auto x:aw.Awards){
		//PRINT(x);
		auto p=points(f,year,x.awardId);
		//PRINT(p);
		//nyi
	}

	auto a=run(f,frc_api::Event_rankings{year,event_code});
	assert(a);
	//a.rank will give qual results
	auto m=mapf([](auto x){ return x.rank; },a->Rankings);
	assert(m.empty() || to_set(m)==range(min(m),max(m)+1));
	//if(m.size()) PRINT(max(m));
	//try figuring out all the teams and then ask for awards per team?
	//run(f,frc_api::Event_awards{year,team});
	try{
		auto as=run(f,frc_api::Alliance_selection{year,event_code});
		assert(as);
		auto i=as->Alliances.size();
		if(i!=8 && i!=16 && i!=4 && i!=2){
			//assert(as->Alliances.size()==8);
			PRINT(as->Alliances.size());
			nyi
		}
	}catch(frc_api::HTTP_error const& e){
		//should get a code 500.
		//The cancelled 2020 events do this.
		cout<<"No alliance selection for:"<<year<<event_code<<"\n";
		//nyi
	}

	//need playoff perf
	auto x1=run(
		f,
		frc_api::Match_results{
			year,
			event_code,
			frc_api::M2{frc_api::Tournament_level::Playoff}
		}
	);
	assert(x1);
	PRINT(x1->Matches.size());
	nyi
}

void demo(){
	auto f=get_frc_fetcher();

	//Season_summary{Season{}};
	for(auto year:range_inclusive(frc_api::Season{2015},frc_api::Season{2022})){
		PRINT(year);

		/*auto t=run(
			f,
			frc_api::Team_listings{
				year,
				//frc_api::Which4{}
				std::tuple<
					std::optional<frc_api::Event_code>,
					std::optional<frc_api::District_code>,
					std::optional<frc_api::State>,
					int
				>{std::nullopt,std::nullopt,std::nullopt,1}
			}
		);
		PRINT(t);
		nyi*/

		auto r=run(f,frc_api::District_listings{year});
		for(auto x:r.districts){
			auto n=x.code;
			PRINT(n);
			auto r2=run(f,frc_api::Event_listings{year,frc_api::Event_criteria{std::nullopt,n}});
			for(auto event1:r2.Events){
				analyze_event(f,year,event1.code);
			}
		}
		
	}
}

int main1(){
	//demo();

	//for each year
		//for each district
			//look at what teams made it
			//look at the point totals of the teams that did not make it
			//look at which teams won chairmans
			//look at what teams tied the cutoff
			//not going to look into tiebreakers...?
	auto f=get_tba_fetcher("../tba/auth_key","../tba/cache.db");

	//cout<<declines(f,tba::Year{2019},tba::District_key{"2019ne"});
	//return 0;
	multiset<tba::Team_key> teams_declined;
	for(auto year:reversed(range(tba::Year{1992},tba::Year{2023}))){
		//PRINT(year)
		for(auto d:districts(f,year)){
			//PRINT(d.key);
			auto result=declines(f,year,d.key);
			if(result){
				cout<<d.key<<"\t"<<result->first<<"\t"<<result->second.size();
				for(auto t:result->second){
					cout<<"\t"<<t.str().c_str()+3;
				}
				cout<<"\n";
				teams_declined|=result->second;
			}
		}
	}

	cout<<"How many times different teams have declined:\n";
	map<int,vector<tba::Team_key>> m;
	for(auto team:to_set(teams_declined)){
		int c=teams_declined.count(team);
		m[c]|=team;
	}
	//print_lines(m);
	//cout<<m<<"\n";
	for(auto [n,teams]:m){
		cout<<n<<"\n";
		cout<<"\t"<<teams<<"\n";
	}
	return 0;
}

int main(){
	try{
		return main1();
	}catch(frc_api::Decode_error const& a){
		cerr<<a<<"\n";
		return 1;
	}catch(frc_api::HTTP_error const& a){
		cerr<<a<<"\n";
		return 1;
	}
}
