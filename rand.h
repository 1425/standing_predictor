#ifndef RAND_H
#define RAND_H

#include<variant>
#include "vector_void.h"

namespace tba{
	class Team_key;
	class Event_key;
};

tba::Team_key rand(tba::Team_key const*);
tba::Event_key rand(tba::Event_key const*);

bool rand(bool const*);
short rand(short const*);
unsigned short rand(unsigned short const*);
int rand(int const*);
unsigned rand(unsigned const*);
double rand(double*);
std::string rand(std::string const*);

template<typename T,size_t N>
auto rand(std::array<T,N> const*);

template<typename A,typename B>
auto rand(std::pair<A,B> const*){
	return std::make_pair(rand((A*)0),rand((B*)0));
}

template<typename T>
std::vector<T> rand(std::vector<T> const*){
	return mapf(
		[](auto x){
			(void)x;
			return rand((T*)0);
		},
		range(rand()%5)
	);
}

template<typename T>
std::optional<T> rand(std::optional<T> const*){
	if(rand()%2){
		return rand((T*)0);
	}
	return std::nullopt;
}

template<typename K,typename V>
std::map<K,V> rand(std::map<K,V> const*){
	std::map<K,V> r;
	for(auto k:rand((std::vector<K>*)0)){
		//r[k]=rand((V*)0);
		r.insert(std::make_pair(k,rand((V*)0)));
	}
	return r;
}

template<typename A,typename B>
std::variant<A,B> rand(std::variant<A,B> const*){
	if(rand()%2){
		return rand((A*)0);
	}
	return rand((B*)0);
}

template<typename A,typename B,typename C,typename D>
std::variant<A,B,C,D> rand(std::variant<A,B,C,D> const*){
	switch(rand()%4){
		case 0: return rand((A*)0);
		case 1: return rand((B*)0);
		case 2: return rand((C*)0);
		case 3: return rand((D*)0);
		default:
			assert(0);
	}
}

template<typename T,size_t N>
auto rand(std::array<T,N> const*){
	return mapf(
		[](auto _){
			(void)_;
			return rand((T*)0);
		},
		range_st<N>()
	);
}

#endif
