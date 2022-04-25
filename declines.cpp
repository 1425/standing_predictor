#include<fstream>
#include<numeric>
#include<filesystem>
#include "../tba/db.h"
#include "../tba/data.h"
#include "../tba/tba.h"
#include "../frc_api/db.h"
#include "../frc_api/query.h"
#include "../frc_api/rapidjson.h"
#include "../frc_api/curl.h"
#include "util.h"

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

#define FILTER(A,B) filter([&](auto x){ return (A)(x); },(B))

template<typename T>
std::set<T>& operator|=(std::set<T>& a,std::set<T> const& b){
	a.insert(b.begin(),b.end());
	return a;
}

template<typename T>
std::set<T> operator|(std::set<T> a,std::set<T> const& b){
	a|=b;
	return a;
}
template<typename A,typename B>
std::vector<std::pair<A,B>> zip(std::vector<A> const& a,std::vector<B> const& b){
	return mapf(
		[&](auto i){ return std::make_pair(a[i],b[i]); },
		range(std::min(a.size(),b.size()))
	);
}

template<typename Func,typename T>
std::vector<T> sort_by(Func f,std::vector<T> a){
	sort(
		a.begin(),
		a.end(),
		[=](auto a,auto b){
			return f(a)<f(b);
		}
	);
	return a;
}

template<typename T>
T last(std::vector<T> const& a){
	assert(a.size());
	return a[a.size()-1];
}

template<typename T>
std::multiset<T> to_multiset(std::vector<T> const& v){
	return std::multiset<T>{v.begin(),v.end()};
}

template<typename Func,typename T>
std::set<T> filter(Func f,std::set<T> const& a){
	std::set<T> r;
	for(auto elem:a){
		if(f(elem)){
			r|=elem;
		}
	}
	return r;
}

template<typename Func,typename T>
auto mapf(Func f,std::multiset<T> const& a){
	std::vector<decltype(f(*a.begin()))> r;
	for(auto elem:a){
		r|=f(elem);
	}
	return r;
}

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

template<typename T>
std::set<T> operator&(std::set<T> const& a,std::set<T> const& b){
	std::set<T> r;
	std::set_intersection(
		a.begin(),a.end(),
		b.begin(),b.end(),
		std::inserter(r,r.begin())
	);
	return r;
}

template<typename T>
std::set<T>& operator|=(std::set<T>& a,std::optional<T> const& b){
	if(b) a|=*b;
	return a;
}

template<typename Func,typename T>
auto group(Func f,std::vector<T> const& v){
	using K=decltype(f(v[0]));
	std::map<K,std::vector<T>> r;
	for(auto x:v){
		r[f(x)]|=x;
	}
	return r;
}

template<typename K,typename V>
std::map<K,V>& operator+=(std::map<K,V>& a,std::map<K,V> const& b){
	for(auto [k,v]:b){
		a[k]+=v;
	}
	return a;
}

template<typename T>
auto enumerate(T const& t){
	return enumerate_from(0,t);
}

template<typename K,typename V>
std::ostream& operator<<(std::ostream& o,std::map<K,V> const& a){
	return o<<to_vec(a);
}

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

template<size_t N,typename T>
std::array<T,N> to_array(std::vector<T> const& a){
	assert(a.size()==N);
	std::array<T,N> r;
	for(auto i:range(N)){
		r[i]=a[i];
	}
	return r;
}

template<typename Func,typename T,size_t N>
auto mapf(Func f,std::array<T,N> const& a){
	std::array<decltype(f(a[0])),N> r;
	for(auto i:range(N)){
		r[i]=f(a[i]);
	}
	return r;
}

void indent(int x){
	for(auto _:range(x)){
		(void)_;
		std::cout<<"\t";
	}
}

void print_r(int,frc_api::Match const&);
void print_r(int,frc_api::TeamListings const&);
void print_r(int,frc_api::Event const&);

template<typename T>
void print_r(int n,T const& t){
	indent(n);
	std::cout<<t<<"\n";
}

template<typename A,typename B>
void print_r(int n,std::pair<A,B> const& a){
	indent(n++);
	std::cout<<"pair\n";
	print_r(n,a.first);
	print_r(n,a.second);
}

