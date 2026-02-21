#ifndef MULTISET_FLAT_H
#define MULTISET_FLAT_H

#include "flat_map.h"

template<typename T>
class multiset_flat{
	using Data=flat_map<T,unsigned>;
	Data data;

	public:

	multiset_flat()=default;

	multiset_flat(std::vector<T> const& a){
		for(auto elem:a){
			*this|=elem;
		}
	}

	explicit multiset_flat(Data a):
		data(a)
	{}

	multiset_flat& operator|=(T const& t){
		data[t]++;
		return *this;
	}

	multiset_flat& operator|=(multiset_flat const& a){
		for(auto [k,v]:a.data){
			data[k]+=v;
		}
		return *this;
	}

	auto const& get()const{
		return data;
	}

	size_t size()const{
		return sum(values(data));
	}

	bool empty()const{
		return size()==0;
	}

	size_t count(T const& t)const{
		auto f=data.find(t);
		if(f==data.end()){
			return 0;
		}
		return f->second;
	}

	auto set()const{
		return keys(data);
	}

	struct const_iterator{
		Data::const_iterator at,end;
		unsigned i;

		T operator*()const{
			assert(at!=end);
			assert(i<(at->second));
			return at->first;
		}

		const_iterator& operator++(){
			assert(at!=end);
			assert(i<(at->second));

			i++;
			if(i>=at->second){
				++at;
				while(at!=end && at->second==0){
					++at;
				}
				i=0;
			}
			return *this;
		}

		const_iterator operator-(int)const;

		auto operator<=>(const_iterator const&)const=default;
	};

	const_iterator begin()const{
		auto r=const_iterator{data.begin(),data.end(),0};
		while(r.at!=r.end && r.at->second==0){
			++r.at;
		}
		return r;
	}

	const_iterator end()const{
		return const_iterator{data.end(),data.end(),0};
	}

	multiset_flat& operator-=(T t){
		auto f=data.find(t);
		assert(f!=data.end());
		auto& v=f->second;
		assert(v);
		v--;
		return *this;
	}

	multiset_flat operator-(T t)const{
		auto r=*this;
		r-=t;
		return r;
	}

	template<typename Func>
	auto map(Func f)const{
		using E=decltype(f(*(T*)0));
		flat_map<E,unsigned> r;
		for(auto [k,v]:data){
			//this conditional is not technically required
			//but should both improve perf and readability of output
			if(v){
				r[f(k)]+=v;
			}
		}
		return multiset_flat<E>(r);
	}

	template<typename Func>
	auto filter(Func f)const{
		multiset_flat r;
		for(auto [k,v]:data){
			if(v && f(k)){
				r.data[k]=v;
			}
		}
		return r;
	}

	auto operator<=>(multiset_flat const& a)const{
		//don't do the default version, because then when there are keys with a value of 0
		//they are included and may make things appear to be different.

		auto i1=data.begin();
		auto i2=a.data.begin();

		auto e1=data.end();
		auto e2=a.data.end();

		while(i1!=e1 && !i1->second){
			++i1;
		}

		while(i2!=e2 && !i2->second){
			++i2;
		}

		while(i1!=e1 && i2!=e2){
			{
				auto k1=i1->first;
				auto k2=i2->first;

				auto c=(k1<=>k2);
				if(c!=std::strong_ordering::equal){
					return c;
				}
			}

			{
				auto c=i1->second <=> i2->second;
				if(c!=std::strong_ordering::equal){
					return c;
				}
			}

			i1++;
			while(i1!=e1 && !i1->second){
				i1++;
			}

			i2++;
			while(i2!=e2 && !i2->second){
				i2++;
			}
		}

		auto done1=(i1==e1);
		auto done2=(i2==e2);
		return done1<=>done2;
	}

	bool operator==(multiset_flat const& a)const{
		auto c=(*this<=>a);
		return c==std::strong_ordering::equal;
	}

	bool operator!=(multiset_flat const& a)const{
		return !(*this==a);
	}
};

template<typename Func,typename T>
auto mapf(Func f,multiset_flat<T> const& a){
	return a.map(f);
}

template<typename Func,typename T>
auto filter(Func f,multiset_flat<T> const& a){
	return a.filter(f);
}

template<typename T>
std::ostream& operator<<(std::ostream& o,multiset_flat<T> const& a){
	return o<<a.get();
}

template<typename T>
auto to_set(multiset_flat<T> const& a){
	return a.set();
}

template<typename T>
auto count(multiset_flat<T> const& a){
	return a.get();
}

template<typename T>
auto min(multiset_flat<T> const& a){
	for(auto const& [k,v]:a.get()){
		if(!v) continue;
		return k;
	}
	assert(0);
}

template<typename T>
auto max(multiset_flat<T> const& a){
	auto const& x=a.get();
	for(auto it=x.rbegin();it!=x.rend();++it){
		if(it->second){
			return it->first;
		}
	}
	assert(0);
}

template<typename T>
T sum(multiset_flat<T> const& a){
	T r{};
	for(auto [k,v]:a.get()){
		r+=k*v;
	}
	return r;
}

template<typename T>
T mean(multiset_flat<T> const& a){
	return sum(a)/a.size();
}

template<typename T>
auto to_vec(multiset_flat<T> const& a){
	std::vector<T> r;
	for(auto [k,v]:a.get()){
		for(auto _:range(v)){
			(void)_;
			r|=k;
		}
	}
	return r;
}

template<typename T>
T median(multiset_flat<T> const& a){
	//obviously not the most efficient way to do this.
	return median(to_vec(a));
}

template<typename T>
auto to_multiset_flat(std::vector<T> a){
	return multiset_flat<T>(a);
}

template<typename A,typename B>
multiset_flat<B> seconds(multiset_flat<std::pair<A,B>> const& a){
	return MAP(second,a);
}

template<typename A,typename B>
multiset_flat<A> firsts(multiset_flat<std::pair<A,B>> const& a){
	return MAP(first,a);
}

#endif
