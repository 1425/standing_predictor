#ifndef UTIL_H
#define UTIL_H

#include<cassert>
#include<algorithm>
#include<chrono>

#define INST(A,B) A B;

#define MAP(F,X) ::mapf([&](auto a){ return (F)(a); },(X))
#define FILTER(A,B) filter([&](auto const& x){ return (A)(x); },(B))

#define RM_CONST(X) typename std::remove_cv<X>::type
#define RM_REF(X) typename std::remove_reference<X>::type
#define ELEM(X) RM_CONST(RM_REF(decltype(*std::begin(X))))

int max(short,int);
int min(int,short);

std::string slurp(std::string const& filename);
std::string tolower(std::string const&);
bool prefix(std::string const& whole,std::string const& p);
bool suffix(std::string const& whole,std::string const&);
std::string strip(std::string const&);
bool contains(std::vector<std::string> const&,const char *);

std::string as_pct(double);

std::string demangle(const char*);

template<typename T>
std::string type_string(T const& x){
	return demangle(typeid(x).name());
}

auto square(auto x){
	return x*x;
}

short coerce(int a,short const*);

//start chrono section

std::chrono::year_month_day& operator++(std::chrono::year_month_day&);
std::chrono::year_month_day operator+(std::chrono::year_month_day,std::chrono::days);

std::chrono::days operator-(std::chrono::year_month_day,std::chrono::year_month_day);

//start tuple section

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

template<typename Func,typename A,typename B,typename C>
auto mapf(Func f,std::tuple<A,B,C> const& t)
	#define G(N) decltype(f(std::get<N>(t)))
	-> std::tuple<G(0),G(1),G(2)>
	#undef G
{
	return std::make_tuple(
		#define X(N) f(std::get<N>(t))
		X(0),X(1),X(2)
		#undef X
	);
}

template<typename Func,typename A,typename B,typename C,typename D>
auto mapf(Func f,std::tuple<A,B,C,D> const& t)
	#define G(N) decltype(f(std::get<N>(t)))
	-> std::tuple<G(0),G(1),G(2),G(3)>
	#undef G
{
	return std::make_tuple(
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
	return std::make_tuple(
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
	return std::make_tuple(
		#define X(N) f(std::get<N>(t))
		X(0),X(1),X(2),X(3),X(4),X(5)
		#undef X
	);
}

template<typename A,typename B,typename C,typename D,typename E>
std::tuple<B,C,D,E> tail(std::tuple<A,B,C,D,E> const& t){
	return make_tuple(std::get<1>(t),std::get<2>(t),std::get<3>(t),std::get<4>(t));
}

template<typename A,typename B,typename C,typename D,typename E,typename F>
auto zip(std::tuple<A,B,C,D,E,F> const& a,std::tuple<A,B,C,D,E,F> const& b){
	return std::make_tuple(
		#define X(N) std::make_pair(get<N>(a),get<N>(b))
		X(0),X(1),X(2),X(3),X(4),X(5)
		#undef X
	);
}

template<typename A,typename B,typename C>
auto zip(std::tuple<A,B,C> const& a,std::tuple<A,B,C> const& b){
	return std::make_tuple(
		#define X(N) std::make_pair(get<N>(a),get<N>(b))
		X(0),X(1),X(2)
		#undef X
	);
}

template<typename A,typename B,typename C,typename D,typename E,typename F>
auto sum(std::tuple<A,B,C,D,E,F> const& a){
	return get<0>(a)+get<1>(a)+get<2>(a)+get<3>(a)+get<4>(a)+get<5>(a);
}

//end tuple section

template<typename T>
auto enumerate(T const& t){
	return enumerate_from(0,t);
}

template<template<typename...> typename V,typename ...T>
auto max(V<T...> const& v){
	if(v.empty()){
		throw std::invalid_argument("max needs non-empty sequence");
	}

	auto it=std::max_element(v.begin(),v.end());
	return *it;

	/*using E=std::tuple_element_t<0,std::tuple<T...>>;
	E r=*begin(v);
	for(auto elem:v){
		r=std::max(r,elem);
	}
	return r;*/
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

template<typename T>
auto first(T const& t){
	assert(!t.empty());
	return *begin(t);
}

auto car(auto const& x){
	assert(!x.empty());
	return *std::begin(x);
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

bool any(auto const& a){
	for(auto const& x:a){
		if(x){
			return 1;
		}
	}
	return 0;
}

template<typename Func,typename T>
auto count_if(Func f,T const& t){
	auto f1=filter(f,t);
	return f1.size();
}

//start std::pair section

template<typename Func,typename A,typename B>
auto mapf(Func f,std::pair<A,B> const& p){
	return std::make_pair(
		f(p.first),
		f(p.second)
	);
}

auto swap_pairs(auto a){
	return mapf([](auto x){ return make_pair(x.second,x.first); },a);
}

template<typename A,typename B>
auto first(std::pair<A,B> const& a){
	return a.first;
}

bool all_equal(std::pair<long int,bool> const&);

template<typename A,typename B>
auto operator-(std::pair<A,B> const& a,std::pair<A,B> const& b){
	return std::make_pair(
		a.first-b.first,
		a.second-b.second
	);
}

template<typename A,typename B,typename C,typename D>
auto operator+(std::pair<A,B> const& a,std::pair<C,D> const& b){
	return std::make_pair(
		a.first+b.first,
		a.second+b.second
	);
}

template<typename A,typename B,typename C,typename D>
std::pair<A,B>& operator+=(std::pair<A,B>& a,std::pair<C,D> const& b){
	a.first+=b.first;
	a.second+=b.second;
	return a;
}

template<typename A,typename B>
bool both_greater_eq(std::pair<A,B> const& a,std::pair<A,B> const& b){
	return a.first>=b.first && a.second>=b.second;
}

template<typename A,typename B>
bool both_less(std::pair<A,B> const& a,std::pair<A,B> const& b){
	return a.first<b.first && a.second<b.second;
}

template<typename A,typename B,typename C,typename D>
bool both_less_eq(std::pair<A,B> const& a,std::pair<C,D> const& b){
	return a.first<=b.first && a.second<=b.second;
}

template<typename A,typename B,typename C>
std::pair<A,B> operator*(std::pair<A,B> a,C c){
	a.first*=c;
	a.second*=c;
	return a;
}

template<typename A,typename B,typename C,typename D>
auto operator-(std::pair<A,B> const& a,std::pair<C,D> const& b){
	return std::make_pair(
		a.first-b.first,
		a.second-b.second
	);
}

template<typename A,typename B,typename C,typename D>
auto& operator-=(std::pair<A,B>& a,std::pair<C,D> const& b){
	a.first-=b.first;
	a.second-=b.second;
	return a;
}

template<typename A,typename B,typename C,typename D>
auto elementwise_max(std::pair<A,B> const& a,std::pair<C,D> const& b){
	return std::make_pair(
		max(a.first,b.first),
		max(a.second,b.second)
	);
}

template<typename A,typename B,typename C,typename D>
auto elementwise_min(std::pair<A,B> const& a,std::pair<C,D> const& b){
	return std::make_pair(
		min(a.first,b.first),
		min(a.second,b.second)
	);
}

template<typename A,typename B>
std::pair<A,B> elementwise_max(std::vector<std::pair<A,B>> const& a){
	return std::make_pair(
		max(firsts(a)),
		max(seconds(a))
	);
}

template<typename A,typename B,typename C,typename D>
std::pair<C,D> coerce(std::pair<A,B> a,std::pair<C,D> const*){
	return std::make_pair(
		coerce(a.first,(C*)0),
		coerce(a.second,(D*)0)
	);
}

#endif
