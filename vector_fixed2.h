#ifndef VECTOR_FIXED2_H
#define VECTOR_FIXED2_H

#include "int_limited.h"

template<typename T,size_t N>
class vector_fixed2{
	//Note: This assumes that the elements are default constructable.
	
	using Data=std::array<T,N>;
	Data data;
	Int_limited<0,N> size_;

	public:

	constexpr bool empty()const{
		return size_==0;
	}

	constexpr auto size()const{
		return size_;
	}

	using iterator=Data::iterator;
	using const_iterator=Data::const_iterator;

	constexpr iterator begin(){
		return data.begin();
	}

	constexpr iterator end(){
		return begin()+size();
	}

	constexpr const_iterator begin()const{
		return data.begin();
	}

	constexpr const_iterator end()const{
		return begin()+size();
	}

	using Index=Int_limited<0,N-1>;

	constexpr T& operator[](Index i){
		assert(i<size());
		return data[i];
	}

	constexpr T const& operator[](Index i)const{
		assert(i<size());
		return data[i];
	}

	constexpr vector_fixed2& operator|=(T const& t){
		assert(size_<N);
		data[size_]=t;
		size_++;
		return *this;
	}

	constexpr vector_fixed2& operator|=(T&& a){
		assert(size_<N);
		data[size_]=std::move(a);
		size_++;
		return *this;
	}
};

#endif
