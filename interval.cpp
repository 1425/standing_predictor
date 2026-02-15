#include "interval.h"

std::ostream& operator<<(std::ostream& o,Interval<tba::Date> const& i){
	auto [a,b]=i;
	if(a.year()!=b.year()){
		return o<<"("<<a<<"-"<<b<<")";
	}
	unsigned ad=static_cast<unsigned>(a.day()),bd=static_cast<unsigned>(b.day());
	if(a.month()!=b.month()){
		return o<<a.month()<<" "<<ad<<" - "<<b.month()<<" "<<bd;
	}
	o<<a.month()<<" ";
	if(ad==bd){
		o<<ad;
	}else{
		o<<ad<<"-"<<bd;
	}
	return o;
}

std::ostream& operator<<(std::ostream& o,Interval_compare a){
	#define X(A) if(a==Interval_compare::A) return o<<""#A;
	INTERVAL_COMPARE(X)
	#undef X
	assert(0);
}

