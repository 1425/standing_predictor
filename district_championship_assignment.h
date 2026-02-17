#ifndef DISTRICT_CHAMPIONSHIP_ASSIGNMENT_H
#define DISTRICT_CHAMPIONSHIP_ASSIGNMENT_H

#include "int_limited.h"

namespace tba{
	class Team_key;
};

class TBA_fetcher;

static constexpr auto MAX_DCMPS=2;
using Dcmp_home=Int_limited<0,MAX_DCMPS-1>;

Dcmp_home calc_dcmp_home(TBA_fetcher&,tba::Team_key const&);

#endif
