#ifndef UTIL_H
#define UTIL_H

#include<cassert>
#include<sstream>
#include<iostream>
#include<vector>
#include<optional>
#include<algorithm>
#include<map>
#include<cmath>

#define PRINT(X) { std::cout<<""#X<<":"<<(X)<<"\n"; }
#define nyi { std::cout<<"nyi "<<__FILE__<<":"<<__LINE__<<"\n"; exit(44); }

std::string slurp(std::string const& filename);

template<typename T,typename T2>
std::vector<T>& operator|=(std::vector<T> &a,T2 t){
	a.push_back(std::move(t));
	return a;
}

template<typename T>
std::vector<T>& operator|=(std::vector<T> &a,std::optional<T> const& b){
	if(b){
		a|=*b;
	}
	return a;
}

template<typename T,template<typename...> typename COLLECTION,typename ...EXTRA>
std::vector<T>& operator|=(std::vector<T> &a,COLLECTION<T,EXTRA...> const& b){
	for(auto const& elem:b){
		a|=elem;
	}
	return a;
}

//definition in map.h.
template<typename K,typename V>
std::ostream& operator<<(std::ostream&,std::map<K,V> const&);

template<typename A,typename B>
std::ostream& operator<<(std::ostream&,std::pair<A,B> const&);

template<typename T>
std::ostream& operator<<(std::ostream&,std::optional<T> const&);

template<typename A,typename B>
std::ostream& operator<<(std::ostream& o,std::tuple<A,B> const& t){
	o<<"(";
	o<<std::get<0>(t)<<" ";
	o<<std::get<1>(t);
	return o<<")";
}

template<typename A,typename B,typename C>
std::ostream& operator<<(std::ostream& o,std::tuple<A,B,C> const& t){
	o<<"(";
	o<<std::get<0>(t)<<" ";
	o<<std::get<1>(t)<<" ";
	o<<std::get<2>(t);
	return o<<")";
}

template<typename A,typename B,typename C,typename D>
std::ostream& operator<<(std::ostream& o,std::tuple<A,B,C,D> const& t){
	o<<"(";
	o<<std::get<0>(t)<<" ";
	o<<std::get<1>(t)<<" ";
	o<<std::get<2>(t)<<" ";
	o<<std::get<3>(t);
	return o<<")";
}

template<typename A,typename B,typename C,typename D,typename E>
std::ostream& operator<<(std::ostream& o,std::tuple<A,B,C,D,E> const& t){
	o<<"(";
	o<<std::get<0>(t)<<" ";
	o<<std::get<1>(t)<<" ";
	o<<std::get<2>(t)<<" ";
	o<<std::get<3>(t)<<" ";
	o<<std::get<4>(t);
	return o<<")";
}

template<typename T>
std::ostream& operator<<(std::ostream& o,std::vector<T> const& v){
	o<<"[ ";
	for(auto x:v){
		o<<x<<" ";
	}
	return o<<"]";
}

template<typename A,typename B>
std::ostream& operator<<(std::ostream& o,std::pair<A,B> const& a){
	return o<<"("<<a.first<<","<<a.second<<")";
}

template<typename T>
std::ostream& operator<<(std::ostream& o,std::optional<T> const& a){
	if(a) return o<<*a;
	return o<<"NULL";
}

std::ostream& operator<<(std::ostream&,std::invalid_argument const&);

template<typename T>
void print_lines(T const& t){
	for(auto elem:t){
		std::cout<<elem<<"\n";
	}
}

template<typename T>
std::vector<T> operator+(std::vector<T> a,std::vector<T> const& b){
	a.insert(a.end(),b.begin(),b.end());
	return a;
}

template<typename T>
std::vector<T> operator+(std::vector<T> a,std::tuple<T,T,T,T> const& t){
	a|=std::get<0>(t);
	a|=std::get<1>(t);
	a|=std::get<2>(t);
	a|=std::get<3>(t);
	return a;
}

