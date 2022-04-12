#include<fstream>
#include<numeric>
#include "../tba/db.h"
#include "../tba/data.h"
#include "../tba/tba.h"
#include "util.h"

using namespace std;

#define PRINT(X) { cout<<""#X<<":"<<(X)<<"\n"; }
#define nyi { cout<<"nyi "<<__LINE__<<"\n"; exit(44); }

#define FILTER(A,B) filter([&](auto x){ return (A)(x); },(B))

template<typename T>
auto to_set(multiset<T> const& v){
	return set<T>{begin(v),end(v)};
}

template<typename A,typename B>
std::ostream& operator<<(std::ostream& o,pair<A,B> const& a){
	return o<<"("<<a.first<<","<<a.second<<")";
}

template<typename T>
std::ostream& operator<<(std::ostream& o,std::optional<T> const& a){
	if(a) return o<<*a;
	return o<<"NULL";
}

template<typename T>
std::multiset<T>& operator|=(std::multiset<T> &a,std::vector<T> const& b){
	a.insert(begin(b),end(b));
	return a;
}

//template<typename A,typename B,typename C,typename D>
//std::ostream& operator<<(std::ostream&,std::tuple<int,bool,bool,tba::Team_key> const&);

template<typename T>
vector<T> range(T start,T lim){
	vector<T> r;
	for(auto i=start;i<lim;i++){
		r|=i;
	}
	return r;
}

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

int main(){
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
}
