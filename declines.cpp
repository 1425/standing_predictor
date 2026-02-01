#include<fstream>
#include<numeric>
#include<filesystem>
#include "../tba/db.h"
#include "../tba/data.h"
#include "../tba/tba.h"
#include "../frc_api/db.h"
#include "../frc_api/query.h"
#include "../frc_api/simdjson.h"
#include "../frc_api/curl.h"
#include "arguments.h"
#include "frc_api.h"
#include "set.h"
#include "map.h"
#include "tba.h"
#include "util.h"
#include "print_r.h"
#include "declines.h"

/*
The program is designed to figure out how many teams are declining invitations to their district championship event.  

This is interesting to know because that means that they leave slots for other teams, who then do get to go.  

It might also be interesting sometime to calculate teams that decline invites to worlds, which would also be useful for the same reason.

How to turn decline data into improved forecasts:
-# of teams invited to the event turns from a constant into a pdf
-that pdf is calculated based on percentages of limit observed in previous events
-cutoff calculation also samples the # of teams pdf

Interesting things to know for championship predictions:
-# of teams from that district that are prequalified
-# of teams that are given each of the awards that auto-qualify
	-which of the teams are eligible for those awards
		TODO: See if you can win those awards at DCMP that if didn't at district event.

TODO: Compare team score totals between TBA and the FRC events API
TODO: Add offline mode
*/

//Start generic code

template<typename T>
T mode(std::vector<T> const& a){
	auto m=to_multiset(a);
	auto lim=max(mapf([=](auto x){ return m.count(x); },m));
	auto f=filter([=](auto x){ return m.count(x)==lim; },to_set(a));
	if(f.size()!=1){
		PRINT(a);
		PRINT(f);
	}
	assert(f.size()==1);
	return *begin(f);
}

std::string operator+(std::string const& a,frc_api::String2 const& b){
	return a+b.get();
}

template<typename Func>
auto mapf(Func f,std::string s){
	return ::mapf(f,to_vec(s));
}

template<typename T,typename T2>
std::vector<T>& operator-=(std::vector<T> &a,T2 const& t){
	a=filter([&](auto x){ return x!=t; },a);
	return a;
}

//Start program-specific code

frc_api::Team_number to_team(tba::Team_key const& a){
	return frc_api::Team_number{stoi(a.str().substr(3,10))};
}

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



#if 0
optional<vector<tba::District_Ranking>> district_rankings(auto& f,tba::District_key){
	//optional<map<Team,pair<vector<int>,optional<int>>>> analyze_district(
	auto a=analyze_district(f,year,district_code);
	if(!a) return std::nullopt;
	auto b=*a;

	return mapf(
		[](auto p)->District_Ranking{
			auto [team,data]=p;

			//TODO: Figure out when teams are getting the rookie points.  
			return District_Ranking{
				"frc"+as_string(team),
				/*X(int,rank)\
				X(int,rookie_bonus)\
				X(double,point_total)\
				X(std::vector<Event_points>,event_points)*/
				
			};
		},
		b
	);
}
#endif

optional<map<Team,pair<vector<int>,optional<int>>>> analyze_district_tba(
	auto &f,
	frc_api::Season season,
	frc_api::District_code district_code
){
	auto d=district_rankings(
		f,
		tba::District_key{::as_string(season)+tolower(district_code.get())}
	);
	if(!d) return std::nullopt;
	return to_map(mapf(
		[](auto x){
			vector<int> district_pts;
			optional<int> dcmp_pts;

			for(auto event:x.event_points){
				if(event.district_cmp){
					if(dcmp_pts){
						//this happens when there is a multi-field DCMP.
						*dcmp_pts+=int(event.total);
					}else{
						dcmp_pts=int(event.total);
					}
				}else{
					district_pts|=int(event.total);
				}
			}
			assert(district_pts.size()<=2);

			return make_pair(
				to_team(x.team_key),
				make_pair(district_pts,dcmp_pts)
			);
		},
		*d
	));
}

