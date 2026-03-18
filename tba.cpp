#include "tba.h"
#include<fstream>
#include<queue>
#include<future>
#include<any>
#include "../tba/tba.h"
#include "set.h"
#include "util.h"
#include "arguments.h"
#include "vector_void.h"
#include "print_r.h"
#include "dates.h"
#include "cat.h"
#include "declines.h"
#include "rand.h"
#include "history.h"

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
	f.add(
		"--tba_refresh",{},
		"Attempt to fetch pages that are expected to change, even if they are cached.",
		refresh
	);
	f.add("--tba_fuzz",{},"Feed random data",fuzz);
}

bool contains(string s,char c){
	for(auto x:s){
		if(x==c){
			return 1;
		}
	}
	return 0;
}

struct TBA_fetcher_log{
	TBA_fetcher inner;
	std::set<tba::URL> data;

	~TBA_fetcher_log(){
		/*auto s=count(MAP(cat,data));
		PRINT(s);
		auto g=group(cat,data);
		for(auto [k,v]:g){
			cout<<k<<"\n";
			print_lines(v);
		}*/
	}

	std::pair<tba::HTTP_Date,tba::Data> fetch(tba::URL url){
		if(!data.count(url)){
			data|=url;
			//PRINT(url);

			/*string p="https://www.thebluealliance.com/api/v3/team/frc";
			if(prefix(url,p)){
				auto rest=url.substr(p.size(),1000);
				if(!contains(rest,'/')){
					//assert(0);
				}
			}*/
		}

		return inner.fetch(url);
	}
};

using URL=tba::URL;

struct TBA_fetcher_refresh{
	tba::Cached_fetcher inner;

	using Result=std::pair<tba::HTTP_Date,tba::Data>;

	std::map<tba::URL,Result> seen;

	~TBA_fetcher_refresh(){
		std::map<URL,Result> to_refresh;
		for(auto [url,old_data]:seen){
			auto c=cat(url);
			if(!c){
				cout<<"No categorization for "<<url<<"\n";
				continue;
			}
			if(std::holds_alternative<Static>(*c)){
				continue;
			}
			if(std::holds_alternative<Now>(*c)){
				to_refresh[url]=old_data;
				continue;
			}
			std::cerr<<"unexpected refresh category";
			exit(1);
		}

		//now go try to refresh the items.
		cout<<"to refresh("<<to_refresh.size()<<"): "<<take<5>(to_refresh)<<"\n";

		for(auto [url,old_data]:to_refresh){
			//PRINT(url);
			auto f2=inner.fetcher.fetch(url);

			//diff(f2,old_data);
			cout<<url<<" "<<(f2.second==old_data.second)<<"\n";

			try{
				inner.cache.update(url,f2);
			}catch(std::string const& s){
				cout<<"Caught:"<<s<<"\n";
			}
		}

		/*std::queue<std::future<int>> q;
		for(auto const& x:to_refresh){
			std::function<int(void)> f=[&]()->int{
				auto [url,old_data]=x;
				auto f2=inner.fetcher.fetch(url);
				bool same=(f2.second==old_data.second);
				cout<<url<<" "<<same<<"\n";
				inner.cache.update(url,f2);
				return same;
			};
			q.push(async(f));
		}
		std::vector<int> found;
		while(!q.empty()){
			found|=q.front().get();
			q.pop();
		}
		PRINT(count(found));*/
	}

	Result fetch(tba::URL url){
		auto f=seen.find(url);
		if(f!=seen.end()){
			return f->second;
		}

		auto r=inner.fetch(url);
		return seen[url]=r;
	}
};

/*class Fetcher_dummy{
	std::optional<tba::URL> url_found;

	std::pair<tba::HTTP_Date,tba::Data> fetcher(URL url)const{
		assert(!url_found);
		url_found=url;
		throw "Dummy";
	}
};*/

/*template<typename R,typename A,typename B>
struct Pattern2{
	std::string name;
	R *return_type;
	string url1;
	A arg1;
	string url2;
	B arg2;
	string url3;

	static std::optional<Pattern2> parse(URL const& a){
		auto sp=split(a,'/');
		PRINT(sp);
		arg1=decode(sp[1],(A*)0);
		arg2=decode(sp[2],(B*)0);
		nyi
	}

	std::string url()const;
};*/

