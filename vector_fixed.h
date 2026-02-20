#ifndef VECTOR_FIXED_H
#define VECTOR_FIXED_H

#include "int_limited.h"

template<typename T,size_t N>
class vector_fixed{
	//Note: This will not work in constexpr context.
	//Also, the makes some assumptions about non-throwing moves etc.

	using Data=std::array<T,N>;
	alignas(Data) char buf[sizeof(Data)];
	Int_limited<0,N> size_;

	Data const& data()const{
		return *(Data*)buf;
	}

	Data& data(){
		return *(Data*)buf;
	}

	public:

	vector_fixed()=default;

	vector_fixed(vector_fixed const& a){
		//adding these two extra lines rather than using a.size() directly
		//to silence a warning on g++ 16 at high optimization levels
		size_t n=a.size_;
		assert(n<=N);
		for(size_t i=0;i<n;i++){
			new( &(data()[i]) ) T(a[i]);
		}
		size_=n;
	}

	vector_fixed& operator=(vector_fixed const& a){
		if(size()<a.size()){
			size_t i=0;
			for(;i<size();i++){
				data()[i]=a[i];
			}
			for(;i<a.size();i++){
				new(&data()[i]) T(a[i]);
			}
		}else{
			size_t i=0;
			for(;i<a.size();i++){
				data()[i]=a[i];
			}
			for(;i<size();i++){
				data()[i].~T();
			}
		}
		size_=a.size_;
		return *this;
	}

	vector_fixed(vector_fixed&& a){
		for(size_t i=0;i<a.size();i++){
			new(&data()[i]) T(std::move(a[i]));
		}
		size_=a.size();
	}

	vector_fixed& operator=(vector_fixed&& a){
		if(size()<a.size()){
			size_t i=0;
			for(;i<size();i++){
				data()[i]=std::move(a[i]);
			}
			for(;i<a.size();i++){
				new(&data()[i]) T(std::move(a[i]));
			}
		}else{
			size_t i=0;
			for(;i<a.size();i++){
				data()[i]=std::move(a[i]);
			}
			for(;i<size();i++){
				data()[i].~T();
			}
		}
		size_=a.size_;
		return *this;
	}

	constexpr ~vector_fixed(){
		for(size_t i=0;i<size_;i++){
			data()[i].~T();
		}
	}

	vector_fixed(std::vector<T> const& a){
		for(auto const& x:a){
			(*this)|=x;
		}
	}

	constexpr bool empty()const{
		return size_==0;
	}

	constexpr auto size()const{
		return size_;
	}

	using iterator=Data::iterator;
	using const_iterator=Data::const_iterator;

	constexpr iterator begin(){
		return data().begin();
	}

	constexpr iterator end(){
		return begin()+size();
	}

	constexpr const_iterator begin()const{
		return data().begin();
	}

	constexpr const_iterator end()const{
		return begin()+size();
	}

	using Index=Int_limited<0,N-1>;

	constexpr T& operator[](Index i){
		assert(i<size());
		return data()[i];
	}

	constexpr T const& operator[](Index i)const{
		assert(i<size());
		return data()[i];
	}

	constexpr T& operator[](size_t i){
		return (*this)[Index(i)];
	}

	constexpr T const& operator[](size_t i)const{
		return (*this)[Index(i)];
	}

	constexpr vector_fixed& operator|=(T const& t){
		if(size_>=N){
			throw std::invalid_argument("not enough free space");
		}
		new(&data()[size_]) T(t);
		size_++;
		return *this;
	}

	constexpr vector_fixed& operator|=(T&& a){
		if(size_>=N){
			throw std::invalid_argument("not enough free space");
		}
		new(&data()[size_]) T(std::move(a));
		size_++;
		return *this;
	}

	std::strong_ordering operator<=>(vector_fixed const& a)const{
		auto cmp=(size()<=>a.size());
		if(cmp!=std::strong_ordering::equal){
			return cmp;
		}
		for(size_t i=0;i<size();i++){
			auto c=(*this)[i]<=>a[i];
			if(c!=std::strong_ordering::equal){
				return c;
			}
		}
		return std::strong_ordering::equal;
	}
};

template<typename T,size_t N>
std::ostream& operator<<(std::ostream& o,vector_fixed<T,N> const& a){
	o<<"[ ";
	for(auto const& x:a){
		o<<x<<" ";
	}
	return o<<"]";
}

/*template<typename T,size_t N>
auto to_set(vector_fixed<T,N> const& a){
	return std::set<T>{a.begin(),a.end()};
}*/

/*template<typename T,size_t N>
class set_limited;

template<typename T,size_t N>
set_limited<T,N> to_set(vector_fixed<T,N>);*/

template<typename T,size_t N>
auto rand(vector_fixed<T,N> const*){
	vector_fixed<T,N> r;
	for(auto _:range(rand()%(N+1))){
		(void)_;
		r|=rand((T*)0);
	}
	return r;
}

template<typename T,size_t N>
auto skip(size_t n,vector_fixed<T,N> const& a){
	vector_fixed<T,N> r;
	for(size_t i=n;i<a.size();i++){
		r|=a[i];
	}
	return r;
}

template<typename T,size_t N>
auto skip(size_t n,std::array<T,N> const& a){
	vector_fixed<T,N> r;
	for(size_t i=n;i<a.size();i++){
		r|=a[i];
	}
	return r;
}

/*template<long long MIN,long long MAX,typename T,size_t N>
auto skip(Int_limited<MIN,MAX> a,vector_fixed<T,N> const& b){
	return skip(a.get(),b);
}*/

template<typename Func,typename T,size_t N>
auto count_if(Func f,vector_fixed<T,N> const& a){
	Int_limited<0,N> r;
	for(auto const& x:a){
		if(f(x)){
			r++;
		}
	}
	return r;
}

template<size_t N,typename T>
auto take(std::vector<T> const& a){
	vector_fixed<T,N> r;
	for(size_t i=0;i<N && i<a.size();i++){
		r|=a;
	}
	return r;
}

template<typename T,size_t N>
auto sum(vector_fixed<T,N> const& a){
	return std::accumulate(a.begin(),a.end(),T());
}

#endif
