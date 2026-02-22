#ifndef MAP_H
#define MAP_H

#include<map>
#include<vector>
#include<algorithm>
#include<fstream>
#include "io.h"
#include "optional.h"
#include "vector.h"

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

#define MAP_VALUES(A,B) map_values([&](auto x){ return (A)(x); },(B))

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

#define GROUP(A,B) group([&](auto const& x){ return (A)(x); },(B))

template<typename Func,typename T>
auto group(Func f,std::vector<T> const& v){
	using K=decltype(f(v[0]));
	std::map<K,std::vector<T>> r;
	for(auto const& x:v){
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

template<typename K,typename V>
auto take(size_t n,std::map<K,V> const& a){
	return take(n,to_vec(a));
}

template<typename K,typename V>
auto adjacent_pairs(std::map<K,V> const& a){
	return adjacent_pairs(to_vec(a));
}

template<typename K,typename V,typename T>
auto zip(std::map<K,V> const& a,std::vector<T> const& b){
	auto ai=a.begin();
	auto ae=a.end();

	auto bi=b.begin();
	auto be=b.end();

	using P=std::pair<std::pair<K,V>,T>;
	std::vector<P> r;
	while(ai!=ae && bi!=be){
		r|=P(*ai,*bi);

		++ai;
		++bi;
	}
	return r;
}

template<
	template<typename,typename> typename MAP1,
	template<typename,typename> typename MAP2,
	typename K,
	typename V1,
	typename V2
>
auto join(MAP1<K,V1> const& a,MAP2<K,V2> const& b){
	using P=std::pair<std::optional<V1>,std::optional<V2>>;
	std::map<K,P> r;
	for(auto k:keys(a)|keys(b)){
		r[k]=P(maybe_get(a,k),maybe_get(b,k));
	}
	return r;
}

template<typename K,typename V>
auto dict(std::vector<std::pair<K,V>> const& a){
	std::map<K,V> r;
	for(auto [k,v]:a){
		r[k]=v;
	}
	return r;
}

template<typename K,typename V>
auto reverse_pairs(std::map<K,V> const& a){
	using P=std::pair<V,K>;
	std::vector<P> r;
	for(auto const& x:a){
		r|=P(x.second,x.first);
	}
	return r;
}

template<typename K,typename V>
auto sorted(std::map<K,V> const& a){
	return sorted(to_vec(a));
}

template<typename K,typename V>
V get(std::map<K,V> const& a,auto const& k){
	auto f=a.find(k);
	assert(f!=a.end());
	return f->second;
}

template<typename K,typename V>
V get(std::map<K,V> const& a,auto const& k,auto const& otherwise){
	auto f=a.find(k);
	if(f==a.end()){
		return otherwise;
	}
	return f->second;
}

template<typename K,typename V>
auto get_key(std::map<K,V> const& a,K const& k){
	auto f=a.find(k);
	assert(f!=a.end());
	return f->second;
}

template<typename Func,typename K,typename V>
auto group(Func f,std::map<K,V> const& a){
	return group(f,to_vec(a));
}

#endif
