#ifndef SET_FIXED_H
#define SET_FIXED_H

#include "set.h"
#include "array.h"
#include "util.h"
#include "io.h"

template<typename T,size_t N>
class set_fixed{
	using Data=std::array<T,N>;
       	Data data;

	static Data convert(std::set<T> const& a){
		assert(a.size()==N);
		auto v=to_vec(a);
		return mapf([=](auto i){ return v[i]; },range_st<N>());
	}

	public:
	set_fixed(Data);

	set_fixed(std::set<T> a):
		data(convert(a))
	{
	}

	set_fixed(std::vector<T> const& a):
		set_fixed(to_set(a))
	{}

	constexpr set_fixed(std::initializer_list<T> a){
		assert(a.size()==N);
		/*for(size_t i=0;i<a.size();i++){
			data[i]=a[i];
		}*/
		size_t i=0;
		for(auto x:a){
			data[i++]=x;
		}
	}

	static constexpr auto size(){
		return N;
	}

	using const_iterator=Data::const_iterator;

	const_iterator begin()const{
		return data.begin();
	}

	const_iterator end()const{
		return data.end();
	}

	auto get()const{
		return data;
	}

	bool count(auto const& t)const{
		//since data is kept sorted, could do a binary search.
		for(auto const& elem:data){
			if(elem==t){
				return 1;
			}
		}
		return 0;
	}
};

template<typename T,size_t N>
std::ostream& operator<<(std::ostream& o,set_fixed<T,N> const& a){
	return o<<a.get();
}

template<typename T,size_t N>
set_fixed<T,N> rand(set_fixed<T,N> const*){
	std::set<T> s;
	while(s.size()<N){
		s|=rand((T*)0);
	}
	return s;
}

template<typename T,size_t N>
std::set<T> to_set(set_fixed<T,N> a){
	return std::set<T>{a.begin(),a.end()};
}

template<typename T,size_t N,size_t M>
std::vector<T> flatten(std::array<set_fixed<T,N>,M> const& a){
	//could return something of fixed size.
	std::vector<T> r;
	for(auto const& x:a){
		for(auto const& elem:x){
			r|=elem;
		}
	}
	return r;
}

template<typename T,size_t N>
bool contains(set_fixed<T,N> const& a,T b){
	for(auto x:a){
		if(x==b){
			return 1;
		}
	}
	return 0;
}

template<typename T,size_t N>
std::set<T> operator-(std::set<T> a,set_fixed<T,N> b){
	for(auto x:b){
		a-=x;
	}
	return a;
}

#endif
