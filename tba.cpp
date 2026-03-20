#include "tba.h"
#include<fstream>
#include<queue>
#include<future>
#include<any>
#include<random>
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
#include "decode.h"
#include "json.h"

using namespace std;

TBA_fetcher_base::~TBA_fetcher_base(){}

tba::Cached_fetcher get_tba_fetcher(std::string const& auth_key_path,std::string const& cache_path){
	ifstream ifs(auth_key_path);
	string tba_key;
	getline(ifs,tba_key);
	return tba::Cached_fetcher{tba::Fetcher{tba::Nonempty_string{tba_key}},tba::Cache{cache_path.c_str()}};
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
	f.add("--tba_fuzz",{"PATH"},"Feed random data",fuzz);
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
	//cout<<"left:"<<s<<"\n";
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
	assert(prefix(url,base));
	auto s=url.substr(base.size(),url.size());

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

	//PRINT(sp);

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
		//PRINT(type_string(x));
		auto r=rand(x);
		//PRINT(r);
		return r;
	}
}

template<typename T>
std::optional<T> rand2(std::optional<T> const* x){
	//return ::rand(x);
	(void)x;
	return rand((T*)0);
}

template<typename Rand>
std::string rand(Rand&,std::string const*);

template<typename Rand>
int rand(Rand& rng,int const*){
	return rng();
}

namespace tba{
template<typename Rand>
tba::API_Status_App_Version rand(Rand &rng,tba::API_Status_App_Version const*);

template<typename Rand>
tba::Team_key rand(Rand&,Team_key const*){
	nyi
}
}

template<typename Rand,typename T>
T rand(Rand &rand,T const* x){
	(void)rand;
	PRINT(type_string(x));
	nyi
}

template<typename Rand,typename T>
std::optional<T> rand(Rand& rng,std::optional<T> const*){
	if(rng()%2){
		return rand(rng,(T*)0);
	}
	return std::nullopt;
}

template<typename Rand>
bool rand(Rand &rng,bool const*){
	return rng()%2;
}

template<typename Rand>
tba::District_abbreviation rand(Rand &rng,tba::District_abbreviation const*){
	std::stringstream ss;
	for(auto _:range(2+rng()%2)){
		ss<<char('a'+rng()%26);
	}
	return ss.str();
}

template<typename Rand>
tba::District_key rand(Rand &rng,tba::District_key const*){
	std::stringstream ss;
	ss<<rand(rng,(tba::Year*)0);
	ss<<rand(rng,(tba::District_abbreviation*)0);
	return tba::District_key(ss.str());
}

template<typename Rand,typename T>
auto rand(Rand &rng,std::vector<T> const*){
	std::vector<T> r;
	for(auto const& _:range(rng()%10)){
		(void)_;
		r|=rand(rng,(T*)0);
	}
	return r;
}

template<typename Rand>
std::string rand(Rand &rng,std::string const*){
	std::stringstream ss;
	for(auto _:range(rng()%10)){
		(void)_;
		ss<<char('a'+rng()%26);
	}
	return ss.str();
}

template<typename Rand>
tba::Year rand(Rand& rng,tba::Year const*){
	return tba::Year(1992+rng()%100);
}

template<typename Rand>
tba::API_Status rand(Rand &rng,tba::API_Status const*){
	using namespace tba;
	return tba::API_Status{
		#define X(A,B) rand(rng,(A*)0),
		TBA_API_STATUS(X)
		#undef X
	};
}

template<typename Rand,typename T,size_t N>
tba::vector_fixed<T,N> rand(Rand &rng,tba::vector_fixed<T,N> const&);

#define STRUCT_TO_RAND_INNER(A,B) rand(rng,(A*)0),

#define STRUCT_TO_RAND(NAME,ITEMS)\
	namespace tba{\
	template<typename Rand>\
	NAME rand(Rand &rng,NAME const*){\
		return NAME{ITEMS(STRUCT_TO_RAND_INNER)};\
	}\
	}

STRUCT_TO_RAND(tba::API_Status_App_Version,TBA_API_STATUS_APP_VERSION)
STRUCT_TO_RAND(tba::Year_info,TBA_YEAR_INFO)
STRUCT_TO_RAND(tba::District_Ranking,TBA_DISTRICT_RANKING)
STRUCT_TO_RAND(tba::Event_points,TBA_EVENT_POINTS)
STRUCT_TO_RAND(tba::Event,TBA_EVENT)
STRUCT_TO_RAND(tba::District_List,TBA_DISTRICT_LIST)
STRUCT_TO_RAND(tba::Webcast,TBA_WEBCAST)
STRUCT_TO_RAND(tba::Team,TBA_TEAM)
STRUCT_TO_RAND(tba::Award,TBA_AWARD)
STRUCT_TO_RAND(tba::Award_Recipient,TBA_RECIPIENT)
STRUCT_TO_RAND(tba::Dcmp_history,TBA_DCMP_HISTORY)
STRUCT_TO_RAND(tba::Event_District_Points,TBA_EVENT_DISTRICT_POINTS)
STRUCT_TO_RAND(tba::Points,TBA_POINTS)
STRUCT_TO_RAND(tba::Tiebreaker,TBA_TIEBREAKER)

struct Rng{
	ifstream f;

	explicit Rng(std::string path):
		f(path)
	{}

	unsigned operator()(){
		if(!f.good()){
			return 0;
		}
		unsigned r=0;
		f.get((char*)&r,sizeof(r));
		return r;
	}
};

//class TBA_fetcher_fuzz:public TBA_fetcher_impl<TBA_fetcher_fuzz>{
class TBA_fetcher_fuzz{
	//mutable ifstream f;
	//mutable std::mt19937_64 rng;
	mutable Rng rng;

	public:

	explicit TBA_fetcher_fuzz(std::string path):rng(path){
		//srand(1001);
	}

	TBA_fetcher_fuzz(TBA_fetcher_fuzz const&)=delete;
	TBA_fetcher_fuzz& operator=(TBA_fetcher_fuzz const&)=delete;

	pair<tba::HTTP_Date,tba::Data> fetch(tba::URL const& url)const{
		/*if(f.good()){
			char c=0;
			f.get(c);
			srand(c);
		}*/
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
				auto s=to_json(rand(rng,(tba::RETURN_VALUE*)0));\
				return make_pair(rand((tba::HTTP_Date*)0),s);\
			}\
		}
		#define X1(NAME,RETURN_VALUE,URL1,TYPE1,URL2) \
			if(parse(URL1,(tba::TYPE1*)0,URL2,url)){\
				using namespace tba;\
				auto s=to_json(rand(rng,(RETURN_VALUE*)0));\
				return make_pair(rand((tba::HTTP_Date*)0),s);\
			}
		#define X2(NAME,RETURN_VALUE,URL1,TYPE1,URL2,TYPE2,URL3)\
			if(parse(URL1,(tba::TYPE1*)0,URL2,(tba::TYPE2*)0,URL3,url)){\
				using namespace tba;\
				auto s=to_json(rand(rng,(RETURN_VALUE*)0));\
				return make_pair(rand((tba::HTTP_Date*)0),s);\
			}

		TBA_QUERIES(X0,X1,X2)

		nyi
	}
};

TBA_fetcher TBA_fetcher_config::get()const{
	if(fuzz){
		return new TBA_fetcher_fuzz(*fuzz);
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

