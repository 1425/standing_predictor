#ifndef SET_LIMITED_H
#define SET_LIMITED_H

#include "vector_fixed.h"

template<typename T,size_t N>
class set_limited{
	//this is not kept sorted.
	//so all the lookups are linear.
	using Data=std::array<T,N>;
	alignas(Data) char buf[sizeof(Data)];
	size_t size_=0;

	Data& data(){
		return *(Data*)buf;
	}

	Data const& data()const{
		return *(Data*)buf;
	}

	public:
	set_limited()=default;

	set_limited(set_limited const& a){
		size_=a.size_;
		size_t i=0;
		for(auto elem:a){
			new(&data()[i]) T(elem);
			i++;
		}
	}

	set_limited(set_limited && a){
		size_=a.size_;
		for(size_t i=0;i<size_;i++){
			new(&data()[i]) T(std::move(a.data()[i]));
		}
		a.size_=0;
	}

	set_limited& operator=(set_limited&& a){
		if(size()<a.size()){
			size_t i=0;
			for(;i<size();i++){
				data()[i]=std::move(a.data()[i]);
			}
			for(;i<a.size();i++){
				new(&data()[i]) T(std::move(a.data()[i]));
			}
			size_=a.size_;
			a.size_=0;
		}else{
			size_t i=0;
			for(;i<a.size();i++){
				data()[i]=a.data()[i];
			}
			for(;i<size();i++){
				data()[i].~T();
			}
			size_=a.size_;
			a.size_=0;
		}
		return *this;
	}

	set_limited& operator=(set_limited const& a){
		if(size()<a.size()){
			size_t i=0;
			for(;i<size();i++){
				data()[i]=a.data()[i];
			}
			for(;i<a.size();i++){
				new(&data()[i]) T(a.data()[i]);
			}
			size_=a.size();
		}else{
			size_t i=0;
			for(;i<a.size();i++){
				data()[i]=a.data()[i];
			}
			for(auto j=i;j<size();j++){
				data()[i].~T();
			}
			size_=a.size();
		}
		return *this;
	}

	set_limited(std::set<T> a){
		if(a.size()>N){
			print_r(a);
		}
		assert(a.size()<=N);
		size_=a.size();
		size_t i=0;
		for(auto elem:a){
			new(&data()[i]) T(elem);
			i++;
		}
	}

	set_limited(std::vector<T>&& a){
		for(auto &elem:a){
			(*this)|=std::move(elem);
		}
	}

	set_limited(std::vector<T> const& a){
		for(auto const& elem:a){
			(*this)|=elem;
		}
	}

	set_limited(vector_fixed<T,N> const& a){
		for(auto const& x:a){
			(*this)|=x;
		}
	}

	set_limited(vector_fixed<T,N> && a){
		for(auto &x:a){
			(*this)|=std::move(x);
		}
	}

	~set_limited(){
		//for(auto i:range(size_)){
		for(size_t i=0;i<size_;i++){
			data()[i].~T();
		}
	}

	auto size()const{
		return size_;
	}

	using const_iterator=Data::const_iterator;

	const_iterator begin()const{
		return data().begin();
	}

	const_iterator end()const{
		return data().begin()+size_;
	}

	bool count(T const& t)const{
		for(auto const& elem:*this){
			if(elem==t){
				return 1;
			}
		}
		return 0;
	}

	bool empty()const{
		return size()==0;
	}

	set_limited& operator|=(T const& t){
		if(!count(t)){
			assert(size_<N);
			new(&data()[size_]) T(t);
			size_++;
		}
		return *this;
	}

	set_limited& operator|=(T&& a){
		if(!count(a)){
			assert(size_<N);
			new(&data()[size_]) T(std::move(a));
			size_++;
		}
		return *this;
	}

	template<size_t M>
	set_limited& operator|=(set_limited<T,M> && a){
		for(auto &x:a){
			(*this)|=std::move(x);
		}
		return *this;
	}

	template<size_t M>
	set_limited& operator|=(set_limited<T,M> const& a){
		for(auto const& x:a){
			(*this)|=x;
		}
		return *this;
	}

	std::strong_ordering operator<=>(set_limited const& a)const{
		auto c=size()<=>a.size();
		if(c!=std::strong_ordering::equal){
			return c;
		}
		for(size_t i=0;i<size();i++){
			auto c=data()[i]<=>a.data()[i];
			if(c!=std::strong_ordering::equal){
				return c;
			}
		}
		return std::strong_ordering::equal;
	}

