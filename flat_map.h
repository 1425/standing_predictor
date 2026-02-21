#ifndef FLAT_MAP_H
#define FLAT_MAP_H

#include<algorithm>
#include "map.h"

template<typename K,typename V>
class flat_map{
	using Data=std::vector<std::pair<K,V>>;
	//this is always kept sorted.
	Data data;

	public:
	flat_map(){}

	explicit flat_map(std::initializer_list<std::pair<K,V>> a):data(a){
		sort(data.begin(),data.end());
	}

	explicit flat_map(std::map<K,V> const& a):data(a.begin(),a.end()){}

	explicit flat_map(std::vector<std::pair<K,V>>&& a):data(a){
		sort(data.begin(),data.end());
		//could check that there are no duplicate keys
		//for now, will just assume that there are no duplicates
	}

	/*operator std::map<K,V>()const{
		return std::map<K,V>{data.begin(),data.end()};
	}*/

	using const_reverse_iterator=Data::const_reverse_iterator;

	auto rbegin()const{
		return data.rbegin();
	}

	auto rend()const{
		return data.rend();
	}

	using const_iterator=Data::const_iterator;

	auto find(K const& k)const{
		auto f=std::lower_bound(
			data.begin(),
			data.end(),
			std::make_pair(k,V{}),
			[](auto a,auto b){ return a.first<b.first; }
		);
		if(f==data.end() || f->first!=k){
			return data.end();
		}
		return f;
	}

	auto find(K const& k){
		auto f=std::lower_bound(
			data.begin(),
			data.end(),
			std::make_pair(k,V{}),
			[](auto a,auto b){ return a.first<b.first; }
		);
		if(f==data.end() || f->first!=k){
			return data.end();
		}
		return f;
	}

	auto begin()const{ return data.begin(); }
	auto end()const{ return data.end(); }

	V& operator[](K const& k){
		auto f=std::lower_bound(
			data.begin(),
			data.end(),
			std::make_pair(k,V{}),
			[](auto a,auto b){ return a.first<b.first; }
		);
		if(f==data.end() || f->first!=k){
			return data.emplace(f,std::make_pair(k,V{}))->second;
		}
		return f->second;
	}

	auto size()const{
		return data.size();
	}

	auto empty()const{
		return data.empty();
	}

	void clear(){
		data.clear();
	}

	auto operator<=>(flat_map const&)const=default;
};

template<typename K,typename V>
std::map<K,V> to_map(flat_map<K,V> const& a){
	std::map<K,V> r;
	for(auto [k,v]:a){
		r[k]=v;
	}
	return r;
}

template<typename K,typename V>
std::ostream& operator<<(std::ostream& o,flat_map<K,V> const& a){
	//obviously not the most efficient way to do this.
	return o<<to_map(a);
}

template<typename K,typename V>
flat_map<K,V> to_flat_map(std::vector<std::pair<K,V>> &&a){
	return flat_map<K,V>{std::forward<std::vector<std::pair<K,V>>>(a)};
}

template<typename Func,typename K,typename V>
auto mapf(Func f,flat_map<K,V> const& a){
	using U=decltype(f(*std::begin(a)));
	std::vector<U> r(a.size());
	std::transform(a.begin(),a.end(),r.begin(),f);
	return r;
}

template<typename K,typename V>
std::vector<V> values(flat_map<K,V> const& a){
	return mapf([](auto const& x){ return x.second; },a);
}

template<typename K,typename V>
std::vector<K> keys(flat_map<K,V> const& a){
	return mapf([](auto const& x){ return x.first; },a);
}

template<typename Func,typename K,typename V>
auto map_values(Func f,flat_map<K,V> const& a){
	using U=decltype(f(a.begin()->second));
	flat_map<K,U> r;
	for(auto [k,v]:a){
		r[k]=f(v);
	}
	return r;
}

#endif
