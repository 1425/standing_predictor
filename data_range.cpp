#include "data_range.h"
#include<set>
#include "tba.h"
#include "../tba/tba.h"

using namespace std;

template<typename T>
struct Counter{
	std::set<T> data;
};

vector<string> operator|(vector<string> a,const char *s){
	assert(s);
	return a|string(s);
}

template<typename T>
auto group(std::vector<std::optional<T>> a){
	vector_void v0;
	std::vector<T> v1;
	for(auto elem:a){
		if(elem){
			v1|=elem;
		}else{
			v0|=elem;
		}
	}
	return make_pair(v0,v1);
}

template<
	typename A,typename B,typename C,typename D,
	typename E,typename F,typename G,typename H,
	typename I,typename J,typename K
>
auto group(vector<std::variant<A,B,C,D,E,F,G,H,I,J,K>> const& a){
	std::tuple<
		vector<A>,vector<B>,vector<C>,vector<D>,
		vector<E>,vector<F>,vector<G>,vector<H>,
		vector<I>,vector<J>,vector<K>
	> r;
	for(auto const& x:a){
		#define X(INDEX,NAME) \
			if(std::holds_alternative<NAME>(x)){\
				std::get<INDEX>(r)|=std::get<INDEX>(x);\
				continue;\
			}
		X(0,A)
		X(1,B)
		X(2,C)
		X(3,D)
		X(4,E)
		X(5,F)
		X(6,G)
		X(7,H)
		X(8,I)
		X(9,J)
		X(10,K)
		#undef X
		assert(0);
	}
	return r;
}

template<typename...Ts>
tuple<Ts...> group(std::vector<std::variant<Ts...>>){
	nyi
}

template<typename Func,typename ...Ts>
int mapf(Func f,tuple<Ts...>){
	(void)f;
	nyi
}

template<
	typename Func,
	typename A,typename B,typename C,typename D,
	typename E,typename F,typename G,typename H,
	typename I,typename J,typename K
>
auto mapf(Func f,std::tuple<A,B,C,D,E,F,G,H,I,J,K> const& a){
	return std::make_tuple(
		#define X(N) f(std::get<N>(a)),
		X(0) X(1) X(2) X(3)
		X(4) X(5) X(6) X(7)
		X(8) X(9) 
		f(std::get<10>(a))
		#undef X
	);
}

template<
	typename Func,
	typename A,typename B,typename C,typename D,
	typename E,typename F,typename G,typename H,
	typename I,typename J,typename K
>
auto mapv(Func f,std::tuple<A,B,C,D,E,F,G,H,I,J,K> const& a){
	#define X(N) f(std::get<N>(a));
	X(0) X(1) X(2) X(3)
	X(4) X(5) X(6) X(7)
	X(8) X(9) X(10)
	#undef X
}

template<typename...Ts>
void examine(std::vector<std::string> path,std::vector<std::variant<Ts...>> const&);

template<typename T>
void examine(std::vector<string> path,std::vector<optional<T>> const&);

template<typename T,size_t N>
void examine(std::vector<std::string> path,vector<tba::vector_fixed<T,N>> const&);

#define KNOWN(X)\
	X(tba::Match_Score_Breakdown_2025,TBA_MATCH_SCORE_BREAKDOWN_2025)\
	X(tba::Match_Score_Breakdown_2025_Alliance,TBA_MATCH_SCORE_BREAKDOWN_2025_ALLIANCE)\
	X(tba::Match_Score_Breakdown_2024,TBA_MATCH_SCORE_BREAKDOWN_2024)\
	X(tba::Match_Score_Breakdown_2024_Alliance,TBA_MATCH_SCORE_BREAKDOWN_2024_ALLIANCE)\
	X(tba::Match_Score_Breakdown_2023,TBA_MATCH_SCORE_BREAKDOWN_2023)\
	X(tba::Match_Score_Breakdown_2023_Alliance,TBA_MATCH_SCORE_BREAKDOWN_2023_ALLIANCE)\
	X(tba::Match_Score_Breakdown_2022,TBA_MATCH_SCORE_BREAKDOWN_2022)\
	X(tba::Match_Score_Breakdown_2022_Alliance,TBA_MATCH_SCORE_BREAKDOWN_2022_ALLIANCE)\
	X(tba::Match_Score_Breakdown_2020,TBA_MATCH_SCORE_BREAKDOWN_2020)\
	X(tba::Match_Score_Breakdown_2020_Alliance,TBA_MATCH_SCORE_BREAKDOWN_2020_ALLIANCE)\
	X(tba::Match_Score_Breakdown_2017,TBA_MATCH_SCORE_BREAKDOWN_2017)\
	X(tba::Match_Score_Breakdown_2017_Alliance,TBA_MATCH_SCORE_BREAKDOWN_2017_ALLIANCE)\
	X(tba::Match_Score_Breakdown_2016,TBA_MATCH_SCORE_BREAKDOWN_2016)\
	X(tba::Match_Score_Breakdown_2016_Alliance,TBA_MATCH_SCORE_BREAKDOWN_2016_ALLIANCE)\
	X(tba::Match_Score_Breakdown_2015,TBA_MATCH_SCORE_BREAKDOWN_2015)\
	X(tba::Match_Score_Breakdown_2015_Alliance,TBA_MATCH_SCORE_BREAKDOWN_2015_ALLIANCE)\
	X(tba::Match_Score_Breakdown_2014,TBA_MATCH_SCORE_BREAKDOWN_2014)\