static const std::string base="https://www.thebluealliance.com/api/v3/";

unsigned decode(std::string s,unsigned const*){
	return stoi(s);
}

auto decode(std::string s,std::string const*){
	return s;
}

template<typename T>
bool parse(std::string url1,T const* t,std::string url2,URL url){
	/*cout<<"\n\n";
	PRINT(url1);
	PRINT(type_string(t));
	PRINT(url2);*/

	assert(prefix(url,base));
	auto s=url.substr(base.size(),url.size());
	//PRINT(s);

	if(!prefix(s,url1)){
		return 0;
	}
	s=s.substr(url1.size(),s.size());

	/*PRINT(s);
	PRINT(url1);
	PRINT(type_string(t));
	PRINT(url2);
	cout<<"url2: \""<<url2<<"\"\n";*/

	if(!suffix(s,url2)){
		return 0;
	}
	s=s.substr(0,s.size()-url2.size());
	cout<<"left:"<<s<<"\n";
	(void)t;
	try{
		auto d=decode(s,t);
		(void)d;
	}catch(...){
		return 0;
	}
	return 1;
}

template<typename A,typename B>
bool parse(std::string url1,A const* a,std::string url2,B const* b,std::string url3,std::string url){
	PRINT(url);
	PRINT(url1);
	PRINT(type_string(a));
	PRINT(url2);
	PRINT(type_string(b));
	PRINT(url3);

	assert(prefix(url,base));
	auto s=url.substr(base.size(),url.size());

	PRINT(s);

	if(!prefix(s,url1)){
		return 0;
	}
	s=s.substr(url1.size(),s.size());

	if(!suffix(s,url3)){
		return 0;
	}
	s=s.substr(0,s.size()-url3.size());

	auto sp=split(s,'/');
	if(sp.size()!=3){
		return 0;
	}

	PRINT(sp);

	decode(sp[0],a);

	auto mid="/"+sp[1]+"/";
	if(mid!=url2){
		return 0;
	}

	decode(sp[2],b);

	return 1;
}

template<typename T>
auto postprocess(std::any a){
	nyi//return a;
}

struct Foo{};

template<typename T>
T postprocess(Foo);

namespace tba{
	template<typename T>
	T postprocess(Foo);
}

/*namespace bar{
	struct Bar{};

	int postprocess(Bar);

	template<typename T>
	int postprocess(Bar);
}*/

/*template<typename T>
std::string to_json(T const& t){
	PRINT(type_string(t));
	nyi
}*/

template<typename T>
void serialize(simdjson::builder::string_builder&,T const& t){
	PRINT(type_string(t));
	nyi
}

using SB=simdjson::builder::string_builder;

template<typename T>
void serialize(SB&,std::vector<T> const&);

template<typename T,size_t N>
void serialize(SB&,tba::vector_fixed<T,N> const&);

template<typename T>
void serialize(SB &sb,std::optional<T> const& a);

void serialize(SB& sb,int a){
	sb.append(a);
}

void serialize(SB& sb,std::string const& a){
	sb.escape_and_append_with_quotes(a);
}

void serialize(SB& sb,bool a){
	sb.append(a);
}

void serialize(SB& sb,double a){
	sb.append(a);
}

void serialize(simdjson::builder::string_builder& sb,tba::Year const& a){
	sb.append(a.get());
}

void serialize(SB& sb,tba::District_key const& a){
	sb.append(a.get());
}

void serialize(SB& sb,tba::Team_key const& a){
	sb.append(a.str());
}

void serialize(SB& sb,tba::Event_key const& a){
	sb.append(a.get());
}

void serialize(SB& sb,tba::Event_type const& a){
	#define X(A,B) if(a==tba::Event_type::A){ sb.append(B); return; }
	TBA_EVENT_TYPES(X)
	#undef X
	assert(0);
}

