#ifndef FLAT_MAP2_H
#define FLAT_MAP2_H

template<typename T>
std::vector<T> sorted(std::initializer_list<T> const& a){
	std::vector<T> v;
	for(auto elem:a){
		v|=elem;
	}
	return sorted(v);
}

template<typename K,typename V>
class flat_map2{
	//Doing pair of vectors rather than vector of pairs
	//This will complicate some operations like find()
	using KS=std::vector<K>;
	using VS=std::vector<V>;
	KS keys;
	VS values;

	public:
	flat_map2(){}

	flat_map2(std::initializer_list<std::pair<K,V>> const& a){
		//sort(a.begin(),a.end());
		for(auto const& elem:sorted(a)){
			keys|=elem.first;
			values|=elem.second;
		}
	}

	explicit flat_map2(std::vector<std::pair<K,V>> const& a){
		//assuming that there are no duplicate keys in input
		for(auto [k,v]:sorted(a)){
			keys|=k;
			values|=v;
		}
	}

	explicit flat_map2(flat_map<K,V> const& a){
		for(auto [k,v]:a){
			keys|=k;
			values|=v;
		}
	}

	explicit flat_map2(std::map<K,V> const& a){
		for(auto [k,v]:a){
			keys|=k;
			values|=v;
		}
	}

	struct proxy{
		K& first;
		V& second;

		//auto operator<=>(proxy const&)const=default;
		bool operator==(proxy const&)const{
			nyi
		}
	};

	struct iterator{
		using K_it=typename KS::iterator;
		using V_it=typename VS::iterator;
		K_it first;
		V_it second;

		proxy *p=nullptr;

		iterator(K_it a,V_it b):first(a),second(b){}

		iterator(iterator const&);

		~iterator(){
			delete p;
		}

		iterator& operator++(){
			first++;
			second++;
			return *this;
		}

		proxy operator*(){
			return proxy{*first,*second};
		}

		proxy* operator->(){
			if(p){
				delete p;
			}
			p=new proxy{*first,*second};
			return &*p;
		}

		auto operator<=>(iterator const&)const=default;
	};

	struct const_proxy{
		K const& first;
		V const& second;

		auto operator<=>(const_proxy const&)const{
			nyi
		}

		bool operator==(const_proxy const& p)const{
			return first==p.first && second==p.second;
		}
	};

	struct const_iterator{
		using K_it=typename KS::const_iterator;
		using V_it=typename VS::const_iterator;
		K_it first;
		V_it second;

		const_iterator& operator++(){
			first++;
			second++;
			return *this;
		}

		const_proxy operator*(){
			return const_proxy{*first,*second};
		}

		auto operator<=>(const_iterator const&)const=default;
	};

	const_iterator find(K)const;

	iterator find(K const& k){
		auto f=std::lower_bound(keys.begin(),keys.end(),k);
		if(f==keys.end() || *f!=k){
			return iterator{keys.end(),values.end()};
		}
		return iterator{f,values.begin()+(f-keys.begin())};
	}

	const_iterator begin()const{
		return const_iterator{keys.begin(),values.begin()};
	}

	const_iterator end()const{
		return const_iterator{keys.end(),values.end()};
	}

	iterator begin(){
		return iterator{keys.begin(),values.begin()};
	}

	iterator end(){
		return iterator{keys.end(),values.end()};
	}

	V& operator[](K const& k){
		auto f=std::lower_bound(keys.begin(),keys.end(),k);
		auto v_it=values.begin()+(f-keys.begin());
		if(f==keys.end() || *f!=k){
			//new key
			keys.emplace(f,k);
			return *values.emplace(v_it,V{});
		}
		return *v_it;
	}

	/*operator map<K,V>()const{
		//for perf, never want to use this.
		map<K,V> r;
		for(auto i:range(keys.size())){
			r[keys[i]]=values[i];
		}
		return r;
	}*/

	auto get_values()const{
		return values;
	}
};

template<typename K,typename V>
auto values(flat_map2<K,V> const& a){
	return a.get_values();
}

#endif