template<typename A,typename B,typename C,typename D,typename E>
std::tuple<A,B,C,D,E> operator|(std::tuple<A> const& a,std::tuple<B,C,D,E> const& b){
	return make_tuple(
		std::get<0>(a),
		std::get<0>(b),
		std::get<1>(b),
		std::get<2>(b),
		std::get<3>(b)
	);
}

template<typename A1,typename A,typename B,typename C,typename D,typename E>
std::tuple<A1,A,B,C,D,E> operator|(std::tuple<A1,A> const& a,std::tuple<B,C,D,E> const& b){
	return make_tuple(
		std::get<0>(a),
		std::get<1>(a),
		std::get<0>(b),
		std::get<1>(b),
		std::get<2>(b),
		std::get<3>(b)
	);
}


template<typename A,typename B,typename C,typename D,typename E,typename F>
auto operator+=(std::tuple<A,B,C,D,E,F>& a,std::tuple<A,B,C,D,E,F> const& b){
	#define X(N) std::get<N>(a)+=std::get<N>(b);
	X(0) X(1) X(2) X(3) X(4) X(5)
	#undef X
        return a;
}

template<typename A,typename B,typename C,typename D,typename E>
auto operator+=(std::tuple<A,B,C,D,E>& a,std::tuple<A,B,C,D,E> const& b){
	#define X(N) std::get<N>(a)+=std::get<N>(b);
	X(0) X(1) X(2) X(3) X(4)
	#undef X
        return a;
}

template<typename T>
std::vector<T> operator+(std::vector<T> a,T b){
	a|=b;
	return a;
}

double sum(std::vector<double> const& v);

template<typename T>
T sum(std::vector<T> const& a){
	T r{};
	for(auto const& elem:a){
		r+=elem;
	}
	return r;
}

class vector_void{
	public:
	explicit vector_void(size_t){}
};

std::ostream& operator<<(std::ostream& o,vector_void);

template<typename Func,typename T>
auto mapf(Func f,std::vector<T> const& v){
	using E=decltype(f(*std::begin(v)));
	if constexpr(std::is_same<void,E>()){
		for(auto const& elem:v){
			f(elem);
		}
		return vector_void{v.size()};
	}else{
		std::vector<decltype(f(v[0]))> r;
		for(auto const& elem:v){
			r|=f(elem);
		}
		return r;
	}
}

template<typename Func,typename A,typename B,typename C,typename D>
auto mapf(Func f,std::tuple<A,B,C,D> const& t)
	#define G(N) decltype(f(std::get<N>(t)))
	-> std::tuple<G(0),G(1),G(2),G(3)>
	#undef G
{
	return make_tuple(
		#define X(N) f(std::get<N>(t))
		X(0),X(1),X(2),X(3)
		#undef X
	);
}

template<typename Func,typename A,typename B,typename C,typename D,typename E>
auto mapf(Func f,std::tuple<A,B,C,D,E> const& t)
	#define G(N) decltype(f(std::get<N>(t)))
	-> std::tuple<G(0),G(1),G(2),G(3),G(4)>
	#undef G
{
	return make_tuple(
		#define X(N) f(std::get<N>(t))
		X(0),X(1),X(2),X(3),X(4)
		#undef X
	);
}

template<typename Func,typename A,typename B,typename C,typename D,typename E,typename F>
auto mapf(Func f,std::tuple<A,B,C,D,E,F> const& t)
	#define G(N) decltype(f(std::get<N>(t)))
	-> std::tuple<G(0),G(1),G(2),G(3),G(4),G(5)>
	#undef G
{
	return make_tuple(
		#define X(N) f(std::get<N>(t))
		X(0),X(1),X(2),X(3),X(4),X(5)
		#undef X
	);
}

template<typename Func,typename A,typename B>
auto mapf(Func f,std::pair<A,B> const& p){
	return make_pair(
		f(p.first),
		f(p.second)
	);
}

#define MAP(F,X) ::mapf([&](auto a){ return (F)(a); },(X))

void indent(int levels);

std::vector<std::string> split(std::string const&);
std::vector<std::string> split(std::string const&,char);

