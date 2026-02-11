#ifndef MAP_FIXED_H
#define MAP_FIXED_H

template<long long MIN,long long MAX1>
struct Converter<Int_limited<MIN,MAX1>>{
	using T=Int_limited<MIN,MAX1>;

	static constexpr auto MAX=MAX1;

	static constexpr auto from(T t){
		return t.get();
	}

	static constexpr T to(auto x){
		return x;
	}
};

template<typename K,typename V>
class map_fixed{
	using C=Converter<K>;
	static constexpr auto N=C::MAX+1;
	std::array<V,N> data;
	std::bitset<N> present;

	public:

	struct const_iterator{
		map_fixed const *parent;
		size_t i;

		const_iterator& operator++(){
			assert(parent);
			i++;
			while(i<N && !parent->present[i]){
				i++;
			}
			return *this;
		}

		auto operator*()const{
			return std::make_pair(C::to(i),parent->data[i]);
		}

		auto operator<=>(const_iterator const&)const=default;
	};

	const_iterator begin()const{
		size_t i=0;
		while(i<N && !present[i]){
			i++;
		}
		return const_iterator{this,i};
	}

	const_iterator end()const{
		return const_iterator{this,N};
	}

	V& operator[](K const& k){
		if(!present[k]){
			present[k]=1;
		}
		return data[k];
	}

	size_t size()const{
		return present.count();
	}
};

template<typename Func,typename K,typename V>
auto mapf(Func f,map_fixed<K,V> const& a){
	using E=decltype(f(*a.begin()));
	std::vector<E> r;
	for(auto const& x:a){
		r|=f(x);
	}
	return r;
}

template<typename K,typename V>
auto values(map_fixed<K,V> const& a){
	return mapf([](auto x){ return x.second; },a);
}

template<typename K,typename V>
std::set<K> keys(map_fixed<K,V> const& a){
	std::set<K> r;
	for(auto [k,v]:a){
		r|=k;
	}
	return r;
}

template<typename K,typename V>
std::ostream& operator<<(std::ostream& o,map_fixed<K,V> const& a){
	o<<"{ ";
	for(auto const& x:a){
		o<<x<<" ";
	}
	return o<<"}";
}

template<typename Func,typename K,typename V>
auto map_values(Func f,map_fixed<K,V> const& a){
	using E=decltype(f((*a.begin()).second));
	map_fixed<K,E> r;
	for(auto [k,v]:a){
		r[k]=f(v);
	}
	return r;
}

#endif