	bool operator==(set_limited const& a)const{
		auto x=(*this)<=>a;
		return x==std::strong_ordering::equal;
	}
};

template<typename T,size_t N>
std::ostream& operator<<(std::ostream& o,set_limited<T,N> const& a){
	o<<"{ ";
	for(auto const& x:a){
		o<<x<<" ";
	}
	return o<<"}";
}

template<typename T,size_t N>
set_limited<T,N> operator&(set_limited<T,N> const& a,set_limited<T,N> const& b){
	set_limited<T,N> r;
	for(auto x:a){
		if(b.count(x)){
			r|=x;
		}
	}
	return r;
}

template<typename T,size_t N>
bool contains(set_limited<T,N> const& a,T const& b){
	for(auto elem:a){
		if(elem==b){
			return 1;
		}
	}
	return 0;
}

#if 0
template<typename T,size_t N>
std::set<T> to_set(set_limited<T,N> const& a){
	return std::set<T>{a.begin(),a.end()};
}
#else
template<typename T,size_t N>
auto to_set(set_limited<T,N> a){
	return std::move(a);
}
#endif

template<typename T,size_t N>
set_limited<T,N> to_set(vector_fixed<T,N> const& a){
	set_limited<T,N> r;
	for(auto x:a){
		r|=x;
	}
	return r;
}

template<typename T,size_t N>
set_limited<T,N> to_set(vector_fixed<T,N> && a){
	set_limited<T,N> r;
	for(auto &x:a) r|=std::move(x);
	return r;
}

template<typename Func,typename T,size_t N>
auto mapf(Func f,set_limited<T,N> const& a){
	using E=decltype(f(*a.begin()));
	vector_fixed<E,N> r;
	for(auto const& elem:a){
		r|=f(elem);
	}
	return r;
}

template<typename T,size_t N,size_t M>
auto flatten(std::array<set_limited<T,N>,M> const& a){
	vector_fixed<T,N*M> r;
	for(auto const& x:a){
		for(auto const& elem:x){
			r|=elem;
		}
	}
	return r;
}

template<typename T,size_t N,size_t M>
auto flatten(std::array<set_limited<T,N>,M> && a){
	vector_fixed<T,N*M> r;
	for(auto& x:a){
		for(auto& elem:x){
			r|=std::move(elem);
		}
	}
	return r;
}

template<typename T,size_t N>
set_limited<T,N> rand(set_limited<T,N> const*);

template<typename T,size_t N>
std::set<T> operator-(std::set<T> a,set_limited<T,N> const& b){
	for(auto const& elem:b){
		a.erase(elem);
	}
	return a;
}

template<typename T,size_t N>
std::set<T>& operator|=(std::set<T>& a,set_limited<T,N> && b);

template<typename T,size_t N>
std::set<T>& operator|=(std::set<T>& a,set_limited<T,N> const& b){
	for(auto x:b){
		a|=x;
	}
	return a;
}

template<typename T,size_t N,size_t M>
set_limited<T,N*M> or_all(std::array<set_limited<T,N>,M> && a){
	set_limited<T,N*M> r;
	for(auto &x:a){
		r|=std::move(x);
	}
	return r;
}

template<typename T,size_t N,size_t M>
auto or_all(std::array<set_limited<T,N>,M> const& a){
	set_limited<T,N*M> r;
	for(auto const& x:a){
		r|=x;
	}
	return r;
}

template<typename T,size_t N>
std::vector<T>& operator|=(std::vector<T>& a,set_limited<T,N> const& b){
	a.insert(a.end(),b.begin(),b.end());
	return a;
}

template<typename T,size_t N>
std::vector<T> flatten(std::vector<set_limited<T,N>> const& a){
	std::vector<T> r;
	for(auto const& x:a){
		r|=x;
	}
	return r;
}

template<typename T,size_t N>
set_limited<T,N> operator-(set_limited<T,N> a,std::set<T> const& b){
	set_limited<T,N> r;
	for(auto &x:a){
		if(!b.count(x)){
			r|=std::move(x);
		}
	}
	return r;
}

template<size_t N,typename T>
auto take(std::set<T> const& a){
	set_limited<T,N> r;
	for(auto const& elem:a){
		if(r.size()>=N){
			return r;
		}
		r|=elem;
	}
	return r;
}

#endif