template<typename T>
void print_r(int n,std::vector<T> const& v){
	indent(n);
	std::cout<<"vector\n";
	for(auto x:v){
		print_r(n+1,x);
	}
}

template<typename K,typename V>
void print_r(int n,std::map<K,V> const& v){
	indent(n);
	std::cout<<"map\n";
	n++;
	for(auto x:v) print_r(n,x);
}

template<typename T>
void print_r(T const& t){
	return print_r(0,t);
}

std::string operator+(std::string const& a,frc_api::String2 const& b){
	return a+b.get();
}

std::vector<char> to_vec(std::string const& s){
	std::vector<char> r;
	for(auto c:s) r|=c;
	return r;
}

template<typename Func>
auto mapf(Func f,std::string s){
	return ::mapf(f,to_vec(s));
}

std::string tolower(std::string s){
	std::stringstream ss;
	for(auto c:s){
		ss<<char(tolower(c));
	}
	return ss.str();
}

bool prefix(std::string const& whole,std::string const& p){
	return whole.substr(0,p.size())==p;
}

std::vector<std::string> find(std::string const& base,std::string const& name){
	//should do something similar to "find $BASE -name $NAME*"
	std::vector<std::string> r;
	for(auto x:std::filesystem::recursive_directory_iterator(base)){
		if(x.is_regular_file()){
			std::string s=as_string(x).c_str()+1;
			s=s.substr(0,s.size()-1);
			auto sp=split(s,'/');
			//PRINT(sp);
			//cout<<"\""<<sp[sp.size()-1]<<"\"\n";
			if(prefix(last(sp),name)){
				r|=s;
			}
		}
	}
	return r;
}

frc_api::Team_number to_team(tba::Team_key const& a){
	return frc_api::Team_number{stoi(a.str().substr(3,10))};
}

//Start program-specific code

using namespace std;

void print_r(int n,frc_api::Match const& a){
	indent(n);
	cout<<"Match\n";
	n++;
	#define X(A,B) indent(n); cout<<""#B<<"\n"; print_r(n+1,a.B);
	FRC_API_MATCH(X)
	#undef X
}

void print_r(int n,frc_api::TeamListings const& a){
	indent(n++);
	cout<<"TeamListings\n";
	#define X(A,B) indent(n); cout<<""#B<<"\n"; print_r(n+1,a.B);
	FRC_API_TEAMLISTINGS(X)
	#undef X
}