template<typename T>
std::string as_string(T const& t){
	std::stringstream ss;
	ss<<t;
	return ss.str();
}

template<typename T>
std::string join(std::vector<T> const& a){
	std::stringstream ss;
	for(auto elem:a){
		ss<<elem;
	}
	return ss.str();
}

template<typename A,typename B,typename C,typename D,typename E>
std::string join(std::tuple<A,B,C,D,E> const& t){
	std::stringstream ss;
	#define X(N) ss<<std::get<N>(t);
	X(0) X(1) X(2) X(3) X(4)
	#undef X
	return ss.str();
}

template<typename A,typename B,typename C,typename D,typename E,typename F>
std::string join(std::tuple<A,B,C,D,E,F> const& t){
	std::stringstream ss;
	#define X(N) ss<<std::get<N>(t);
	X(0) X(1) X(2) X(3) X(4) X(5)
	#undef X
	return ss.str();
}

template<typename A,typename B>
std::string join(std::pair<A,B> const& p){
	std::stringstream ss;
	ss<<p.first;
	ss<<p.second;
	return ss.str();
}

template<typename T>
std::string tag(std::string const& name,T const& body){
	std::stringstream ss;
	ss<<"<"<<name<<">"<<body<<"</"<<split(name).at(0)<<">";
	return ss.str();
}

template<typename T>
auto tr(T t){ return tag("tr",t); }

template<typename T>
auto td(T t){ return tag("td",t); }

std::string td1(std::string const&);

template<typename T>
std::string table(T const& body){ return tag("table",body); }

template<typename T>
auto h2(T const& t){ return tag("h2",t); }

template<typename T>
auto h3(T const& t){ return tag("h3",t); }

template<typename T>
auto th(T const& t){ return tag("th",t); }

std::string th1(std::string const&);

std::string link(std::string const& url,std::string const& body);

template<typename A,typename B,typename C,typename D,typename E>
std::tuple<B,C,D,E> tail(std::tuple<A,B,C,D,E> const& t){
	return make_tuple(std::get<1>(t),std::get<2>(t),std::get<3>(t),std::get<4>(t));
}

template<typename T>
std::vector<std::pair<size_t,T>> enumerate_from(size_t start,std::vector<T> const& v){
	std::vector<std::pair<size_t,T>> r;
	for(auto elem:v){
		r|=std::make_pair(start++,elem);
	}
	return r;
}

template<typename T>
auto enumerate(T const& t){
	return enumerate_from(0,t);
}

template<typename A,typename B>
std::vector<B> seconds(std::vector<std::pair<A,B>> a){
	std::vector<B> r;
	for(auto elem:a) r|=elem.second;
	return r;
}

template<typename A,typename B,typename C,typename D,typename E>
std::vector<B> seconds(std::vector<std::tuple<A,B,C,D,E>> const& v){
	std::vector<B> r;
	for(auto t:v){
		r|=std::get<1>(t);
	}
	return r;
}

template<typename A,typename B,typename C,typename D,typename E,typename F,typename G,typename H,typename I>
std::vector<B> seconds(std::vector<std::tuple<A,B,C,D,E,F,G,H,I>> const& v){
	std::vector<B> r;
	for(auto t:v){
		r|=std::get<1>(t);
	}
	return r;
}

template<typename F,typename T>
std::vector<T> filter(F f,std::vector<T> const& v){
	std::vector<T> r;
	std::copy_if(v.begin(),v.end(),std::back_inserter(r),f);
	return r;
}

#define FILTER(A,B) filter([&](auto x){ return (A)(x); },(B))

template<typename Func,typename T>
T filter_unique(Func f,std::vector<T> const& a){
	/*auto found=filter(f,a);
	assert(found.size()==1);
	return std::move(found[0]);*/

	//The version below is meant to be faster by not making the list
	//this does not make a major difference though.
	
	auto it=a.begin();
	while(it!=a.end() && !f(*it)){
		++it;
	}

	if(it==a.end()){
		assert(0);
	}

	auto r=*it;

	++it;
	while(it!=a.end() && !f(*it)){
		++it;
	}
	assert(it==a.end());
	return r;
}

