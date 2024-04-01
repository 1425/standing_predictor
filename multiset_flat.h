#ifndef MULTISET_FLAT_H
#define MULTISET_FLAT_H

#include "flat_map.h"

template<typename T>
class multiset_flat{
	using Data=flat_map<T,unsigned>;
	Data data;

	public:
	multiset_flat()=default;

	multiset_flat(std::vector<T> const& a){
		for(auto elem:a){
			*this|=elem;
		}
	}

	multiset_flat& operator|=(T const& t){
		data[t]++;
		return *this;
	}

	auto const& get()const{
		return data;
	}

	size_t size()const{
		return sum(values(data));
	}
};

template<typename T>
std::ostream& operator<<(std::ostream& o,multiset_flat<T> const& a){
	return o<<a.get();
}

template<typename T>
auto count(multiset_flat<T> const& a){
	return a.get();
}

template<typename T>
auto min(multiset_flat<T> const& a){
	auto x=a.get();
	assert(!x.empty());
	return x.begin()->first;
}

template<typename T>
auto max(multiset_flat<T> const& a){
	auto x=a.get();
	assert(!x.empty());
	return (x.end()-1)->first;
}

template<typename T>
T sum(multiset_flat<T> const& a){
	T r{};
	for(auto [k,v]:a.get()){
		r+=k*v;
	}
	return r;
}

template<typename T>
T mean(multiset_flat<T> const& a){
	return sum(a)/a.size();
}

template<typename T>
auto to_vec(multiset_flat<T> const& a){
	std::vector<T> r;
	for(auto [k,v]:a.get()){
		for(auto _:range(v)){
			(void)_;
			r|=k;
		}
	}
	return r;
}

template<typename T>
T median(multiset_flat<T> const& a){
	//obviously not the most efficient way to do this.
	return median(to_vec(a));
}

#endif