#define X(A,B) void examine(std::vector<string>,vector<A> const&);
KNOWN(X)
#undef X

void examine(vector<string>,vector<int> const&);
void examine(vector<string>,vector<bool> const&);
void examine(vector<string>,vector<short> const&);
void examine(vector<string>,vector<double> const&);

void examine(vector<string>,vector<tba::Yes_no> const&);

void examine(vector<string>,vector<tba::Endgame_2025> const&){
}

void examine(vector<string>,vector<tba::Endgame_2024> const&){
}

void examine(vector<string>,vector<tba::Ignore> const&){
}

#define TO_IGNORE(X) void examine(vector<string>,vector<X> const&){}
TO_IGNORE(tba::Auto_charge_station)
TO_IGNORE(tba::End_charge_station)
TO_IGNORE(tba::Bridge_state)
TO_IGNORE(tba::Endgame_2022)
TO_IGNORE(tba::Init_line)
TO_IGNORE(tba::Endgame)

template<typename T>
auto examine(std::vector<std::string> path,std::vector<T> const& data){
	if(sizeof(T)==1){
		return;
	}
	path|=type_string(*(T*)0);
	auto s=to_set(data);
	cout<<path<<":"<<sizeof(T)<<"\t"<<s.size()<<"\n";
}

auto examine(std::vector<string> path,std::vector<string> const& data){
	path|="string";
	auto s=to_set(data);
	if(s.size()<20){
		cout<<path<<":"<<s.size()<<" "<<s<<"\n";
	}
}

template<typename T>
void examine(std::vector<string> path,vector<optional<T>> const& a){
	path|="optional";
	auto g=group([](auto x){ return !!x; },a);
	if(g[0].empty()){
		cout<<(path|"null")<<g[0].size()<<"\n";
	}
	auto g2=nonempty(g[1]);
	if(g2.empty()){
		cout<<path<<" always null\n";
	}else{
		examine(path,nonempty(g[1]));
	}
}

auto examine(std::vector<string> path,std::vector<tba::Event> const& a){
	path|="Event";
	#define X(A,B) examine(path|string(""#B),mapf([](auto const& x){ return x.B; },a));
	TBA_EVENT(X)
	#undef X
}

template<typename T,size_t N>
void examine(std::vector<std::string> path,std::vector<tba::vector_fixed<T,N>> const& a){
	path|=type_string(*(T*)0);
	auto x=to_set(a);
	auto s=to_set(mapf([](auto x){ return x.size(); },a));
	cout<<path<<": "<<x.size()<<" lengths:"<<min(s)<<" "<<max(s)<<"\n";
}

template<typename...Ts>
void examine(std::vector<std::string> path,std::vector<std::variant<Ts...>> const& a){
	auto g=group(a);
	mapv([=](auto x){ examine(path,x); },g);
}

#define SHOW_INNER(A,B) examine(path|""#B,mapf([](auto const& x){ return x.B; },a));

#define SHOW(NAME,ITEMS) \
	void examine(vector<string> path,vector<NAME> const& a){\
		cout<<path<<" "<<sizeof(NAME)<<"\n";\
		path|=""#NAME;\
		ITEMS(SHOW_INNER)\
	}

KNOWN(SHOW)
//SHOW(tba::Match_Score_Breakdown_2025,TBA_MATCH_SCORE_BREAKDOWN_2025)

auto examine(vector<string> path,vector<tba::Match> const& a){
	path|="Match";
	#define X(A,B) examine(path|string(""#B),mapf([](auto const& x){ return x.B; },a));
	TBA_MATCH(X)
	#undef X
}

void examine(vector<string> path,vector<bool> const& a){
	auto s=to_set(a);
	if(s.size()==2){
		return;
	}
	path|="bool";
	cout<<path<<" always:"<<s<<"\n";
}

void examine(vector<string> path,vector<int> const& a){
	path|="int";
	auto m1=min(a);
	auto m2=max(a);
	if(m1>=std::numeric_limits<short>::min() && m2<=std::numeric_limits<short>::max()){
		cout<<path<<" could be short\n";
	}
}

void examine(vector<string> path,vector<short> const& a){
	(void)path;
	(void)a;
	//path|="short";
	//auto m1=min(a);
	//auto m2=max(a);
	/*if(m1==m2){
		cout<<path<<" "<<m1<<" "<<m2<<"\n";
	}*/
}

void examine(vector<string> path,vector<double> const& a){
	(void)path;
	auto s=to_set(a);
	if(s.size()<3){
		nyi
	}
}

void examine(vector<string> path,vector<tba::Yes_no> const&){
	(void)path;
}

template<typename T>
auto examine(std::vector<T> const& a){
	return examine({},a);
}

int data_range_demo(TBA_fetcher& f){
	//the purpose of this is to identify the data ranges of things that are actually returned from the API
	//this should help refine the types that are used to hold the values.
	//first obvious target of this is the match score breakdowns.  
	//lots of things are declared as optional, that may not need to be
	//also, there may be many things whose ranges are much larger than needed
	//in particular:
	//1) integers that are excessibely wide
	//2) strings that only have<10 values
	//3) variants that don't use all of their options
	//this will not pick up on the cases where there is extra data that doesn't get held by the parser.

	//auto e=events(f);
	//auto e=playoff_matches(f,tba::Event_key("2025orwil"));
	auto e=flatten(mapf(
		[&](auto x){
			return tba::event_matches(f,x);
		},
		events_keys(f)
	));
	auto e2=nonempty(mapf([](auto x){ return x.score_breakdown; },e));
	examine(e2);

	return 0;
}