template<typename T>
T choose(std::vector<T> const& v){
	return v[rand()%v.size()];
}

template<typename T>
std::vector<T> sorted(std::vector<T> a){
	std::sort(begin(a),end(a));
	return a;
}

template<typename T,typename Func>
std::vector<T> sorted(std::vector<T> v,Func f){
	sort(begin(v),end(v),[&](auto a,auto b){ return f(a)<f(b); });
	return v;
}

template<typename T>
std::vector<T> reversed(std::vector<T> a){
	reverse(begin(a),end(a));
	return a;
}

template<template<typename...> typename V,typename ...T>
auto max(V<T...> const& v){
	assert(!v.empty());
	using E=std::tuple_element_t<0,std::tuple<T...>>;
	E r=*begin(v);
	for(auto elem:v){
		r=std::max(r,elem);
	}
	return r;
}

template<template<typename...> typename V,typename ...T>
auto min(V<T...> const& v){
	assert(!v.empty());
	using E=std::tuple_element_t<0,std::tuple<T...>>;
	E r=*begin(v);
	for(auto elem:v){
		r=std::min(r,elem);
	}
	return r;
}

double sum(std::vector<int> const&);

template<typename T>
double mean(std::vector<T> const& v){
	return sum(v)/v.size();
}

template<typename T>
T median(std::vector<T> const& v){
	assert(v.size());
	return sorted(v)[v.size()/2];
}

template<typename T>
std::vector<T> range(T start,T lim,T step){
	std::vector<T> r;
	for(auto i=start;i<lim;i+=step){
		r|=i;
	}
	return r;
}

template<typename T>
constexpr std::vector<T> range(T start,T lim){
	std::vector<T> r;
	for(auto i=start;i<lim;i++){
		r|=i;
	}
	return r;
}

template<typename T>
std::vector<T> range(T lim){
	return range(T{0},lim);
}

template<typename T>
std::vector<T> skip(size_t n,std::vector<T> const& a){
	if(n>a.size()) return {};
	return std::vector<T>{a.begin()+n,a.end()};
}

template<typename T>
T last(std::vector<T> const& a){
	assert(a.size());
	return a[a.size()-1];
}

std::string tolower(std::string const&);
bool prefix(std::string const& whole,std::string const& p);