class Local_fetcher_frc{
	vector<unique_ptr<frc_api::Cache>> cache;

	public:
	Local_fetcher_frc(){
		for(auto path:find("..","cache.db")){
			try{
				cache.emplace_back(new frc_api::Cache(path.c_str()));
			}catch(std::runtime_error const&){
			}
		}
	}

	std::pair<optional<frc_api::HTTP_Date>,frc_api::Data> fetch(frc_api::URL url)const{
		for(auto & c:cache){
			assert(c);
			auto f=c->fetch(url);
			if(f) return *f;
		}
		throw No_data{url};
	}
};

struct FRC_fetcher_base{
	virtual std::pair<optional<frc_api::HTTP_Date>,frc_api::Data> fetch(frc_api::URL url)const=0;
	virtual ~FRC_fetcher_base(){}
};

template<typename T>
class FRC_fetcher_impl:public FRC_fetcher_base{
	std::unique_ptr<T> data;

	public:
	FRC_fetcher_impl(T *t):data(t){}

	std::pair<optional<frc_api::HTTP_Date>,frc_api::Data> fetch(frc_api::URL url)const{
		return data->fetch(url);
	}
};

class FRC_fetcher{
	std::unique_ptr<FRC_fetcher_base> data;

	public:
	template<typename T>
	FRC_fetcher(T *t):
		data(new FRC_fetcher_impl<T>{t})
	{}

	std::pair<optional<frc_api::HTTP_Date>,frc_api::Data> fetch(frc_api::URL url)const{
		return data->fetch(url);
	}
};

FRC_fetcher get_frc_fetcher(bool local_only){
	if(local_only){
		return FRC_fetcher{new Local_fetcher_frc{}};
	}
	ifstream f("../frc_api/api_key");
	string s;
	getline(f,s);
	auto x=new frc_api::Cached_fetcher{
		frc_api::Fetcher{frc_api::Nonempty_string{s}},
		frc_api::Cache{}
	};
	return FRC_fetcher{x};
}

auto rankings1(auto &f,frc_api::Season season,frc_api::District_code district){
	//multiple pages; look at all of them.
	vector<frc_api::DistrictRankings_item> r;
	frc_api::District_rankings q{season,district,std::nullopt,std::nullopt,std::nullopt};
	q.page=1;
	while(1){
		auto d=run(f,q).districtRanks;
		if(d.empty()){
			return filter([=](auto x){ return x.districtCode==district; },r);
		}
		r|=d;
		q.page=(*q.page)+1;
	}
}

std::map<Point,Pr> dcmp_distribution(auto& f){
	vector<pair<frc_api::Season,frc_api::District_code>> old_districts{
		//2022pnw, to 2015 pnw
	};

	for(auto i:range(2015,2025)){
		old_districts|=make_pair(frc_api::Season{i},frc_api::District_code{"PNW"});
	}

	multiset<Point> v;
	for(auto [season,district]:old_districts){
		auto x=rankings1(f,season,district);
		auto d=mapf([](auto x){ return x.districtCode; },x);
		for(auto elem:x){
			v|=Point(elem.totalPoints);
		}
	}
	map<Point,Pr> r;
	for(auto x:v){
		r[x]=v.count(x)/double(v.size());
	}
	return r;
}

multiset<Point> point_results(auto &f,frc_api::Season season,frc_api::District_code district){
	//auto d=district_rankings(f,season,district);
	auto a=rankings1(f,season,district);
	multiset<Point> r;

	auto add=[&](auto x){
		if(!x) return;
		r|=Point(*x);
	};

	for(auto elem:a){
		add(elem.event1Points);
		add(elem.event2Points);
	}
	return r;
}

std::map<Point,Pr> historical_event_pts(auto& f){
	vector<pair<frc_api::Season,frc_api::District_code>> old_keys;
	for(auto i:range(2015,2023)){
		old_keys|=make_pair(frc_api::Season{i},frc_api::District_code{"PNW"});
	}

	multiset<Point> old_results;
	for(auto [season,district]:old_keys){
		old_results|=point_results(f,season,district);
	}
	std::map<Point,Pr> r;
	for(auto n:old_results){
		r[n]=double(old_results.count(n))/old_results.size();
	}
	return r;
}

