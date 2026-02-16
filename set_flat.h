#ifndef SET_FLAT_H
#define SET_FLAT_H

#include "io.h"

template<typename T>
class set_flat{
	//this is kept sorted.
	using Data=std::vector<T>;
	Data data;

	public:

	set_flat()=default;

	constexpr set_flat(std::initializer_list<T> a){
		for(auto x:a){
			(*this)|=x;
		}
	}

	auto const& get()const{
		return data;
	}

	using const_iterator=Data::const_iterator;

	const_iterator begin()const{
		return data.begin();
	}

	constexpr const_iterator end()const{
		return data.end();
	}

	bool empty()const{
		return data.empty();
	}

	bool count(T const& t)const{
		auto f=std::lower_bound(data.begin(),data.end(),t);
		return !(f==data.end() || *f!=t);

		/*for(auto const& elem:data){
			if(elem==t){
				return 1;
			}
		}
		return 0;*/
	}

	constexpr set_flat& operator|=(T const& t){
		auto f=std::lower_bound(data.begin(),data.end(),t);
		if(f==end() || *f!=t){
			data.insert(f,t);
		}
		/*if(!count(t)){
			data|=t;
			std::sort(data.begin(),data.end());
		}*/
		return *this;
	}

	set_flat operator|(set_flat const& a)const{
		set_flat r(*this);
		for(auto x:a){
			r|=x;
		}
		return r;
	}

	void erase(T const& t){
		auto f=std::lower_bound(data.begin(),data.end(),t);
		if(f==data.end() || *f!=t){
			return;
		}
		//auto f=std::find(data.begin(),data.end(),t);
		//assert(f!=data.end());
		data.erase(f);
	}
};

template<typename T>
std::ostream& operator<<(std::ostream& o,set_flat<T> const& a){
	return o<<a.get();
}

template<typename T>
auto to_vec(set_flat<T> const& a){
	return a.get();
}

#endif
