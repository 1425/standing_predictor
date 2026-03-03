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

#define GEN_HTML_INPUT(X)\
	X(tba::Year,year)\
	X(std::vector<Output_tuple>,result)\
	X(std::vector<tba::Team>,team_info)\
	X(std::array<TBA_SINGLE_ARG(Cutoff2,MAX_DCMPS)>,dcmp_cutoff_pr)\
	X(Cutoff,cmp_cutoff_pr)\
	X(std::string,title)\
	X(std::string,district_short)\
	X(std::vector<int>,dcmp_size)\
	X(std::map<TBA_SINGLE_ARG(tba::Team_key,Team_points_used)>,points_used)\
	X(bool,plot)

struct Gen_html_input{
	GEN_HTML_INPUT(INST)

	Gen_html_input(tba::Year year1):
		year(year1)
	{}

	auto operator<=>(Gen_html_input const&)const=default;
};

void gen_html(std::ostream&,Gen_html_input const&);

int make_spreadsheet(
	TBA_fetcher &f,
	std::map<tba::District_key,std::map<tba::Team_key,Pr>> const&,
	std::string const& output_dir
);

#endif
