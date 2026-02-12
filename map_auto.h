#ifndef MAP_AUTO_H
#define MAP_AUTO_H

#include "map_small_int.h"

template<typename T>
static constexpr bool small_int(T const*){
	return 0;
}

template<long long MIN,long long MAX>
static constexpr bool small_int(Int_limited<MIN,MAX> const*){
	return MIN>=0 && MAX<=255;
}

template<typename K,typename V>
class map_auto{
	using Data=typename std::conditional<
		small_int((K*)0),
		map_small_int<K,V>,
		//map_fixed<K,V>,
		std::map<K,V>
	>::type;
	//using Data=std::map<K,V>;
	Data data;

	public:

	map_auto()=default;

	map_auto(Data const& a):data(a){}

	map_auto& operator=(std::map<K,V>);

	V& operator[](K const& a){
		return data[a];
	}

	V const& operator[](K const& a)const{
		//in case whatever the underlying thing is allows this.
		//return data[a];
		auto f=data.find(a);
		if(f==data.end()){
			throw "not found";
		}
		return f->second;
	}

	using const_iterator=typename Data::const_iterator;

	const_iterator begin()const{
		return data.begin();
	}

	const_iterator end()const{
		return data.end();
	}

	const_iterator find(K const& k)const{
		return data.find(k);
	}

	using iterator=typename Data::iterator;

	iterator begin(){
		return data.begin();
	}

	iterator end(){
		return data.end();
	}

	iterator find(K const& k){
		return data.find(k);
	}

	Data const& get()const{
		return data;
	}

	constexpr size_t size()const{
		return data.size();
	}

	constexpr auto empty()const{
		return data.empty();
	}
};

template<typename K,typename V>
std::ostream& operator<<(std::ostream& o,map_auto<K,V> const& a){
	return o<<a.get();
}

template<typename K,typename V>
std::set<K> keys(map_auto<K,V> const& a){
	return keys(a.get());
}

template<typename K,typename V>
auto values(map_auto<K,V> const& a){
	return values(a.get());
}

template<typename Func,typename K,typename V>
auto mapf(Func f,map_auto<K,V> const& a){
	return mapf(f,a.get());
}

template<typename Func,typename K,typename V>
auto map_values(Func f,map_auto<K,V> const& a){
	return map_values(f,a.get());
}

template<typename K,typename V>
auto dict_auto(std::vector<std::pair<K,V>> const& a){
	map_auto<K,V> r;
	for(auto [k,v]:a){
		r[k]=v;
	}
	return r;
}

template<typename K,typename V>
auto to_map_auto(std::map<K,V> const& a){
	map_auto<K,V> r;
	for(auto [k,v]:a){
		r[k]=v;
	}
	return r;
}

template<typename K,typename V>
auto print_r(int n,map_auto<K,V> const& a){
	return print_r(n,a.get());
}

#endif
