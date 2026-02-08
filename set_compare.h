#ifndef SET_COMPARE_H
#define SET_COMPARE_H

template<typename T>
struct set_compare{
	using B=std::set<T>;
	using A=set_flat<T>;
	std::pair<A,B> data;

	void check(){
		//assert(data.first.size()==data.second.size());
		assert(to_vec(data.first)==to_vec(data.second));
	}

	public:

	bool empty()const{
		auto m=mapf([](auto const& x){ return x.empty(); },data);
		assert(all_equal(m));
		return get<0>(m);
	}

	using const_iterator=A::const_iterator;

	const_iterator begin()const{
		return data.first.begin();
	}

	const_iterator end()const{
		return data.first.end();
	}

	set_compare& operator|=(T const& t){
		check();
		data.first|=t;
		data.second|=t;
		check();
		return *this;
	}

	set_compare operator|(set_compare const& b)const{
		set_compare r(*this);
		for(auto x:b){
			r|=x;
		}
		return r;
	}

	bool count(T const& t)const{
		auto m=mapf([&](auto const& x){ return x.count(t); },data);
		assert(all_equal(m));
		return get<0>(m);
	}

	void erase(T const& t){
		check();
		data.first.erase(t);
		data.second.erase(t);
		check();
	}
};

template<typename T>
std::ostream& operator<<(std::ostream& o,set_compare<T> const& a){
	return o<<a.data.first;
}

#endif
