#ifndef CA_H
#define CA_H

#include<iosfwd>
#include<string>
#include<cstdint>
#include "int_limited.h"

class TBA_fetcher;

namespace tba{
	class Team_key;
}

enum class California_region{
	NORTH,SOUTH
};

std::ostream& operator<<(std::ostream& o,California_region);

struct Zipcode{
	std::string data;

	auto operator<=>(Zipcode const&)const=default;
};

std::ostream& operator<<(std::ostream&,Zipcode const&);

California_region california_region(Zipcode const&);

struct City{
	std::string data;

	auto operator<=>(City const&)const=default;
	bool operator==(std::string const&)const;
	bool operator==(City const&)const;
};

std::ostream& operator<<(std::ostream&,City const&);

California_region california_region(City const&);

static constexpr auto MAX_DCMPS=2;
using Dcmp_home=Int_limited<0,MAX_DCMPS-1>;

Dcmp_home calc_dcmp_home(TBA_fetcher&,tba::Team_key const&);

#endif