template<typename A,typename B>
std::vector<std::pair<A,B>> zip(std::vector<A> const& a,std::vector<B> const& b){
	return mapf(
		[&](auto i){ return std::make_pair(a[i],b[i]); },
		range(std::min(a.size(),b.size()))
	);
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

std::vector<char> to_vec(std::string const&);

template<typename T>
bool all_equal(std::vector<T> const& a){
	for(auto elem:a){
		if(elem!=a[0]){
			return 0;
		}
	}
	return 1;
}

std::vector<std::string> find(std::string const& base,std::string const& name);

template<template<typename...> typename INNER,typename T,typename ... EXTRA>
auto flatten(std::vector<INNER<T,EXTRA...>> a){
	std::vector<T> r;
	for(auto const& elem:a){
		r|=elem;
	}
	return r;
}

template<typename T>
auto flatten(std::vector<std::optional<T>> a){
	std::vector<T> r;
	for(auto elem:a){
		r|=elem;
	}
	return r;
}

template<typename T>
bool operator==(std::optional<T> const& a,std::optional<T> const& b){
	if(a){
		if(b){
			return *a==*b;
		}
		return 0;
	}
	return !b;
}

template<typename T>
std::vector<T> nonempty(std::vector<std::optional<T>> const& a){
	std::vector<T> r;
	for(auto elem:a){
		if(elem){
			r|=*elem;
		}
	}
	return r;
}

template<typename T>
std::vector<T> take(size_t n,std::vector<T> const& v){
	return std::vector<T>{v.begin(),v.begin()+std::min(n,v.size())};
}

template<typename Func,typename T>
auto sort_by(std::vector<T> a,Func f){
	std::sort(
		a.begin(),
		a.end(),
		[&](auto a,auto b){
			return f(a)<f(b);
		}
	);
	return a;
}

auto sort_by(auto a,auto f){
	return sort_by(to_vec(a),f);
}

auto head(auto x){
	return take(10,x);
}

template<typename T>
auto seconds(std::vector<std::vector<T>> const& a){
	return mapf(
		[](auto x){
			assert(x.size()>=2);
			return x[1];
		},
		a
	);
}

template<typename T>
auto second(std::vector<T> const& a){
	assert(a.size()>=2);
	return a[1];
}

template<typename T,size_t N>
auto second(std::array<T,N> const& a){
	static_assert(N>=2);
	return a[1];
}

template<typename T>
auto seconds(std::vector<T> const& a){
	return MAP(second,a);
}

std::string as_pct(double);

template<typename T>
auto keys(T const& t){
	return ::mapf([](auto x){ return x.key; },t);
}

template<typename T>
std::vector<T> operator|(std::vector<T>,std::optional<T>);

template<size_t N>
std::array<size_t,N> range_st(){
	std::array<size_t,N> r;
	for(size_t i=0;i<N;i++){
		r[i]=i;
	}
	return r;
}

template<typename T>
std::vector<T> cdr(std::vector<T> a){
	if(a.empty()) return a;
	return std::vector<T>{a.begin()+1,a.end()};
}

template<typename Func,typename T,size_t N>
auto mapf(Func f,std::array<T,N> const& a){
	using E=decltype(f(*begin(a)));
	std::array<E,N> r;
	for(auto i:range_st<N>()){
		r[i]=f(a[i]);
	}
	return r;
}

template<typename T,size_t N>
std::array<T,N> as_array(std::vector<T> const& a){
	assert(a.size()==N);
	return mapf([&](auto x){ return a[x]; },range_st<N>());
}

template<typename T,size_t N>
std::ostream& operator<<(std::ostream& o,std::array<T,N> const& a){
	o<<"[ ";
	for(auto elem:a){
		o<<elem<<" ";
	}
	return o<<"]";
}

auto swap_pairs(auto a){
	return mapf([](auto x){ return make_pair(x.second,x.first); },a);
}

template<typename T>
auto first(T const& t){
	assert(!t.empty());
	return *begin(t);
}

template<typename T>
auto firsts(T const& t){
	return MAP(first,t);
}

auto square(auto x){
	return x*x;
}

template<typename T>
auto variance(std::vector<T> v)->double{
	auto mu=mean(v);
	return mean(mapf(
		[mu](auto x){ return square(x-mu); },
		v
	));
}

template<typename T>
auto std_dev(std::vector<T> v){
	return sqrt(variance(v));
}

template<typename T>
auto mad(std::vector<T> v)->double{
	//mean absolute devaition
	auto mu=mean(v);
	return mean(mapf(
		[mu](auto x){ return fabs(x-mu); },
		v
	));
}

#define RM_CONST(X) typename std::remove_cv<X>::type
#define RM_REF(X) typename std::remove_reference<X>::type
#define ELEM(X) RM_CONST(RM_REF(decltype(*std::begin(X))))

auto cross(auto a,auto b){
	using A=ELEM(a);
	using B=ELEM(b);
	std::vector<std::pair<A,B>> r;
	for(auto a1:a){
		for(auto b1:b){
			r|=std::make_pair(a1,b1);
		}
	}
	return r;
}

template<typename T>
auto to_array(std::pair<T,T> a){
	return std::array<T,2>{a.first,a.second};
}

auto quartiles(auto a){
	assert(!a.empty());
	auto b=sorted(a);
	return std::array{b[b.size()/4],b[b.size()/2],b[b.size()*3/4]};
}


template<typename Func,typename T>
auto filter_first(Func f,T const& t){
	for(auto const& elem:t){
		if(f(elem)){
			return elem;
		}
	}
	assert(0);
}

std::string consolidate(std::vector<int> const&);

#endif
