#ifndef EVENT_LIMITS_H
#define EVENT_LIMITS_H

#include "award_limits.h"

Rank_status event_limits(TBA_fetcher&,tba::Event_key const&);
Rank_status district_limits(TBA_fetcher&,tba::District_key const&);

int event_limits_demo(TBA_fetcher&);

std::pair<std::map<tba::Team_key,Interval<Point>>,Point> points_only(Rank_status const&);

#endif
