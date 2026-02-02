#ifndef INTERVAL_H
#define INTERVAL_H

#include<iostream>
#include "../tba/data.h"

template<typename T>
struct Interval{
	T min,max;//both inclusive

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

#endif
