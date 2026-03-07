#ifndef EVENT_LIMITS_H
#define EVENT_LIMITS_H

#include "award_limits.h"
#include "tournament_status.h"

template<typename Status,template<typename,typename>typename MAP>
/*std::tuple<
	std::map<tba::Team_key,Interval<Point>>,
	Point,
	Status
/>*/
auto points_only(Rank_status<Status,MAP> const& a){
	auto m=map_values(
		[](auto x){
			return Interval{x.min.second,x.max.second};
		},
		a.by_team
	);
	return std::make_tuple(m,a.unclaimed.second,a.status);
}

STRUCT_DECLARE(District_status_future,EMPTY)

struct District_status_locals_in_progress{
	std::map<Tournament_status,std::vector<tba::Event_key>> data;

	auto operator<=>(District_status_locals_in_progress const&)const=default;
};

std::ostream& operator<<(std::ostream&,District_status_locals_in_progress const&);

STRUCT_DECLARE(District_status_locals_complete,EMPTY)

struct District_status_dcmp_in_progress{
	std::map<Tournament_status,std::vector<tba::Event_key>> data;

	auto operator<=>(District_status_dcmp_in_progress const&)const=default;
};

std::ostream& operator<<(std::ostream&,District_status_dcmp_in_progress const&);

STRUCT_DECLARE(District_status_complete,EMPTY)

using District_status=std::variant<
	District_status_future,
	District_status_locals_in_progress,
	District_status_locals_complete,
	District_status_dcmp_in_progress,
	District_status_complete
>;

bool in_progress(District_status);

Rank_status<Tournament_status> event_limits(TBA_fetcher&,tba::Event_key const&,bool normal=1);

Rank_status<District_status> district_limits(TBA_fetcher&,tba::District_key const&);

int event_limits_demo(TBA_fetcher&);

#endif
