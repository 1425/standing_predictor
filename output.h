#ifndef OUTPUT_H
#define OUTPUT_H

#include "../tba/data.h"
#include "../tba/db.h"
#include "flat_map2.h"
#include "district_championship_assignment.h"
#include "probability.h"

class TBA_fetcher;

using Extended_cutoff=std::pair<Point,Pr>;

using Cutoff=std::map<Extended_cutoff,Pr>;
using Cutoff2=flat_map2<Extended_cutoff,Pr>;

using A_Point_3=std::array<Point,3>;

#define OUTPUT_TUPLE(X)\
	X(tba::Team_key,team)\
	X(Dcmp_home,dcmp_home)\
	X(Pr,dcmp_make)\
	X(A_Point_3,dcmp_interesting)\
	X(Pr,cmp_make)\
	X(A_Point_3,cmp_interesting)

STRUCT_DECLARE(Output_tuple,OUTPUT_TUPLE)
Output_tuple rand(Output_tuple const*);

#define TEAM_POINTS_USED(X)\
	X(std::vector<Point>,event_points_earned)\
	X(Point,rookie_bonus)\
	X(int,events_left)\
	X(Team_dist,pre_dcmp_dist)

STRUCT_DECLARE(Team_points_used,TEAM_POINTS_USED)

std::string gen_html(
	std::vector<Output_tuple> const& result,
	std::vector<tba::Team> const& team_info,
	std::array<Cutoff2,MAX_DCMPS> const& dcmp_cutoff_pr,
	Cutoff const& cmp_cutoff_pr,
	std::string const& title,
	std::string const& district_short,
	tba::Year year,
	std::vector<int> dcmp_size,
	std::map<tba::Team_key,Team_points_used> points_used
);

int make_spreadsheet(
	TBA_fetcher &f,
	std::map<tba::District_key,std::map<tba::Team_key,Pr>> const&,
	std::string const& output_dir
);

#endif
