#ifndef PROBABILITY_H
#define PROBABILITY_H

#include "flat_map2.h"
#include "interval.h"
#include "map_auto.h"

using Pr=double; //probability

double entropy(Pr);

//a point total.
//Note that despite negative point totals being impossible under current
//rules, some of the code depends on this being signed.
using Point=short;

using Team_dist=flat_map2<Point,Pr>;

template<typename T>
double entropy(Interval<T> const& a){
	int n=a.max-a.min+1;
	return log2(n);
}

template<typename T>
double entropy(map_auto<tba::Team_key,Interval<T>> const& a){
	return sum(MAP(entropy,values(a)));
}

std::map<Point,Pr> operator+(std::map<Point,Pr>,int);
flat_map<Point,Pr> operator+(flat_map<Point,Pr> const&,int);
flat_map2<Point,Pr> operator+(flat_map2<Point,Pr> const&,int);

#endif
