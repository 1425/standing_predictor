#ifndef EVENT_PARTIAL_H
#define EVENT_PARTIAL_H

#include<map>
#include "probability.h"
#include "run.h"

class TBA_fetcher;

struct Team_event_status_rank{
	Interval<Point> data;
};

std::ostream& operator<<(std::ostream&,Team_event_status_rank const&);

struct Team_event_status_post_rank{
	Point data;
};

std::ostream& operator<<(std::ostream&,Team_event_status_post_rank const&);

struct Team_event_status_post_pick{
	Point data;
};

std::ostream& operator<<(std::ostream&,Team_event_status_post_pick const&);

struct Team_event_status_post_elims{
	Point data;
};

std::ostream& operator<<(std::ostream&,Team_event_status_post_elims const&);

using Team_event_status=std::variant<
	Team_event_status_rank,
	Team_event_status_post_rank,
	Team_event_status_post_pick,
	Team_event_status_post_elims
>;


using Conditional_distribution=map_auto<Point,Team_dist>;

#define EVENT_PARTIAL(X)\
	X(Conditional_distribution,post_rank)\
	X(Conditional_distribution,post_pick)\
	X(Conditional_distribution,post_elims)\

struct Event_partial{
	EVENT_PARTIAL(INST)

	Team_dist operator[](Team_event_status_rank const&)const;
	Team_dist operator[](Team_event_status_post_rank const&)const;
	Team_dist operator[](Team_event_status_post_pick const&)const;
	Team_dist operator[](Team_event_status_post_elims const&)const;
	Team_dist operator[](Team_event_status const&)const;
};

std::ostream& operator<<(std::ostream&,Event_partial const&);

Event_partial event_partial(TBA_fetcher&);
Run_input read_status(TBA_fetcher&,tba::District_key const&);

int event_partial_demo(TBA_fetcher&);

#endif
