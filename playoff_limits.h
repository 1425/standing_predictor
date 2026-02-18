#ifndef PLAYOFF_LIMITS_H
#define PLAYOFF_LIMITS_H

#include "rank_limits.h" //for Point_range
#include "../tba/data.h"

namespace tba{
	class Team_key;
}

class TBA_fetcher;

#define PLAYOFF_LIMITS(X)\
	X(Point_range<tba::Team_key>,by_team)\
	X(unsigned,unclaimed_points)

TBA_MAKE_INST(Playoff_limits,PLAYOFF_LIMITS)

Playoff_limits playoff_limits(TBA_fetcher&,std::map<tba::Team_key,Interval<bool>> const&);

int playoff_limits_demo(TBA_fetcher&);

#endif
