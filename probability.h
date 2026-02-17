#ifndef PROBABILITY_H
#define PROBABILITY_H

#include "flat_map2.h"

using Pr=double; //probability

double entropy(Pr);

//a point total.
//Note that despite negative point totals being impossible under current
//rules, some of the code depends on this being signed.
using Point=short;

using Team_dist=flat_map2<Point,Pr>;



#endif
