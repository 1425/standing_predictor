#ifndef INTERVAL_H
#define INTERVAL_H

#include<iostream>
#include "../tba/data.h"

template<typename T>
struct Interval{
	T min,max;//both inclusive

	Interval()=default;

	explicit Interval(T a):min(a),max(a){}

	constexpr Interval(T a,T b):min(a),max(b){
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

	bool operator==(T const& a){
		return min==a && max==a;
	}

	Interval& operator+=(Interval const& a){
		min+=a.min;
		max+=a.max;
		return *this;
	}

	auto width()const{
		return max-min;
	}

	auto options()const{
		//this is accurate if T is integer-like, anyway.
		return width+1;
	}

	//This exists so that this type can work with things like std::map
	//And does not actually give you a logical comparison between two intervals.
	auto operator<=>(Interval const&)const=default;
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

/*template<typename T>
bool subset(T a,Interval<T> b){
	return a>=b.min && a<=b.max;
}*/

template<typename A,typename B>
bool subset(A a,Interval<B> b){
	return a>=b.min && a<=b.max;
}

template<typename T>
bool match(Interval<T> const& a,Interval<T> const& b){
	return a.min==b.min && a.max==b.max;
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

template<typename T>
Interval_compare compare(std::optional<Interval<T>> const& a,Interval<T> const& b){
	//treating the null optional as an empty interval.
	if(a){
		return compare(*a,b);
	}
	return Interval_compare::INDETERMINATE;
}

template<typename T>
bool overlap(Interval<T> const& a,Interval<T> const& b){
	auto c=compare(a,b);
	switch(c){
		case LESS:
		case GREATER:
			return 0;
		case EQUAL:
		case INDETERMINATE:
			return 1;
		default:
			assert(0);
	}
}

template<typename Func,typename T>
auto apply_monotonic(Func f,Interval<T> const& a){
	//this only works if f is a monotonic function.
	//otherwise, need to go through every possibility.
	auto x1=f(a.min);
	auto x2=f(a.max);
	return Interval{std::min(x1,x2),std::max(x1,x2)};
}

template<typename T>
std::optional<Interval<T>> or_all(std::vector<Interval<T>> a){
	if(a.empty()){
		return std::nullopt;
	}
	auto m1=min(MAP(min,a));
	auto m2=max(MAP(max,a));
	return Interval<T>{m1,m2};
}

template<typename T>
auto sum(std::vector<Interval<T>> const& a){
	//seperating these so that you get something useful when T is bool
	return Interval{
		sum(MAP(min,a)),
		sum(MAP(max,a))
	};
}

#endif
