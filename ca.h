#ifndef CA_H
#define CA_H

#include<iosfwd>
#include<string>
#include<cstdint>
#include "int_limited.h"
#include "io.h"

namespace tba{
	struct Team;
}

class TBA_fetcher;
struct City;
struct Zipcode;

#define CALIFORNIA_REGION_OPTIONS(X)\
	X(NORTH)\
	X(SOUTH)\

ENUM_CLASS(California_region,CALIFORNIA_REGION_OPTIONS)

California_region california_region(Zipcode const&);
California_region california_region(City const&);
California_region california_region(tba::Team const&);

#endif
