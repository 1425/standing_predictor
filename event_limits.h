#ifndef EVENT_LIMITS_H
#define EVENT_LIMITS_H

#include "award_limits.h"

template<typename Status,template<typename,typename>typename MAP>
/*std::tuple<
	std::map<tba::Team_key,Interval<Point>>,
	Point,
	Status
>*/
auto points_only(Rank_status<Status,MAP> const& a){
	auto m=map_values(
		[](auto x){
			return Interval{x.min.second,x.max.second};
		},
		a.by_team
	);
	return std::make_tuple(m,a.unclaimed.second,a.status);
}

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

ENUM_CLASS(Tournament_status,TOURNAMENT_STATUS)
Tournament_status rand(Tournament_status const*);

bool in_progress(Tournament_status);

class District_status_future{
	auto operator<=>(District_status_future const&)const=default;
};

std::ostream& operator<<(std::ostream&,District_status_future const&);

struct District_status_locals_in_progress{
	std::map<Tournament_status,std::vector<tba::Event_key>> data;

	auto operator<=>(District_status_locals_in_progress const&)const=default;
};

std::ostream& operator<<(std::ostream&,District_status_locals_in_progress const&);

class District_status_locals_complete{
	auto operator<=>(District_status_locals_complete const&)const=default;
};

std::ostream& operator<<(std::ostream&,District_status_locals_complete const&);

class District_status_dcmp_in_progress{
	//set of event status? or map of event_name -> status
	auto operator<=>(District_status_dcmp_in_progress const&)const=default;
};

std::ostream& operator<<(std::ostream&,District_status_dcmp_in_progress const&);

class District_status_complete{
	auto operator<=>(District_status_complete const&)const=default;
};

std::ostream& operator<<(std::ostream&,District_status_complete const&);

using District_status=std::variant<
	District_status_future,
	District_status_locals_in_progress,
	District_status_locals_complete,
	District_status_dcmp_in_progress,
	District_status_complete
>;

bool in_progress(District_status);

Rank_status<Tournament_status> event_limits(TBA_fetcher&,tba::Event_key const&);
Rank_status<District_status> district_limits(TBA_fetcher&,tba::District_key const&);

int event_limits_demo(TBA_fetcher&);

#endif
