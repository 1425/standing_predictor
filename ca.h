#ifndef CA_H
#define CA_H

#include<iosfwd>
#include<string>
#include<cstdint>
#include "int_limited.h"

namespace tba{
	struct Team;
}

class TBA_fetcher;
struct City;
struct Zipcode;

enum class California_region{
	NORTH,SOUTH
};

std::ostream& operator<<(std::ostream& o,California_region);

California_region california_region(Zipcode const&);
California_region california_region(City const&);
California_region california_region(tba::Team const&);

#endif
