#ifndef MAP_SMALL_INT_H
#define MAP_SMALL_INT_H

#include<vector>

template<typename K,typename V>
class map_small_int{
	using P=std::pair<K,V>;

	//this is obviously not the most space-efficient layout, but it does mean
	//that the pairs can be directly returned and are on a regular stride.
	using Data=std::vector<std::optional<P>>;

	Data data;

	public:

	map_small_int()=default;

	map_small_int(std::vector<P>::const_iterator a,std::vector<P>::const_iterator b){
		//this is not the fastest way to do this.
		for(auto it=a;it!=b;++it){
			(*this)[it->first]=it->second;
		}
	}

	using I1=std::vector<P>::const_iterator;
	using It=std::move_iterator<I1>;
	//template<typename It>
	map_small_int(It a,It b){
		for(auto it=a;it!=b;++it){
			(*this)[std::move(it->first)]=std::move(it->second);
		}
	}

	#if 0
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

		//auto operator<=>(const_iterator const&)const=default;

		auto operator<=>(const_iterator const& a)const{
			return i<=>a.i;
		}

		bool operator!=(const_iterator const& a)const{
			return i!=a.i;
		}
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
	#else
	struct const_iterator{
		using It=Data::const_iterator;
		It at,end;

		public:
		const_iterator& operator++(){
			assert(at!=end);
			++at;
			while(at!=end && !*at){
				++at;
			}
			return *this;
		}

		P const& operator*()const{
			return **at;
		}

		//auto operator<=>(const_iterator const&)const=default;
		auto operator<=>(const_iterator const& a)const{
			return at<=>a.at;
		}

		auto operator!=(const_iterator const& a)const{
			return at!=a.at;
		}
	};

	const_iterator begin()const{
		return const_iterator{data.begin(),data.end()};
	}

	const_iterator end()const{
		return const_iterator{data.end(),data.end()};
	}
	#endif

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