void print_r(int n,frc_api::Event const& a){
	indent(n++);
	cout<<"Event\n";
	#define X(A,B) indent(n); cout<<""#B<<"\n"; print_r(n+1,a.B);
	FRC_API_EVENT(X)
	#undef X
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

int points(auto &f,frc_api::Season season,int awardId){
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

using Color=frc_api::Alliance_color;

Color color(frc_api::Station a){
	switch(a){
		case frc_api::Station::Red1:
		case frc_api::Station::Red2:
		case frc_api::Station::Red3:
			return Color::Red;
		case frc_api::Station::Blue1:
		case frc_api::Station::Blue2:
		case frc_api::Station::Blue3:
			return Color::Blue;
		default:
			assert(0);
	}
}

using Team=frc_api::Team_number;

enum class Level{OF,QF,SF,F};

std::ostream& operator<<(std::ostream& o,Level a){
	switch(a){
		case Level::OF: return o<<"OF";
		case Level::QF: return o<<"QF";
		case Level::SF: return o<<"SF";
		case Level::F: return o<<"F";
		default: assert(0);
	}
}

struct Match_data{
	Level level;
	map<Color,set<Team>> teams;
	Color winner;
};

std::ostream& operator<<(std::ostream& o,Match_data const& a){
	o<<"Match_data(";
	o<<a.level<<" ";
	o<<a.teams<<" ";
	o<<a.winner;
	return o<<")";
}

optional<map<frc_api::Team_number,int>> playoff_pts(
	auto& f,
	frc_api::Season const& year,
	frc_api::Event_code const& event_code
){
	auto x1=run(
		f,
		frc_api::Match_results{
			year,
			event_code,
			frc_api::M2{
				frc_api::Tournament_level::Playoff,
				{},{},{}
			}
		}
	);
	assert(x1);
	if(x1->Matches.size()==0){
		return std::nullopt;
	}
	//PRINT(x1->Matches.size());
	//print_r(x1->Matches);
	//print_lines(enumerate(x1->Matches));

	auto as=run(f,frc_api::Alliance_selection{year,event_code});
	assert(as);

	auto alliances=mapf(
		[](auto x){
			set<Team> teams;
			teams|=x.captain;
			teams|=x.round1;
			teams|=x.round2;
			teams|=x.round3;
			teams|=x.backup;
			return make_pair(x.number,teams);
		},
		as->Alliances
	);

	auto alliance=[&](set<frc_api::Team_number> v)->int{
		//print_r(as->Alliances);
		auto f=filter([=](auto x){ return (v&x.second).size(); },alliances);
		assert(f.size()==1);
		return f[0].first;
	};

	vector<Match_data> v;

	//print_r(x1->Matches);

	Level level_last=Level::OF;
	for(auto match:x1->Matches){
		auto sp=split(match.description);
		assert(sp.size());
		Level level;
		if(sp[0]=="Quarterfinal"){
			level=Level::QF;
		}else if(sp[0]=="Semifinal"){
			level=Level::SF;
		}else if(sp[0]=="Finals" || sp[0]=="Final"){
			level=Level::F;
		}else if(sp[0]=="Tiebreaker" || sp[0]=="Overtime"){
			level=level_last;
		}else if(sp[0]=="Octofinal"){
			level=Level::OF;
		}else{
			PRINT(sp);
			nyi
		}
		level_last=level;

		map<Color,set<Team>> teams;
		for(auto team:match.teams){
			/*if(team.dq){
				print_r(match);
			}
			assert(team.dq==0);*/
			teams[color(team.station)]|=team.teamNumber;
		}
		auto r=match.scoreRedFinal;
		auto b=match.scoreBlueFinal;

		//ignore ties		
		if(r==b) continue;

		v|=Match_data{level,teams,(r>b)?Color::Red:Color::Blue};
	}
	//print_lines(v);

	//calculate the result for each alliance number
	//then calculate how many matches each of their members contributed to that
	//this is harder than it really seems like it should be due to 2015.
	//auto g=group([&](auto x){ return map_values(alliance,x.teams); },v);
	//print_r(g);

	auto finals_matches=filter([](auto x){ return x.level==Level::F; },v);

	auto winning_alliance_num=[=](auto x){
		return alliance(x.teams[x.winner]);
	};

	auto ff=mapf(winning_alliance_num,finals_matches);
	//PRINT(ff);
	//assert(ff.size()==2 || ff.size()==3);

	auto won=winning_alliance_num(last(finals_matches));
	//PRINT(won);
	using Alliance_num=int;
	enum Alliance_finish{OF,QF,SF,F,W};
	map<Alliance_num,Alliance_finish> finish;
	finish[won]=W;
	//for all of the rest of the alliances, their result is defined by the highest level in which they played a match.

	auto to_finish=[](Level l)->Alliance_finish{
		switch(l){
			case Level::OF: return OF;
			case Level::QF: return QF;
			case Level::SF: return SF;
			case Level::F: return F;
			default:
				assert(0);
		}
	};

	auto mark=[&](Level level,set<Team> const& teams){
		auto a=alliance(teams);
		auto f=finish.find(a);
		Alliance_finish fin;
		if(f==finish.end()){
			fin=QF;
		}else{
			fin=f->second;
		}
		finish[a]=max(fin,to_finish(level));
	};

	for(auto x:v){
		mark(x.level,x.teams[Color::Red]);
		mark(x.level,x.teams[Color::Blue]);
	}

	//PRINT(finish);

	//for each match, look at the winning alliance
	//if they advanced, then credit the teams playing for them
	map<Team,int> r;
	for(auto match:v){
		//PRINT(match);
		auto winners=match.teams[match.winner];
		auto an=alliance(winners);
		if(finish[an]>to_finish(match.level)){
			for(auto team:winners){
				r[team]+=5;
			}
		}
	}
	return r;
}

int rank_pts(unsigned teams,unsigned rank){
	//Teams=# of teams at the event
	assert(rank>0);
	assert(rank<=teams);

	static map<int,map<int,int>> cache;

	auto parse=[&](){
		//Could make this cache results.
		ifstream f("data/table/pts_"+as_string(teams)+".txt");
		map<int,int> m;
		while(f.good()){
			string s;
			getline(f,s);
			if(s.empty()){
				continue;
			}
			auto sp=split(s);
			assert(sp.size()==2);
			auto [k,v]=MAP(stoi,to_array<2>(sp));
			m[k]=v;
		}
		return m;
	};

	auto f=cache.find(teams);
	if(f==cache.end()){
		cache[teams]=parse();
		f=cache.find(teams);
	}

	return f->second[rank];
}

optional<map<frc_api::Team_number,int>> rank_pts(
	auto &f,
	frc_api::Season const& year,
	frc_api::Event_code const& event_code
){
	auto a=run(
		f,
		frc_api::Event_rankings{year,event_code,{}}
	);
	assert(a);
	auto m=mapf([](auto x){ return x.rank; },a->Rankings);
	if(m.empty()) return std::nullopt;
	assert(m.empty() || to_set(m)==range(min(m),max(m)+1));
	auto teams=max(m);
	//print_lines(enumerate(a->Rankings));
	return to_map(mapf(
		[=](auto x){
			return make_pair(x.teamNumber,rank_pts(teams,x.rank));
		},
		a->Rankings
	));
}

std::optional<map<frc_api::Team_number,int>> alliance_pts(
	auto &f,
	frc_api::Season const& year,
	frc_api::Event_code const& event_code
){
	try{
		auto as=run(f,frc_api::Alliance_selection{year,event_code});
		assert(as);
		assert(as->count>=0);
		assert(as->Alliances.size()==unsigned(as->count));
		auto i=as->Alliances.size();
		if(i!=8 && i!=16 && i!=4 && i!=2){
			//assert(as->Alliances.size()==8);
			PRINT(as->Alliances.size());
			nyi
		}
		//print_lines(as->Alliances);
		map<frc_api::Team_number,int> r;
		//TODO: Check what the values are for different-size events.
		auto mark=[&](optional<frc_api::Team_number> t,int value){
			if(t){
				r[*t]=value;
			}
		};
		for(auto alliance:as->Alliances){
			mark(alliance.captain,17-alliance.number);
			mark(alliance.round1,17-alliance.number);
			mark(alliance.round2,alliance.number);
		}
		return r;
	}catch(frc_api::HTTP_error const& e){
		//should get a code 500.
		//The cancelled 2020 events do this.
		//cout<<"No alliance selection for:"<<year<<event_code<<"\n";
		//nyi
		return std::nullopt;
	}
}

optional<map<frc_api::Team_number,int>> award_points(
	auto &f,
	frc_api::Season const& year,
	frc_api::Event_code const& event_code
){
	auto aw=run(f,frc_api::Event_awards{year,event_code});

	if(aw.Awards.empty()){
		//obviously, this means that we don't know who won the awards rather 
		//than that there were no awards given.
		return std::nullopt;
	}

	//TODO: Will want to make this note when winning an auto-advance award
	map<frc_api::Team_number,int> r;
	for(auto x:aw.Awards){
		if(x.teamNumber){
			auto n=*x.teamNumber;
			auto p=points(f,year,x.awardId);
			r[n]+=p;
		}
	}
	return r;
}

auto analyze_event(auto &f,frc_api::Season const& year,frc_api::Event_code const& event_code){
	map<frc_api::Team_number,int> r;
	vector<string> missing;

	auto total=[&](string name,auto x){
		if(x){
			r+=*x;
		}else{
			missing|=name;
		}
	};

	total("rank",rank_pts(f,year,event_code));
	total("alliance",alliance_pts(f,year,event_code));
	total("award",award_points(f,year,event_code));
	total("playoff",playoff_pts(f,year,event_code));
	return make_pair(missing,r);
}

map<frc_api::District_code,vector<Team>> teams(auto& f,frc_api::Season year){
	auto get_page=[&](int page){
		auto r=run(
			f,
			frc_api::Team_listings{
				year,
				//frc_api::Which4{}
				std::tuple<
					std::optional<frc_api::Event_code>,
					std::optional<frc_api::District_code>,
					std::optional<frc_api::State>,
					int
				>{std::nullopt,std::nullopt,std::nullopt,page}
			}
		);
		assert(r.pageCurrent==page);
		return r;
	};

	map<frc_api::District_code,vector<Team>> r;
	auto p=get_page(1);
	for(auto i:range(1,1+p.pageTotal)){
		auto x=get_page(i);
		for(auto team:x.teams){
			if(team.districtCode){
				r[*team.districtCode]|=team.teamNumber;
			}
		}
	}
	return r;
}

optional<map<Team,pair<vector<int>,optional<int>>>> analyze_district(
	auto &f,
	frc_api::Season year,
	frc_api::District_code district
){
	PRINT(district);

	auto team_list=teams(f,year)[district];

	auto r2=run(
		f,
		frc_api::Event_listings{
			year,frc_api::Event_criteria{std::nullopt,district}
		}
	);

	auto x=sort_by([](auto x){ return x.dateStart; },r2.Events);

	map<Team,vector<int>> pts_earned;
	map<Team,int> dcmp_pts;
	for(auto event:x){
		auto [error,result]=analyze_event(f,year,event.code);

		//Right now, no data for 2022oncmp1
		//seems just like a dummy and they ran the whole thing as a normal event.
		if(result.empty()) continue;

		switch(event.type){
			case frc_api::Event_Event_type::DistrictEvent:{
				if(error.size()){
					return std::nullopt;
				}
				if(error.size()){
					print_r(event);
					PRINT(error)
				}
				assert(error.empty());
				for(auto [team,pts]:result){
					auto &v=pts_earned[team];
					if(v.size()<2){
						v|=pts;
					}
				}
				break;
			}
			case frc_api::Event_Event_type::DistrictParent:
				//no idea what these are used for.
				break;
			case frc_api::Event_Event_type::DistrictChampionshipDivision:
			case frc_api::Event_Event_type::DistrictChampionship:{
				if(error.size()){
					PRINT(error);
					print_r(event);
					print_r(result);
				}
				if(error.size()) return std::nullopt;
				for(auto [team,pts]:result){
					dcmp_pts[team]+=pts*3;
				}
				break;
			}
			case frc_api::Event_Event_type::DistrictChampionshipWithLevels:
				//This is the equivalent of the Einstein field.
				//So we expect there to be no ranks.
				if(error!=vector<string>{"rank"}){
					return std::nullopt;
				}
				assert(error==vector<string>{"rank"});
				for(auto [team,pts]:result){
					dcmp_pts[team]+=pts*3;
				}
				break;
			default:{
				print_r(event);
				auto y=analyze_event(f,year,event.code);
				print_r(y);
				nyi
			}
		}
	}


	//then total up the points that are earned at those events
	using V=std::pair<vector<int>,optional<int>>;
	map<Team,V> r;
	for(auto team:keys(pts_earned)|keys(dcmp_pts)){
		r[team]=make_pair(
			[&](){
				auto f=pts_earned.find(team);
				if(pts_earned.end()==f){
					PRINT(team);
					PRINT(pts_earned.size());
				}
				if(f==pts_earned.end()){
					return vector<int>{};
				}else{
					return f->second;
				}
			}(),
			[&]()->optional<int>{
				auto f=dcmp_pts.find(team);
				if(f==dcmp_pts.end()){
					return std::nullopt;
				}
				return f->second;
			}()
		);
	}
	return r;
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
		tba::District_key{as_string(season)+tolower(district_code.get())}
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

struct No_data{ string url; };

std::ostream& operator<<(std::ostream& o,No_data const& a){
	return o<<"No_data("<<a.url<<")";
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

//void diff(int,int)

template<typename T>
void diff(int n,T const& a,T const& b){
	if(a!=b){
		indent(n);
		cout<<"a:"<<a<<"\n";
		indent(n);
		cout<<"b:"<<b<<"\n";
	}
}

template<typename T>
void diff(int n,vector<T> const& a,vector<T> const& b){
	/*if(a!=b){
		indent(n++);
		cout<<"vector\n";
	}*/
	for(auto [i,p]:enumerate(zip(a,b))){
		auto [a1,b1]=p;
		if(a1!=b1){
			indent(n);
			cout<<i<<"\n";
			diff(n+1,a1,b1);
		}
	}
	if(a.size()>b.size()){
		for(auto [i,x]:skip(b.size(),enumerate(a))){
			indent(n);
			cout<<"Only in a:"<<i<<" "<<x<<"\n";
		}
	}
	if(a.size()<b.size()){
		for(auto [i,x]:skip(a.size(),enumerate(b))){
			indent(n);
			cout<<"Only in b:"<<i<<" "<<x<<"\n";
		}
	}
}

template<typename A,typename B>
void diff(int n,std::pair<A,B> const& a,std::pair<A,B> const& b){
	/*if(a!=b){
		indent(n++);
		cout<<"pair\n";
	}*/
	if(a.first!=b.first){
		indent(n);
		cout<<"first\n";
		diff(n+1,a.first,b.first);
	}
	if(a.second!=b.second){
		indent(n);
		cout<<"second\n";
		diff(n+1,a.second,b.second);
	}
}

template<typename K,typename V>
void diff(int n,map<K,V> a,map<K,V> b){
	if(a!=b){
		indent(n++);
		cout<<"map\n";
	}
	auto ka=keys(a);
	auto kb=keys(b);
	for(auto elem:ka&kb){
		auto an=a[elem],bn=b[elem];
		if(an!=bn){
			indent(n);
			cout<<elem<<"\n";
			diff(n+1,a[elem],b[elem]);
		}
	}
	for(auto elem:ka-kb){
		indent(n);
		cout<<"Only in a:"<<elem<<"\n";
	}
	for(auto elem:kb-ka){
		indent(n);
		cout<<"Only in b:"<<elem<<"\n";
	}
}

template<typename T>
void diff(int n,optional<T> const& a,optional<T> const& b){
	if(a!=b){
		indent(n++);
		cout<<"optional\n";
	}
	if(a){
		if(b){
			return diff(n,*a,*b);
		}
		nyi
	}
	if(b){
		indent(n);
		cout<<"No a\n";
	}
}

template<typename T>
void diff(T const& a,T const& b){
	return diff(0,a,b);
}

void demo(){
	//auto f=get_frc_fetcher();
	auto f=Local_fetcher_frc{};
	//auto tba_f=get_tba_fetcher("../tba/auth_key","../tba/cache.db");
	auto tba_f=Local_fetcher_tba{};

	//Season_summary{Season{}};
	for(
		auto year:
		range_inclusive(frc_api::Season{2015},frc_api::Season{2022})
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

struct Args{
	bool demo=0;
};

Args parse_args(int argc,char **argv){
	struct Flag{
		string flag;
		string help;
		std::function<void(void)> f;
	};
	Args r;
	vector<Flag> flags{
		Flag{
			"--demo",
			"See how The Blue Alliance & FIRST's API comare",
			[&](){
				r.demo=1;
			}
		}
	};
	flags|=Flag{
		"--help",
		"Show this message",
		[&](){
			cout<<"./declines";
			for(auto flag:flags){
				cout<<" ["<<flag.flag<<"]";
			}
			cout<<"\n\n";
			for(auto flag:flags){
				cout<<flag.flag<<"\n";
				cout<<"\t"<<flag.help<<"\n";
			}
			exit(0);
		}
	};
	for(auto i:range(1,argc)){
		auto f=filter([=](auto x){ return x.flag==argv[i]; },flags);
		if(f.size()!=1){
			cerr<<"Unrecognized argument.\n";
			exit(1);
		}
		f[0].f();
	}
	return r;
}

int main1(int argc,char **argv){
	auto a=parse_args(argc,argv);

	if(a.demo){
		demo();
		return 0;
	}

	//for each year
		//for each district
			//look at what teams made it
			//look at the point totals of the teams that did not make it
			//look at which teams won chairmans
			//look at what teams tied the cutoff
			//not going to look into tiebreakers...?
	auto f=get_tba_fetcher("../tba/auth_key","../tba/cache.db");
	//auto f=Local_fetcher{};

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
	}
}