void demo_reg(auto &f){
	(void)f;
	#if 0
	frc_api::Season season{2024};
	frc_api::Team_number team{254};
	frc_api::Event_code event{"ORORE"};
	auto a=run(f,frc_api::Registrations_query{season,team});
	//auto a=run(f,frc_api::Registrations_query{season,event});
	//these results seem to come out as empty all the time; so need to get the data some different way.
	PRINT(a);
	#endif

	

	nyi
}

void demo2(auto&){
	ifstream f("../frc_api/api_key");
	string s;
	getline(f,s);
	frc_api::Cached_fetcher f1{
		frc_api::Fetcher{frc_api::Nonempty_string{s}},
		frc_api::Cache{}
	};
	FRC_api_fetcher_impl f2{&f1};
	auto x=chairmans_winners(f2,frc_api::Season{2025},frc_api::District_code{"fit"});
	PRINT(x);

	auto d=dcmp_distribution(f2);
	PRINT(d);

	auto h=historical_event_pts(f2);
	PRINT(h);

	//demo_reg(f2);

	exit(0);
}

void demo(bool frc_api_local,auto& tba_f){
	auto f=get_frc_fetcher(frc_api_local);
	demo2(f);
	//auto f=Local_fetcher_frc{};
	//auto tba_f=get_tba_fetcher("../tba/auth_key","../tba/cache.db");
	
	/*if(!tba_api_local){
		nyi
	}
	auto tba_f=Local_fetcher_tba{};*/
	//auto tba_f=get_tba_fetcher(tba_api_local);

	//Season_summary{Season{}};
	for(
		auto year:
		range_inclusive(frc_api::Season{2015},frc_api::Season{2023})
	){
		PRINT(year);

		auto t=teams(f,year);
		//print_r(t);

		auto r=run(f,frc_api::District_listings{year});
		for(auto x:r.districts){
			try{
				auto a=analyze_district(f,year,x.code);
				auto b=analyze_district_tba(tba_f,year,x.code);
				diff(a,b);
				//assert(a==b);
			}catch(No_data const& a){
				cout<<a<<"\n";
			}
		}
	}
}

int main1(int argc,char **argv){
	bool do_demo=0;
	bool frc_api_local=0;
	auto p=Argument_parser("Calculate how many teams decline DCMP invitations");
	p.add("--demo",{},"See how The Blue Alliance & FIRST's API compare",do_demo);
	p.add("--frc_api_local",{},"Do not fetch data via FRC API; just use cached versions",frc_api_local);
	TBA_fetcher_config tba_config;
	tba_config.add(p);
	p.parse(argc,argv);
	auto tba=tba_config.get();

	if(do_demo){
		demo(frc_api_local,tba);
		return 0;
	}

	//for each year
		//for each district
			//look at what teams made it
			//look at the point totals of the teams that did not make it
			//look at which teams won chairmans
			//look at what teams tied the cutoff
			//not going to look into tiebreakers...?

	multiset<tba::Team_key> teams_declined;
	auto years=range(tba::Year{1992},tba::Year{2024});
	for(auto year:reversed(years)){
		//PRINT(year)
		for(auto d:districts(tba,year)){
			//PRINT(d.key);
			auto result=declines(tba,year,d.key);
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

int main(int argc,char **argv){
	try{
		return main1(argc,argv);
	}catch(frc_api::Decode_error const& a){
		cerr<<a<<"\n";
		return 1;
	}catch(frc_api::HTTP_error const& a){
		cerr<<a<<"\n";
		return 1;
	}catch(string const& s){
		cerr<<s<<"\n";
		return 1;
	}catch(No_data const& a){
		cerr<<a<<"\n";
		return 1;
	}catch(std::invalid_argument const& a){
		cerr<<a<<"\n";
		return 1;
	}
}
