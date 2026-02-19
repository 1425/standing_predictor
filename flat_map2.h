#ifndef FLAT_MAP2_H
#define FLAT_MAP2_H

#include "flat_map.h"
#include "io.h"

template<typename T>
std::vector<T> sorted(std::initializer_list<T> const& a){
	std::vector<T> v;
	for(auto elem:a){
		v|=elem;
	}
	return sorted(v);
}

template<typename K,typename V>
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

template<typename K,typename V>
std::ostream& operator<<(std::ostream& o,const_proxy<K,V> const& a){
	return o<<"("<<a.first<<","<<a.second<<")";
}

template<typename Func,typename K,typename V>
auto mapf(Func f,const_proxy<K,V> const& a){
	return std::make_pair(
		f(a.first),
		f(a.second)
	);
}

template<typename K,typename V>
V second(const_proxy<K,V> const& a){
	return a.second;
}

template<typename K,typename V>
class flat_map2{
	//Doing pair of vectors rather than vector of pairs
	//This will complicate some operations like find()
	//These are kept in order.
	using KS=std::vector<K>;
	using VS=std::vector<V>;
	KS keys;
	VS values;

	public:

	using key_type=K;
	using mapped_type=V;

	flat_map2(){}

	flat_map2(std::initializer_list<std::pair<K,V>> const& a){
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

		bool operator<=>(auto)const;
		//bool operator<(auto)const;

		auto operator<=>(std::pair<K,V> const& a)const{
			auto c=(first<=>a.first);
			if(c!=std::strong_ordering::equal){
				return std::partial_ordering(c);
			}
			return second<=>a.second;
		}

		bool operator<(std::pair<K,V> const& a)const{
			auto c=(*this)<=>a;
			return c==std::strong_ordering::less;
		}

		operator std::pair<K,V>()const{
			return std::make_pair(first,second);
		}

		operator std::pair<const K,V>()const{
			return std::make_pair(first,second);
		}

		friend std::ostream& operator<<(std::ostream& o,proxy const& a){
			return o<<"("<<a.first<<","<<a.second<<")";
		}
	};

	struct iterator{
		using iterator_category=typename KS::iterator::iterator_category;
		using difference_type=typename KS::iterator::difference_type;
		using value_type=std::pair<K,V>;
		//using pointer=value_type*;
		using reference=value_type&;//proxy;

		using K_it=typename KS::iterator;
		using V_it=typename VS::iterator;
		K_it first;
		V_it second;

		iterator(K_it a,V_it b):first(a),second(b){}

		/*iterator(iterator const& a):
			first(a.first),
			second(a.second)
		{}*/

		iterator& operator++(){
			first++;
			second++;
			return *this;
		}

		iterator operator-(int i)const{
			iterator r(*this);
			r.first-=i;
			r.second-=i;
			return r;
		}

		iterator& operator--(){
			first--;
			second--;
			return *this;
		}

		iterator& operator+=(long int x){
			first+=x;
			second+=x;
			return *this;
		}

		difference_type operator-(iterator a)const{
			auto r1=first-a.first;
			auto r2=second-a.second;
			assert(r1==r2);
			return r1;
		}

		proxy operator*(){
			return proxy{*first,*second};
		}

		std::shared_ptr<proxy> operator->(){
			return std::make_shared<proxy>(*first,*second);
		}

		auto operator<=>(iterator const&)const=default;
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

		const_proxy<K,V> operator*()const{
			return const_proxy<K,V>{*first,*second};
		}

		auto operator<=>(const_iterator const&)const=default;
	};

	const_iterator find(K const& k)const{
		auto f=std::lower_bound(keys.begin(),keys.end(),k);
		if(f==keys.end() || *f!=k){
			return const_iterator{keys.end(),values.end()};
		}
		return const_iterator{f,values.begin()+(f-keys.begin())};
	}

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

	iterator emplace_hint(iterator i,std::pair<K,V> p){
		while(i<end() && i->first<p.first){
			++i;
		}
		if(i==end()){
			keys|=p.first;
			values|=p.second;
			return end()-1;
		}
		
		if(i->first==p.first){
			i->second=p.second;
			return i;
		}

		auto i1=std::lower_bound(keys.begin(),keys.end(),p.first);
		auto i2=values.begin()+(i1-keys.begin());

		try{
			keys.insert(i1,p.first);
			values.insert(i2,p.second);
		}catch(...){
			//detect, but don't handle if either of these fails.  
			assert(0);
		}
		return iterator{i1,i2};
	}

	void reserve(size_t n){
		keys.reserve(n);
		values.reserve(n);
	}

	std::map<K,V> to_map()const{
		std::map<K,V> r;
		for(auto i:range(keys.size())){
			r[keys[i]]=values[i];
		}
		return r;
	}

	/*operator std::map<K,V>()const{
		//for perf, never want to use this.
		std::map<K,V> r;
		for(auto i:range(keys.size())){
			r[keys[i]]=values[i];
		}
		return r;
	}*/

	auto const& get_values()const{
		return values;
	}

	auto size()const{
		return keys.size();
	}

	void clear(){
		keys.clear();
		values.clear();
	}

	auto empty()const{
		return keys.empty();
	}

	auto const& get_keys()const{
		return keys;
	}

	template<typename Func>
	auto map_values(Func f)const{
		using E=decltype(f(values[0]));
		flat_map2<K,E> r;
		r.keys=keys;
		r.values=mapf(f,values);
		return r;
	}

	auto operator<=>(flat_map2 const&)const=default;
};

template<typename K,typename V>
std::ostream& operator<<(std::ostream& o,flat_map2<K,V> const& a){
	return o<<a.to_map();
}

template<typename K,typename V>
std::map<K,V> to_map(flat_map2<K,V> const& a){
	return a.to_map();
}

template<typename K,typename V>
auto const& values(flat_map2<K,V> const& a){
	return a.get_values();
}

template<typename K,typename V>
auto keys(flat_map2<K,V> const& a){
	return a.get_keys();
}

template<typename Func,typename K,typename V>
auto mapf(Func f,flat_map2<K,V> const& a){
	using U=decltype(f(*std::begin(a)));
	std::vector<U> r(a.size());
	std::transform(a.begin(),a.end(),r.begin(),f);
	return r;
}

template<typename K,typename V>
auto to_vec(flat_map2<K,V> const& a){
	using P=std::pair<K,V>;
	std::vector<P> r;
	for(auto x:a){
		r|=P(x.first,x.second);
	}
	return r;
}

template<typename K,typename V>
auto sorted(flat_map2<K,V> const& a){
	return to_vec(a); //because the items are already in sorted order
}

template<typename K,typename V>
V get(flat_map2<K,V> const& a,auto const& k,auto const& otherwise){
	auto f=a.find(k);
	if(f==a.end()){
		return otherwise;
	}
	return (*f).second;
}

template<typename K,typename V>
V get(flat_map2<K,V> const& a,auto const& k){
	auto f=a.find(k);
	assert(f!=a.end());
	return (*f).second;
}

#endif
