#ifndef EVENT_LIMITS_H
#define EVENT_LIMITS_H

#include "award_limits.h"

template<typename Status>
std::tuple<
	std::map<tba::Team_key,Interval<Point>>,
	Point,
	Event_status
> points_only(Rank_status<Status> const&);

#define TOURNAMENT_STATUS(X)\
	X(FUTURE)\
	X(QUAL_MATCHES_IN_PROGRESS)\
	X(QUAL_MATCHES_COMPLETE)\
	X(PICKING_IN_PROGRESS)\
	X(PICKING_COMPLETE)\
	X(ELIMINATIONS_IN_PROGRESS)\
	X(ELIMINATIONS_COMPLETE)\
	X(AWARDS_IN_PROGRESS)\
	X(COMPLETE)\

enum class Tournament_status{
	#define X(A) A,
	TOURNAMENT_STATUS(X)
	#undef X
};

std::ostream& operator<<(std::ostream&,Tournament_status const&);

bool in_progress(Tournament_status);

#define DISTRICT_STATUS(X)\
	X(FUTURE)\
	X(LOCALS_IN_PROGRESS)\
	X(LOCALS_COMPLETE)\
	X(DCMP_IN_PROGRESS)\
	X(COMPLETE)\

enum class District_status{
	#define X(A) A,
	DISTRICT_STATUS(X)
	#undef X
};

std::ostream& operator<<(std::ostream&,District_status const&);

bool in_progress(District_status);

Rank_status<Tournament_status> event_limits(TBA_fetcher&,tba::Event_key const&);
Rank_status<Event_status> district_limits(TBA_fetcher&,tba::District_key const&);

int event_limits_demo(TBA_fetcher&);

#endif
