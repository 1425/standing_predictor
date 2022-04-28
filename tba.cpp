#include "tba.h"
#include "../tba/tba.h"
#include "set.h"
#include "util.h"

using namespace std;

//start generic code

template<typename T>
std::vector<T> take(size_t n,std::vector<T> const& v){
	return std::vector<T>{v.begin(),v.begin()+std::min(n,v.size())};
}

//start program-specific stuff

set<tba::Team_key> chairmans_winners(tba::Cached_fetcher& f,tba::District_key const& district){
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

map<Point,Pr> dcmp_distribution(tba::Cached_fetcher &f){
	vector<tba::District_key> old_districts{
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
					v|=int(event_points.total);
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

multiset<Point> point_results(tba::Cached_fetcher& server,tba::District_key dk){
	//district-level point totals from events.
	auto d=district_rankings(server,dk);
	assert(d);
	multiset<Point> r;
	for(auto team_result:*d){
		for(auto event:take(2,team_result.event_points)){
			r|=Point(event.total);
		}
	}
	return r;
}

map<Point,Pr> historical_event_pts(tba::Cached_fetcher &f){
	vector<tba::District_key> old_keys{
		//excluding 2014 since point system for quals was different.
		tba::District_key{"2015pnw"},
		tba::District_key{"2016pnw"},
		tba::District_key{"2017pnw"},
		tba::District_key{"2018pnw"},
		tba::District_key{"2019pnw"}
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
