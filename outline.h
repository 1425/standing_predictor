#ifndef OUTLINE_H
#define OUTLINE_H

#include "output.h"
#include "flat_map.h"

flat_map<Point,Pr> convolve(std::map<Point,Pr> const&,std::map<Point,Pr> const&);

#endif
