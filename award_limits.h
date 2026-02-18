#ifndef AWARD_LIMITS_H
#define AWARD_LIMITS_H

#include "rank_limits.h"

class TBA_fetcher;

namespace tba{
	class Team_key;
	class Event_key;
};

#define AWARD_LIMITS(X)\
	X(Point_range<tba::Team_key>,by_team)\
	X(size_t,unclaimed)

struct Award_limits{
	AWARD_LIMITS(INST)
};

std::ostream& operator<<(std::ostream&,Award_limits const&);

Award_limits award_limits(TBA_fetcher&,tba::Event_key const&);

int award_limits_demo(TBA_fetcher&);

#endif
