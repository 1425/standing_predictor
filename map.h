#ifndef MAP_H
#define MAP_H

#include<map>
#include<vector>
#include<algorithm>
#include<fstream>
#include "util.h"

template<typename K,typename V>
std::vector<V> seconds(std::map<K,V> const& a){
	std::vector<V> r;
	for(auto elem:a){
		r|=elem.second;
	}
	return r;
}

template<typename Func,typename K,typename V>
auto mapf(Func f,std::map<K,V> const& v)->std::vector<decltype(f(*std::begin(v)))>{
	std::vector<decltype(f(*std::begin(v)))> r;
	for(auto p:v){
		r.push_back(f(p));
	}
	return r;
}

template<typename Func,typename K,typename V>
auto map_values(Func f,std::map<K,V> m)->std::map<K,decltype(f(begin(m)->second))>{
	std::map<K,decltype(f(begin(m)->second))> r;
	for(auto [k,v]:m){
		r[k]=f(v);
	}
	return r;
}

template<typename K,typename V>
std::map<K,V> to_map(std::vector<std::pair<K,V>> v){
	std::map<K,V> r;
	for(auto p:v){
		auto f=r.find(p.first);
		assert(f==r.end());
		r[p.first]=p.second;
	}
	return r;
}

template<typename K,typename V>
auto values(std::map<K,V> a)->std::vector<V>{
	std::vector<V> r;
	std::transform(a.begin(),a.end(),back_inserter(r),[](auto x){ return x.second; });
	return r;
}

template<typename K,typename V>
std::vector<std::pair<K,V>> to_vec(std::map<K,V> const& m){
	return std::vector<std::pair<K,V>>{m.begin(),m.end()};
}

template<typename K,typename V>
std::ostream& operator<<(std::ostream& o,std::map<K,V> const& a){
	return o<<to_vec(a);
}

template<typename Func,typename T>
auto group(Func f,std::vector<T> const& v){
	using K=decltype(f(v[0]));
	std::map<K,std::vector<T>> r;
	for(auto x:v){
		r[f(x)]|=x;
	}
	return r;
}

template<typename K,typename V>
std::map<K,V>& operator+=(std::map<K,V>& a,std::map<K,V> const& b){
	for(auto [k,v]:b){
		a[k]+=v;
	}
	return a;
}

template<typename K,typename V>
std::map<V,std::vector<K>> invert(std::map<K,V> const& a){
	std::map<V,std::vector<K>> r;
	for(auto [k,v]:a){
		r[v]|=k;
	}
	return r;
}

#endif
