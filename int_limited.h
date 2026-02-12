#ifndef INT_LIMITED_H
#define INT_LIMITED_H

#include<cstddef>
#include<cassert>
#include<vector>
#include<cstdlib>
#include<cstdint>
#include<ostream>

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

	public:
	Int_limited():data(MIN){
		static_assert(MIN<=MAX);
	}

	constexpr Int_limited(Data a):data(a){
		assert(a>=MIN);
		assert(a<=MAX);
	}

	auto get()const{
		return data;
	}

	operator Data()const{
		return data;
	}

	template<long long MIN2,long long MAX2>
	Int_limited& operator+=(Int_limited<MIN2,MAX2> a){
		data+=a.get();
		return *this;
	}

	Int_limited& operator+=(int a){
		data+=a;
		return *this;
	}

	Int_limited operator++(int){
		auto r=*this;
		data++;
		return r;
	}

	Int_limited& operator++(){
		data++;
		return *this;
	}

	auto operator<=>(Int_limited const&)const=default;

	bool operator>(size_t a)const{
		return data>(long long)a;
	}

	bool operator<(auto a)const{
		return data<(long long)a;
	}

	bool operator<=(auto a)const{
		return data<=a;
	}

	bool operator==(auto a)const{
		return data==a;
	}
};

template<long long MIN,long long MAX>
std::ostream& operator<<(std::ostream& o,Int_limited<MIN,MAX> a){
	return o<<(long long)a.get();
}

template<long long MIN,long long MAX>
bool operator<=(size_t a,Int_limited<MIN,MAX> const& b){
	return a<=b.get();
}

template<long long MIN,long long MAX>
auto options(Int_limited<MIN,MAX> const*){
	using E=Int_limited<MIN,MAX>;
	std::vector<E> r;
	for(long long i=MIN;i<=MAX;i++){
		r|=i;
	}
	return r;
}

template<long long MIN,long long MAX>
auto rand(Int_limited<MIN,MAX> const*){
	return Int_limited<MIN,MAX>(MIN+rand()%(MAX-MIN+1));
}

#endif
