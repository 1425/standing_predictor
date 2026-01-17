#ifndef OUTLINE_H
#define OUTLINE_H

#include "output.h"
#include "flat_map.h"
#include "ca.h"

flat_map<Point,Pr> convolve(std::map<Point,Pr> const&,std::map<Point,Pr> const&);

Dcmp_home calc_dcmp_home(TBA_fetcher&,tba::Team_key const&);
California_region california_region(tba::Team const&);

#endif
