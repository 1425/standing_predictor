#ifndef OPTIONAL_H
#define OPTIONAL_H

#include<optional>
#include "vector.h"

template<typename T>
std::vector<T>& operator|=(std::vector<T> &a,std::optional<T> const& b){
	if(b){
		a|=*b;
	}
	return a;
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
std::vector<T> operator|(std::vector<T>,std::optional<T>);

/*template<template<typename,typename> typename MAP,typename K,typename V>
std::optional<V> maybe_get(MAP<K,V> const& a,K const& k){
	auto f=a.find(k);
	if(f==a.end()){
		return std::nullopt;
	}
	return f->second;
}*/

template<template<typename,typename> typename MAP,typename K,typename V,typename K2>
std::optional<V> maybe_get(MAP<K,V> const& a,K2 const& k){
	auto f=a.find(k);
	if(f==a.end()){
		return std::nullopt;
	}
	return f->second;
}

std::optional<std::string> strip(std::optional<std::string> const&);
bool prefix(std::optional<std::string> const& whole,std::string const& part);

#endif