void serialize(SB& sb,tba::Webcast_type const& a){
	#define X(A) if(a==tba::Webcast_type::A){ sb.append(""#A); return; }
	TBA_WEBCAST_TYPES(X)
	#undef X
	assert(0);
}

void serialize(SB& sb,tba::Playoff_type a){
	#define X(A,B,C) if(a==tba::Playoff_type::B){ sb.append(A); return; }
	TBA_PLAYOFF_TYPES(X)
	#undef X
	assert(0);
}

void serialize(SB& sb,tba::Award_type a){
	#define X(A,B) if(a==tba::Award_type::A){ sb.append(B); return; }
	TBA_AWARD_TYPES(X)
	#undef X
	assert(0);
}

void serialize(SB& sb,std::chrono::year_month_day const& a){
	std::stringstream ss;
	ss<<a;
	sb.append(ss.str());
}

#define STRUCT_TO_JSON_INNER(A,B) {\
	if(first){\
		first=0;\
	}else{\
		sb.append_comma();\
	}\
	sb.append(""#B);\
	sb.append_colon();\
	serialize(sb,a.B);\
}\

#define STRUCT_TO_JSON(NAME,ITEMS)\
	void serialize(SB& sb,NAME const& a){\
		sb.start_object();\
		bool first=1;\
		ITEMS(STRUCT_TO_JSON_INNER)\
		sb.end_object();\
	}\

STRUCT_TO_JSON(tba::API_Status_App_Version,TBA_API_STATUS_APP_VERSION)
STRUCT_TO_JSON(tba::Year_info,TBA_YEAR_INFO)
STRUCT_TO_JSON(tba::District_Ranking,TBA_DISTRICT_RANKING)
STRUCT_TO_JSON(tba::Event_points,TBA_EVENT_POINTS)
STRUCT_TO_JSON(tba::Event,TBA_EVENT)
STRUCT_TO_JSON(tba::District_List,TBA_DISTRICT_LIST)
STRUCT_TO_JSON(tba::Webcast,TBA_WEBCAST)
STRUCT_TO_JSON(tba::Team,TBA_TEAM)
STRUCT_TO_JSON(tba::Award,TBA_AWARD)
STRUCT_TO_JSON(tba::Award_Recipient,TBA_RECIPIENT)
STRUCT_TO_JSON(tba::Dcmp_history,TBA_DCMP_HISTORY)

/*void serialize(SB& sb,tba::API_Status_App_Version const& a){
	sb.start_object();
	#define X(A,B) sb.append(""#B); sb.append_colon(); serialize(sb,a.B); sb.append_comma();
	TBA_API_STATUS_APP_VERSION(X)
	#undef X
	sb.end_object();
}*/

template<typename T>
void serialize(SB& sb,std::vector<T> const& a){
	sb.start_array();
	bool first=1;
	for(auto const& x:a){
		if(first){
			first=0;
		}else{
			sb.append_comma();
		}
		serialize(sb,x);
	}
	sb.end_array();
}

template<typename T,size_t N>
void serialize(SB& sb,tba::vector_fixed<T,N> const& a){
	sb.start_array();
	bool first=1;
	for(auto const& x:a){
		if(first){
			first=0;
		}else{
			sb.append_comma();
		}
		serialize(sb,x);
	}
	sb.end_array();
}

template<typename T>
void serialize(SB &sb,std::optional<T> const& a){
	if(a){
		serialize(sb,*a);
	}else{
		sb.append_null();
	}
}

void serialize(simdjson::builder::string_builder& sb,tba::API_Status const& a){
	sb.start_object();
	//#define X(A,B) sb.append_key_value(""#B,a.B); sb.append_comma();
	bool first=1;
	#define X(A,B){\
		if(first){\
			first=0;\
		}else{\
			sb.append_comma();\
		}\
		sb.append(""#B); \
		sb.append_colon(); \
		serialize(sb,a.B); \
	}
	TBA_API_STATUS(X)
	#undef X
	sb.end_object();
}

/*std::string to_json(tba::API_Status const& a){
	simdjson::builder::string_builder sb;
	serialize(sb,a);
	return sb;
}*/

