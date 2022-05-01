#ifndef SET_H
#define SET_H

#include<set>

template<typename T>
std::ostream& operator<<(std::ostream& o,std::set<T> const& a){
	o<<"{ ";
	for(auto const& x:a){
		o<<x<<" ";
	}
	return o<<"}";
}

template<typename T>
std::multiset<T>& operator|=(std::multiset<T>& a,T t){
	a.insert(std::move(t));
	return a;
}

template<typename T>
std::multiset<T>& operator|=(std::multiset<T>& a,std::multiset<T> const& b){
	a.insert(b.begin(),b.end());
	return a;
}

template<typename T>
std::set<T>& operator|=(std::set<T>& a,T t){
	a.insert(std::move(t));
	return a;
}

template<typename T>
std::set<T> operator-(std::set<T> const& a,std::set<T> const& b){
	std::set<T> r;
	std::set_difference(
		a.begin(),a.end(),
		b.begin(),b.end(),
		std::inserter(r,r.begin())
	);
	return r;
}

template<typename T>
std::set<T> operator&(std::set<T> const& a,std::set<T> const& b){
	std::set<T> r;
	std::set_intersection(
		a.begin(),a.end(),
		b.begin(),b.end(),
		std::inserter(r,r.begin())
	);
	return r;
}

template<typename T>
std::set<T> to_set(std::vector<T> const& a){
	std::set<T> r;
	for(auto x:a) r|=x;
	return r;
}

template<typename K,typename V>
std::set<K> keys(std::map<K,V> const& a){
	return to_set(mapf([](auto x){ return x.first; },a));
}

template<typename T>
std::set<T>& operator|=(std::set<T>& a,std::optional<T> const& b){
	if(b) a|=*b;
	return a;
}

template<typename T>
std::set<T>& operator|=(std::set<T>& a,std::set<T> const& b){
	a.insert(b.begin(),b.end());
	return a;
}

template<typename T>
std::set<T> operator|(std::set<T> a,std::set<T> const& b){
	a|=b;
	return a;
}

template<typename T>
bool operator==(std::set<T> const& a,std::vector<T> const& b){
	return a==to_set(b);
}

template<typename T>
std::multiset<T> to_multiset(std::vector<T> const& v){
	return std::multiset<T>{v.begin(),v.end()};
}

template<typename T>
auto to_set(std::multiset<T> const& v){
	return std::set<T>{begin(v),end(v)};
}

template<typename Func,typename T>
auto mapf(Func f,std::multiset<T> const& a){
	std::vector<decltype(f(*a.begin()))> r;
	for(auto elem:a){
		r|=f(elem);
	}
	return r;
}

template<typename Func,typename T>
std::set<T> filter(Func f,std::set<T> const& a){
	std::set<T> r;
	for(auto elem:a){
		if(f(elem)){
			r|=elem;
		}
	}
	return r;
}

template<typename T>
std::multiset<T>& operator|=(std::multiset<T> &a,std::vector<T> const& b){
	a.insert(begin(b),end(b));
	return a;
}

template<typename T>
std::map<T,size_t> count(std::multiset<T> const& a){
	std::map<T,size_t> r;
	for(auto elem:a){
		r[elem]=a.count(elem); //slow
	}
	return r;
}

#endif
