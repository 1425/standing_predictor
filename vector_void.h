#ifndef VECTOR_VOID_H
#define VECTOR_VOID_H

#include "util.h"
#include "set.h"
#include "array.h"

class vector_void{
	size_t size_;

	public:
	explicit vector_void(size_t);

	size_t size()const;
};

std::ostream& operator<<(std::ostream& o,vector_void);

std::set<int> to_set(vector_void const&);

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

template<typename T>
auto keys(T const& t){
	return ::mapf([](auto x){ return x.key; },t);
}

template<typename T>
auto seconds(std::vector<T> const& a){
	return MAP(second,a);
}

template<typename T>
auto firsts(T const& t){
	return MAP(first,t);
}

template<typename A,typename B>
std::vector<std::pair<A,B>> zip(std::vector<A> const& a,std::vector<B> const& b){
	return mapf(
		[&](auto i){ return std::make_pair(a[i],b[i]); },
		range(std::min(a.size(),b.size()))
	);
}

template<typename A,typename B,typename C,typename D,typename E,typename F>
size_t match_degree(std::tuple<A,B,C,D,E,F> const& a,std::tuple<A,B,C,D,E,F> const& b){
	return sum(mapf([](auto p){ return p.first==p.second; },zip(a,b)));
}

#endif
