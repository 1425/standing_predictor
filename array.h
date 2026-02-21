#ifndef ARRAY_H
#define ARRAY_H

#include<array>
#include<numeric>
#include<algorithm>

template<size_t N>
struct array_void{
};

template<typename T,size_t N>
auto sorted(std::array<T,N> a){
	std::sort(a.begin(),a.end());
	return a;
}

template<typename T,size_t N>
auto second(std::array<T,N> const& a){
	static_assert(N>=2);
	return a[1];
}

template<size_t N>
constexpr std::array<size_t,N> range_st(){
	std::array<size_t,N> r;
	for(size_t i=0;i<N;i++){
		r[i]=i;
	}
	return r;
}

template<long long MIN,long long LIM>
constexpr auto range_st(){
	static constexpr auto N=LIM-MIN;
	std::array<long long,N> r;
	for(size_t i=0;i<N;i++){
		r[i]=MIN+i;
	}
	return r;
}


template<typename Func,typename T,size_t N>
constexpr auto mapf(Func f,std::array<T,N> const& a){
	using E=decltype(f(*begin(a)));
	if constexpr(std::is_same<void,E>()){
		for(auto const& elem:a){
			f(elem);
		}
		return array_void<N>();
	}else{
		using R=std::array<E,N>;
		if constexpr(std::is_constructible<R>::value){
			R r;
			for(size_t i=0;i<N;++i){
				r[i]=f(a[i]);
			}
			return r;
		}else{
			alignas(R) char buf[sizeof(R)];
			R& r=*(R*)buf;
			for(size_t i=0;i<N;++i){
				new(&r[i]) E(f(a[i]));
			}
			return r;
		}
	}
}

template<typename T,size_t N>
auto enumerate(std::array<T,N> const& a){
	return mapf([=](auto i){ return std::make_pair(i,a[i]); },range_st<N>());
}

template<typename T,size_t N>
std::array<std::pair<size_t,T>,N> enumerate_from(size_t start,std::array<T,N> const& a){
	using P=std::pair<size_t,T>;
	std::array<P,N> r;
	size_t i=0;
	for(auto const& elem:a){
		r[i]=P(start+i,elem);
		i++;
	}
	return r;
}

template<typename T,size_t N>
std::array<T,N> as_array(std::vector<T> const& a){
	assert(a.size()==N);
	return mapf([&](auto x){ return a[x]; },range_st<N>());
}

template<typename T>
auto to_array(std::pair<T,T> a){
	return std::array<T,2>{a.first,a.second};
}

template<typename T,size_t N>
bool contains(std::array<T,N> const& a,T const& b){
	for(auto const& elem:a){
		if(elem==b){
			return 1;
		}
	}
	return 0;
}

template<typename T,size_t N,size_t M>
std::array<T,N*M> flatten(std::array<std::array<T,N>,M> const& a){
	using R=std::array<T,N*M>;
	alignas(R) char buf[sizeof(R)];
	R& r=*(R*)buf;
	size_t i=0;
	for(auto const& x:a){
		for(auto const& elem:x){
			new(&r[i]) T(elem);
			i++;
		}
	}
	return r;
}

auto quartiles(auto a){
	assert(!a.empty());
	auto b=sorted(a);
	return std::array{
		b[0],
		b[b.size()/4],
		b[b.size()/2],
		b[b.size()*3/4],
		b[b.size()-1]
	};
}

template<typename T>
auto deciles(std::vector<T> a){
	assert(!a.empty());
	std::sort(a.begin(),a.end());
	return mapf(
		[=](auto i){
			return a[i*a.size()/10];
		},
		range_st<10>()
	);
}

template<typename A,typename B,size_t N>
std::array<std::pair<A,B>,N> zip(std::array<A,N> const& a,std::array<B,N> const& b){
	return mapf([=](auto i){ return std::make_pair(a[i],b[i]); },range_st<N>());
}

template<typename T,size_t N>
auto sum(std::array<T,N> const& a){
	return std::accumulate(a.begin(),a.end(),T{});
}

template<typename A,typename B,size_t N>
std::array<A,N>& operator+=(std::array<A,N> &a,std::array<B,N> const& b){
	for(auto i:range_st<N>()){
		a[i]+=b[i];
	}
	return a;
}

#endif
