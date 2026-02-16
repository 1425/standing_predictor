#ifndef UTIL_H
#define UTIL_H

#include<cassert>
#include<vector>
#include<algorithm>
#include<cmath>
#include<chrono>

#define INST(A,B) A B;

std::string slurp(std::string const& filename);

std::chrono::year_month_day& operator++(std::chrono::year_month_day&);
std::chrono::year_month_day operator+(std::chrono::year_month_day,std::chrono::days);

std::chrono::days operator-(std::chrono::year_month_day,std::chrono::year_month_day);

template<typename T,typename T2>
constexpr std::vector<T>& operator|=(std::vector<T> &a,T2 t){
	a.push_back(std::move(t));
	return a;
}

template<typename T>
auto operator|(std::vector<T> a,T b){
	a|=b;
	return a;
}

template<typename T,template<typename...> typename COLLECTION,typename ...EXTRA>
std::vector<T>& operator|=(std::vector<T> &a,COLLECTION<T,EXTRA...> const& b){
	a.insert(a.end(),b.begin(),b.end());
	return a;
}

template<typename T,template<typename...> typename COLLECTION,typename ...EXTRA>
std::vector<T>& operator|=(std::vector<T> &a,COLLECTION<T,EXTRA...> && b){
	a.insert(
		a.end(),
		std::make_move_iterator(b.begin()),
		std::make_move_iterator(b.end())
	);
	return a;
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
size_t sum(std::vector<bool> const&);

template<typename T>
T sum(std::vector<T> const& a){
	T r{};
	for(auto const& elem:a){
		r+=elem;
	}
	return r;
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

template<typename Func,typename A,typename B>
auto mapf(Func f,std::pair<A,B> const& p){
	return std::make_pair(
		f(p.first),
		f(p.second)
	);
}

#define MAP(F,X) ::mapf([&](auto a){ return (F)(a); },(X))

template<typename A,typename B,typename C,typename D,typename E>
std::tuple<B,C,D,E> tail(std::tuple<A,B,C,D,E> const& t){
	return make_tuple(std::get<1>(t),std::get<2>(t),std::get<3>(t),std::get<4>(t));
}

template<typename T>
std::vector<std::pair<size_t,T>> enumerate_from(size_t start,std::vector<T> const& v){
	std::vector<std::pair<size_t,T>> r;
	for(auto const& elem:v){
		r|=std::make_pair(start++,elem);
	}
	return r;
}

template<typename T,size_t N>
std::array<std::pair<size_t,T>,N> enumerate_from(size_t start,std::array<T,N> const& a){
	using P=std::pair<size_t,T>;
	std::array<P,N> r;
	size_t i=0;
	for(auto const& elem:a){
		r[i]=P(start+i,elem);
		i++;
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

template<typename T>
bool contains(std::vector<T> const& a,T const& b){
	for(auto const& x:a){
		if(x==b){
			return 1;
		}
	}
	return 0;
}

template<typename T>
bool subset(std::vector<T> const& a,std::vector<T> const& b){
	for(auto const& x:a){
		if(!contains(b,x)){
			return 0;
		}
	}
	return 1;
}

template<typename Func,typename T>
std::vector<T> filter(Func f,std::vector<T>&& v){
	//reuse the memory
	std::vector<T> r=std::move(v);
	auto out=r.begin();

	for(auto in=r.begin();in!=r.end();++in){
		if(f(*in)){
			//you have to check for this case because moving an object to itself will leave it in
			//and undefined state.
			if(out!=in){
				*out=std::move(*in);
			}
			out++;
		}
	}

	r.erase(out,r.end());
	return r;
}

#define FILTER(A,B) filter([&](auto const& x){ return (A)(x); },(B))

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

template<typename T>
std::optional<T> maybe_max(std::vector<T> const& a){
	if(a.empty()){
		return std::nullopt;
	}
	return max(a);
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
constexpr std::vector<T> range(T lim){
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

/*template<template<typename...> typename INNER,typename T,typename ... EXTRA>
auto flatten(std::vector<INNER<T,EXTRA...>> a){
	std::vector<T> r;
	for(auto const& elem:a){
		r|=elem;
	}
	return r;
}*/

template<template<typename...> typename INNER,typename T,typename ... EXTRA>
auto flatten(std::vector<INNER<T,EXTRA...>> const& a){
	std::vector<T> r;
	for(auto const& elem:a){
		r|=elem;
	}
	return r;
}

template<template<typename...> typename INNER,typename T,typename ... EXTRA>
auto flatten(std::vector<INNER<T,EXTRA...>> && a){
	std::vector<T> r;
	for(auto& elem:a){
		r|=std::move(elem);
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

std::string as_pct(double);

template<typename T>
std::vector<T> cdr(std::vector<T> a){
	if(a.empty()) return a;
	return std::vector<T>{a.begin()+1,a.end()};
}

auto swap_pairs(auto a){
	return mapf([](auto x){ return make_pair(x.second,x.first); },a);
}

template<typename T>
auto first(T const& t){
	assert(!t.empty());
	return *begin(t);
}

template<typename A,typename B>
auto first(std::pair<A,B> const& a){
	return a.first;
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

template<typename T>
auto adjacent_pairs(std::vector<T> const& a){
	std::vector<std::pair<T,T>> r;
	if(a.size()<2){
		return r;
	}
	for(auto i:range(a.size()-1)){
		r|=std::make_pair(a[i],a[i+1]);
	}
	return r;
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

std::string demangle(const char*);

template<typename T>
std::string type_string(T const& x){
	return demangle(typeid(x).name());
}

bool any(auto const& a){
	for(auto const& x:a){
		if(x){
			return 1;
		}
	}
	return 0;
}

template<typename T>
bool all(std::vector<T> const& a){
	for(auto const& x:a){
		if(!x){
			return 0;
		}
	}
	return 1;
}

bool all_equal(std::pair<long int,bool> const&);

template<typename T,size_t N>
std::vector<T> flatten(std::vector<std::array<T,N>> const& a){
	std::vector<T> r;
	for(auto const& x:a){
		for(auto const& elem:x){
			r|=elem;
		}
	}
	return r;
}

template<typename Func,typename T>
auto count_if(Func f,T const& t){
	auto f1=filter(f,t);
	return f1.size();
}

std::string strip(std::string const&);

bool contains(std::vector<std::string> const&,const char *);

#endif