template<typename T>
std::string to_json(T const& a){
	simdjson::builder::string_builder sb;
	print_r(a);
	serialize(sb,a);
	return sb;
}

template<typename ...Ts>
std::variant<Ts...> rand(std::variant<Ts...> const*)nyi

template<typename T>
T rand2(T const* x){
	if constexpr(std::is_same<T,std::string>()){
		nyi
	/*}else if constexpr(std::is_same<T,tba::API_Status>()){
		auto r=rand(x);
		PRINT(r);
		return r;*/
	}else{
		PRINT(type_string(x));
		auto r=rand(x);
		PRINT(r);
		return r;
	}
}

template<typename T>
std::optional<T> rand2(std::optional<T> const* x){
	//return ::rand(x);
	(void)x;
	return rand((T*)0);
}

//class TBA_fetcher_fuzz:public TBA_fetcher_impl<TBA_fetcher_fuzz>{
class TBA_fetcher_fuzz{
	public:

	TBA_fetcher_fuzz(){
		srand(1001);
	}

	TBA_fetcher_fuzz(TBA_fetcher_fuzz const&)=delete;
	TBA_fetcher_fuzz& operator=(TBA_fetcher_fuzz const&)=delete;

	pair<tba::HTTP_Date,tba::Data> fetch(tba::URL const& url)const{
	//bar::Bar fetch(tba::URL const& url)const{
		PRINT(url);

		/*Fetcher_dummy dummy;
		status(dummy);
		if(dummy.url_found){
			nyi
		}*/

		//using Item=std::variant<std::string,
		//using Pattern=std::vector<Item> patterns;
		#define X0(NAME,RETURN_VALUE,URL) {\
			auto sp=split(url,'/');\
			assert(!sp.empty());\
			auto name=last(sp);\
			if(""#NAME==name){\
				auto s=to_json(rand2((tba::RETURN_VALUE*)0));\
				PRINT(s);\
				return make_pair(rand((tba::HTTP_Date*)0),s);\
			}\
		}
		#define X1(NAME,RETURN_VALUE,URL1,TYPE1,URL2) \
			if(parse(URL1,(tba::TYPE1*)0,URL2,url)){\
				using namespace tba;\
				auto s=to_json(rand2((RETURN_VALUE*)0));\
				PRINT(s);\
				return make_pair(rand((tba::HTTP_Date*)0),s);\
			}
		#define X2(NAME,RETURN_VALUE,URL1,TYPE1,URL2,TYPE2,URL3)\
			if(parse(URL1,(tba::TYPE1*)0,URL2,(tba::TYPE2*)0,URL3,url)){\
				using namespace tba;\
				auto s=to_json(rand2((RETURN_VALUE*)0));\
				PRINT(s);\
				return make_pair(rand((tba::HTTP_Date*)0),s);\
			}

		TBA_QUERIES(X0,X1,X2)

		nyi
	}
};

TBA_fetcher TBA_fetcher_config::get()const{
	if(fuzz){
		return new TBA_fetcher_fuzz();
	}

	TBA_fetcher r=[&]()->TBA_fetcher{
		if(local_only){
			return new Local_fetcher_tba{};
		}
		auto base=[&](){ return get_tba_fetcher(auth_key_path,cache_path); };
		if(refresh){
			return new TBA_fetcher_refresh(base(),{});
		}
		return new tba::Cached_fetcher(base());
	}();

	if(log){
		r=new TBA_fetcher_log(std::move(r),{});
	}
	return r;
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

std::vector<tba::District_key> districts(TBA_fetcher &f){
	static std::vector<tba::District_key> cache;
	if(!cache.empty()){
		return cache;
	}
	auto found=flatten(mapf([&](auto year){ return tba::districts(f,year); },years()));
	return cache=mapf([](auto x){ return x.key; },found);
}

std::vector<tba::Event> events(TBA_fetcher &f,tba::District_key const& district){
	return tba::district_events(f,district);
}

std::vector<tba::Event_key> events_keys(TBA_fetcher &f,tba::District_key const& district){
	static map<tba::District_key,std::vector<tba::Event_key>> cache;
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

