#ifndef VECTOR_H
#define VECTOR_H

#include<vector>
#include<algorithm>
#include<cassert>
#include "util.h" //for ELEM

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

template<typename T,template<typename,size_t>typename Collection,size_t N>
std::vector<T>& operator|=(std::vector<T>& a,Collection<T,N> const& b){
	a.insert(
		a.end(),
		std::make_move_iterator(b.begin()),
		std::make_move_iterator(b.end())
	);
	return a;
}

template<typename T>
auto operator|(std::vector<T> a,std::vector<T> b){
	a|=b;
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

template<typename T>
std::vector<T> operator+(std::vector<T> a,T b){
	a|=b;
	return a;
}

template<typename T>
T sum(std::vector<T> const& a){
	T r{};
	for(auto const& elem:a){
		r+=elem;
	}
	return r;
}

namespace std{
	//Adding to this namespace is ugly, but means the this can be found via ADL
	//which avoids extra dependencies for things that sometimes want to enumerate a vector
	//and sometimes a different type.
	template<typename T>
	std::vector<std::pair<size_t,T>> enumerate_from(size_t start,std::vector<T> const& v){
		std::vector<std::pair<size_t,T>> r;
		for(auto const& elem:v){
			r.push_back(std::make_pair(start++,elem));
		}
		return r;
	}
}

template<typename A,typename B>
std::vector<A> firsts(std::vector<std::tuple<A,B>> a){
	return mapf([](auto x){ return get<0>(x); },a);
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

template<template<typename,size_t> typename Collection,typename T,size_t N>
bool subset(Collection<T,N> const& a,std::vector<T> const& b){
	for(auto const& x:a){
		if(!contains(b,x)){
			return 0;
		}
	}
	return 1;
}

template<typename F,typename T>
std::vector<T> filter(F f,std::vector<T> const& v){
	std::vector<T> r;
	std::copy_if(v.begin(),v.end(),std::back_inserter(r),f);
	return r;
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

template<typename T>
std::optional<T> maybe_max(std::vector<T> const& a){
	if(a.empty()){
		return std::nullopt;
	}
	return max(a);
}

template<typename T>
T max_else(std::vector<T> const& a,T b){
	if(a.empty()){
		return b;
	}
	return max(a);
}

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

template<typename T>
bool all_equal(std::vector<T> const& a){
	for(auto elem:a){
		if(elem!=a[0]){
			return 0;
		}
	}
	return 1;
}

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

	size_t size=0;
	for(auto const& elem:a){
		size+=elem.size();
	}
	r.reserve(size);

	for(auto& elem:a){
		r|=std::move(elem);
	}
	return r;
}

template<template<typename,size_t>typename Collection,typename T,size_t N>
auto flatten(std::vector<Collection<T,N>> const& a){
	std::vector<T> r;
	for(auto const& elem:a){
		r|=elem;
	}
	return r;
}

template<typename T>
std::vector<T> take(size_t n,std::vector<T> const& v){
	return std::vector<T>{v.begin(),v.begin()+std::min(n,v.size())};
}

template<template<typename,size_t>typename Collection,typename T,size_t N>
auto take(size_t n,Collection<T,N> const& a){
	std::vector<T> r;
	size_t lim=min(n,a.size());
	for(size_t i=0;i<lim;i++){
		r|=a[i];
	}
	return r;
}

template<typename Func,template<typename,size_t>typename Collection,typename T,size_t N>
auto filter(Func f,Collection<T,N> const& a){
	std::vector<T> r;
	for(auto const& x:a){
		if(f(x)){
			r|=x;
		}
	}
	return r;
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
std::vector<T> cdr(std::vector<T> a){
	if(a.empty()) return a;
	return std::vector<T>{a.begin()+1,a.end()};
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

template<typename T>
bool all(std::vector<T> const& a){
	for(auto const& x:a){
		if(!x){
			return 0;
		}
	}
	return 1;
}

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

template<typename T,size_t N>
std::vector<T> flatten(std::array<std::vector<T>,N> const& a){
	std::vector<T> r;
	for(auto const& x:a){
		r|=x;
	}
	return r;
}

template<typename T>
auto to_vec(std::tuple<T,T,T> const& a){
	return std::array<T,3>{get<0>(a),get<1>(a),get<2>(a)};
}

template<typename T>
auto sorted(std::tuple<T,T,T> a){
	auto v=to_vec(a);
	std::sort(v.begin(),v.end());
	return std::make_tuple(v[0],v[1],v[2]);
}

std::string consolidate(std::vector<int> const&);
std::vector<std::string> find(std::string const& base,std::string const& name);
std::vector<char> to_vec(std::string const&);
double sum(std::vector<int> const&);
double sum(std::vector<double> const& v);
size_t sum(std::vector<bool> const&);

template<typename T>
std::vector<T> duplicate(T t,size_t n){
	std::vector<T> r;
	for(auto _:range(n)){
		r|=t;
	}
	return r;
}

#endif
