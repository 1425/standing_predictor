#ifndef EVENT_PARTIAL_H
#define EVENT_PARTIAL_H

#include<map>
#include "probability.h"
#include "run.h"
#include "skill.h"
#include "event_categories.h"
#include "annotated_complex.h"

class TBA_fetcher;

STRUCT_SINGLE(Team_event_status_rank,Interval<Point>,data)
STRUCT_SINGLE(Team_event_status_post_rank,Point,data)
STRUCT_SINGLE(Team_event_status_post_pick,Point,data)
STRUCT_SINGLE(Team_event_status_post_elims,Point,data)

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

std::tuple<
	Run_input,
	Skill_estimates,
	Annotated,
	std::map<tba::Team_key,std::string>
> read_status(TBA_fetcher&,tba::District_key const&,Skill_method,std::optional<tba::Date>);

int event_partial_demo(TBA_fetcher&);

#endif
