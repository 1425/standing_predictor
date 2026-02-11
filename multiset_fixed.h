#ifndef MULTISET_FIXED_H
#define MULTISET_FIXED_H

template<typename T>
struct Converter{

};

template<>
struct Converter<Interval_compare>{
	static constexpr auto MAX=Interval_compare::INDETERMINATE;

	static constexpr uint8_t from(Interval_compare a){
		return (uint8_t)a;
	}

	static constexpr Interval_compare to(uint8_t a){
		return (Interval_compare)a;
	}
};

template<typename T>
class multiset_fixed{
	using C=Converter<T>;
	//could check that the min value is >=0
	static constexpr auto N=C::from(C::MAX)+1;
	using Data=std::array<size_t,N>;
	Data data;

	public:

	multiset_fixed(){
		for(auto &elem:data){
			elem=0;
		}
	}

	multiset_fixed& operator|=(T a){
		data[C::from(a)]++;
		return *this;
	}

	size_t count(T const& a){
		return data[C::from(a)];
	}
};



#endif
