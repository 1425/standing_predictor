#ifndef MAP_SMALL_INT_H
#define MAP_SMALL_INT_H

#include<vector>

template<typename K,typename V>
class map_small_int{
	using P=std::pair<K,V>;
	using Data=std::vector<std::optional<P>>;
	Data data;

	public:

	struct const_iterator{
		map_small_int const *parent;
		size_t i;

		public:
		const_iterator& operator++(){
			assert(parent);

			auto &d=parent->data;
			assert(i<d.size());

			i++;
			while(i<d.size() && !d[i]){
				i++;
			}
			return *this;
		}

		P const& operator*()const{
			assert(parent);
			auto const& d=parent->data;
			assert(d.size()>i);
			auto const& v=d[i];
			assert(v);
			return *v;
		}

		auto operator<=>(const_iterator const&)const=default;
	};

	const_iterator begin()const{
		size_t i=0;
		while(i<data.size() && !data[i]){
			i++;
		}
		return const_iterator{this,i};
	}

	const_iterator end()const{
		return const_iterator{this,data.size()};
	}

	using iterator=const_iterator;

	V& operator[](K const& k){
		while(data.size()<=k){
			data|=std::nullopt;
		}
		if(!data[k]){
			data[k]=P(k,V());
		}
		return data[k]->second;
	}

	size_t size()const{
		size_t r=0;
		for(auto const& x:data){
			if(x){
				r++;
			}
		}
		return r;
	}
};

template<typename K,typename V>
std::ostream& operator<<(std::ostream& o,map_small_int<K,V> const& a){
	o<<"{ ";
	for(auto const& x:a){
		o<<x<<" ";
	}
	return o<<")";
}

template<typename K,typename V>
std::set<K> keys(map_small_int<K,V> const& a){
	std::set<K> r;
	for(auto const& p:a){
		r|=p.first;
	}
	return r;
}

template<typename Func,typename K,typename V>
auto mapf(Func f,map_small_int<K,V> const& a){
	using E=decltype(f(*std::begin(a)));
	std::vector<E> r;
	for(auto x:a){
		r|=f(x);
	}
	return r;
}

template<typename Func,typename K,typename V>
auto map_values(Func f,map_small_int<K,V> const& a){
	using E=decltype(f(*(V*)0));
	map_small_int<K,E> r;
	for(auto [k,v]:a){
		r[k]=f(v);
	}
	return r;
}

template<typename K,typename V>
auto values(map_small_int<K,V> const& a){
	return mapf([](auto x){ return x.second; },a);
}

#endif
