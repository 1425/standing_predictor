#ifndef SET_FLAT_H
#define SET_FLAT_H

#include "io.h"
#include<map>
#include "set_limited.h"

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

	explicit set_flat(std::vector<T> const& a){
		for(auto const& x:a){
			(*this)|=x;
		}
	}
	
	set_flat& operator=(set_flat const&)=default;

	set_flat& operator=(std::set<T> const& a){
		data=to_vec(a);
		return *this;
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

	set_flat& operator-=(T const& t){
		erase(t);
		return *this;
	}

	template<typename V>
	static auto keys(std::map<T,V> const& a){
		set_flat r;
		r.data.reserve(a.size());
		for(auto const& [k,v]:a){
			r.data|=k;
		}
		return r;
	}

	bool operator!=(std::set<T>)const;

	auto operator<=>(set_flat const&)const=default;

	bool operator==(set_flat a)const{
		auto c=(*this<=>a);
		return c==std::strong_ordering::equal;
	}

	bool operator==(std::set<T> const& a)const{
		//obviously not the fastest way to do this.
		return to_vec(*this)==to_vec(a);
	}

	size_t size()const{
		return data.size();
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

template<typename K,typename V>
auto keys(std::map<K,V> const& a){
	return set_flat<K>::keys(a);
}

template<typename T>
std::set<T> operator-(std::set<T> a,set_flat<T> const& b){
	for(auto const& x:b){
		a-=x;
	}
	return a;
}

template<typename T>
set_flat<T> operator-(set_flat<T> a,set_flat<T> const& b){
	//not a fast way to do this.
	for(auto const& x:b){
		a-=x;
	}
	return a;
}

template<typename T>
set_flat<T> operator-(set_flat<T> a,std::set<T> b){
	for(auto const& x:b){
		a.erase(x);
	}
	return a;
}

template<typename T>
set_flat<T> to_set(set_flat<T> a){
	return a;
}

template<typename T>
bool operator!=(std::set<T> const& a,set_flat<T> const& b){
	return !(b==a);
}

template<typename T>
void diff(std::set<T> const& a,set_flat<T> const& b){
	auto a_only=a-b;
	auto b_only=b-a;
	if(a_only.size()){
		PRINT(a_only);
	}
	if(b_only.size()){
		PRINT(b_only);
	}
}

template<typename T>
bool operator==(std::set<T> const& a,set_flat<T> const& b){
	return b==a;
}

template<typename T>
set_flat<T> operator|(set_flat<T> a,std::vector<T> const& b){
	for(auto const& x:b){
		a|=x;
	}
	return a;
}

template<typename T>
bool operator==(std::set<std::optional<T>> const& a,set_flat<T> const& b){
	for(auto x:a){
		if(!x){
			return 0;
		}
		if(!b.count(*x)){
			return 0;
		}
	}
	for(auto x:b){
		if(!a.count(x)){
			return 0;
		}
	}
	return 1;
}

template<typename T>
std::multiset<T>& operator|=(std::multiset<T>& a,set_flat<T> const& b){
	a.insert_range(b);
	return a;
}

template<typename T,size_t N>
set_limited<T,N> operator-(set_limited<T,N> a,set_flat<T> const& b){
	return filter([&](auto const& x){ return !b.count(x); },a);
}

template<typename Func,typename T>
set_flat<T> filter(Func f,set_flat<T> a){
	return set_flat<T>{filter(f,a.get())};
}

template<typename T>
std::vector<T> operator-(std::vector<T> a,set_flat<T> const& b){
	return filter([&](auto const& x){ return !b.count(x); },a);
}

template<typename T>
set_flat<T> operator&(set_flat<T>,set_flat<T>);

template<typename K,typename V>
std::map<K,V> remove_keys(std::map<K,V> a,set_flat<K> const& b){
	for(auto const& x:b){
		a.erase(x);
	}
	return a;
}

template<typename Func,typename T>
auto group(Func f,set_flat<T> const& a){
	using K=decltype(f(*std::begin(a)));
	std::map<K,set_flat<T>> r;
	nyi
	return r;
}

template<typename T>
T choose(set_flat<T>);

template<typename T>
set_flat<T> choose(size_t,set_flat<T>);

template<typename T,size_t N>
set_flat<T> operator-(set_flat<T>,set_limited<T,N>);

template<typename T>
bool subset(set_flat<T>,std::set<T>);

template<typename T>
bool subset(set_flat<T> const& a,set_flat<T> const& b){
	auto ai=a.begin();
	auto ae=a.end();
	auto bi=b.begin();
	auto be=b.end();

	//this is O(N+M)
	while(ai!=ae && bi!=be){
		auto av=*ai;
		auto bv=*bi;

		if(av<bv){
			return 0;
		}

		if(av==bv){
			ai++;
			bi++;
		}else{
			while(bi!=be && *bi<av){
				++bi;
			}
		}
	}

	return ai==ae;

	//this is O(N*log(M))
	/*for(auto const& x:a){
		if(!b.count(x)){
			return 0;
		}
	}
	return 1;*/
}

template<typename T>
auto to_std_set(set_flat<T> const& a){
	return std::set<T>{a.begin(),a.end()};
}

#endif
