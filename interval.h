#ifndef INTERVAL_H
#define INTERVAL_H

#include<iostream>
#include "../tba/data.h"

template<typename T>
struct Interval{
	T min,max;//both inclusive

	Interval()=default;

	explicit Interval(T a):min(a),max(a){}

	Interval(T a,T b):min(a),max(b){
		assert(min<=max);
	}

	Interval& operator=(T const& a){
		min=max=a;
		return *this;
	}

	/*auto size()const{
		if an integer-like type should give max-min+1
		if a float-like type should give max-min+std::numeric_limits::lowest()
	}*/
};

template<typename T>
std::ostream& operator<<(std::ostream& o,Interval<T> const& a){
	o<<"("<<a.min<<","<<a.max<<")";
	return o;
}

std::ostream& operator<<(std::ostream&,Interval<tba::Date> const&);

template<typename T>
auto max(Interval<T> const& a){
	return a.max;
}

template<typename T>
auto min(Interval<T> const& a){
	return a.min;
}

template<typename T>
auto rand(Interval<T> const*){
	auto a=rand((T*)0);
	auto b=rand((T*)0);
	return Interval<T>{std::min(a,b),std::max(a,b)};
}

template<typename T>
bool subset(Interval<T> a,Interval<T> b){
	return a.min>=b.min && a.max<=b.max;
}

#define INTERVAL_COMPARE(X)\
	X(LESS)\
	X(EQUAL)\
	X(GREATER)\
	X(INDETERMINATE)

enum class Interval_compare{
	#define X(A) A,
	INTERVAL_COMPARE(X)
	#undef X
};

using enum Interval_compare;

std::ostream& operator<<(std::ostream&,Interval_compare);

template<typename T>
Interval_compare compare(Interval<T> const& a,Interval<T> const& b){
	if(a.max<b.min){
		return Interval_compare::LESS;
	}
	if(b.max<a.min){
		return Interval_compare::GREATER;
	}
	if(a.min==a.max && b.min==b.max){
		return Interval_compare::EQUAL;
	}
	return Interval_compare::INDETERMINATE;
}

template<typename Func,typename T>
auto apply_monotonic(Func f,Interval<T> const& a){
	//this only works if f is a monotonic function.
	//otherwise, need to go through every possibility.
	auto x1=f(a.min);
	auto x2=f(a.max);
	return Interval{std::min(x1,x2),std::max(x1,x2)};
}



#endif
