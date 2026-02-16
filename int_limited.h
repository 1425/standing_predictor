#ifndef INT_LIMITED_H
#define INT_LIMITED_H

#include<cstddef>
#include<cassert>
#include<vector>
#include<cstdlib>
#include<cstdint>
#include<ostream>
#include "array.h"
#include "util.h"

template<long long MIN,long long MAX>
auto get_int(){
	if constexpr(MIN>=0 && MAX<=255){
		return uint8_t(0);
	}else if constexpr(MIN>=0 && MAX<=std::numeric_limits<uint16_t>::max()){
		return uint16_t(0);
	}else{
		return (long long)0;
	}
}

template<long long MIN,long long MAX>
class Int_limited{
	using Data=decltype(get_int<MIN,MAX>());
	Data data;

	constexpr void check()const{
		assert(data>=MIN);
		assert(data<=MAX);
	}

	public:
	constexpr Int_limited():data(MIN){
		static_assert(MIN<=MAX);
	}

	constexpr Int_limited(Data a):data(a){
		assert(a>=MIN);
		assert(a<=MAX);
	}

	auto get()const{
		return data;
	}

	constexpr operator Data()const{
		return data;
	}

	template<long long MIN2,long long MAX2>
	Int_limited& operator+=(Int_limited<MIN2,MAX2> a){
		data+=a.get();
		return *this;
	}

	Int_limited& operator+=(int a){
		data+=a;
		check();
		return *this;
	}

	constexpr Int_limited operator++(int){
		auto r=*this;
		data++;
		check();
		return r;
	}

	Int_limited& operator++(){
		data++;
		check();
		return *this;
	}

	auto operator<=>(Int_limited const&)const=default;

	constexpr bool operator>(size_t a)const{
		return data>(long long)a;
	}

	constexpr bool operator<(auto a)const{
		return data<(long long)a;
	}

	constexpr bool operator<=(auto a)const{
		return data<=a;
	}

	constexpr bool operator==(auto a)const{
		return data==a;
	}

	constexpr bool operator>=(auto a)const{
		return data>=a;
	}

	template<long long MIN2,long long MAX2>
	constexpr bool operator<(Int_limited<MIN2,MAX2> a)const{
		return get()<a.get();
	}

	template<long long MIN2,long long MAX2>
	constexpr bool operator<=(Int_limited<MIN2,MAX2> a)const{
		return get()<=a.get();
	}
};

template<typename T,long long MIN,long long MAX>
bool operator>(T a,Int_limited<MIN,MAX> b){
	return a>b.get();
}

template<typename T,long long MIN,long long MAX>
bool operator>=(T a,Int_limited<MIN,MAX> b){
	return a>=b.get();
}

template<typename T,long long MIN,long long MAX>
bool operator<(T a,Int_limited<MIN,MAX> b){
	return a<b.get();
}

template<typename T,long long MIN,long long MAX>
bool operator<=(T a,Int_limited<MIN,MAX> b){
	return a<=b.get();
}

template<long long MIN,long long MAX>
std::ostream& operator<<(std::ostream& o,Int_limited<MIN,MAX> a){
	return o<<(long long)a.get();
}

template<long long MIN,long long MAX>
bool operator<=(size_t a,Int_limited<MIN,MAX> const& b){
	return a<=b.get();
}

template<long long MIN,long long MAX>
constexpr auto options(Int_limited<MIN,MAX> const*){
	using E=Int_limited<MIN,MAX>;
	auto r=range_st<MIN,MAX+1>();
	return MAP(E,r);
	//return range_st<MIN,MAX+1>();
	/*std::vector<E> r;
	for(long long i=MIN;i<=MAX;i++){
		r|=i;
	}
	return r;*/
}

template<long long MIN,long long MAX>
auto rand(Int_limited<MIN,MAX> const*){
	return Int_limited<MIN,MAX>(MIN+rand()%(MAX-MIN+1));
}

template<long long MIN,long long MAX,size_t N>
auto sum(std::array<Int_limited<MIN,MAX>,N> const& a){
	//could put some logic in here to see that this doesn't overflow
	using R=Int_limited<MIN*N,MAX*N>;
	return std::accumulate(a.begin(),a.end(),R());
}

Int_limited<0,3> sum(std::tuple<bool,bool,bool> const&);

#endif
