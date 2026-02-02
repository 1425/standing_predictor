#ifndef ARRAY_H
#define ARRAY_H

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
std::array<size_t,N> range_st(){
	std::array<size_t,N> r;
	for(size_t i=0;i<N;i++){
		r[i]=i;
	}
	return r;
}

template<typename Func,typename T,size_t N>
auto mapf(Func f,std::array<T,N> const& a){
	using E=decltype(f(*begin(a)));
	std::array<E,N> r;
	for(auto i:range_st<N>()){
		r[i]=f(a[i]);
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

#endif
