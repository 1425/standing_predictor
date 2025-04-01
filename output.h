#ifndef OUTPUT_H
#define OUTPUT_H

#include "../tba/data.h"
#include "../tba/db.h"

class TBA_fetcher;

using Pr=double; //probability

//a point total.
//Note that despite negative point totals being impossible under current
//rules, some of the code depends on this being signed.
using Point=short;

using Extended_cutoff=std::pair<Point,Pr>;

std::string gen_html(
	std::vector<std::tuple<
		tba::Team_key,
		Pr,Point,Point,Point,
		Pr,Point,Point,Point
	>> const& result,
	std::vector<tba::Team> const& team_info,
	std::map<Extended_cutoff,Pr> const& dcmp_cutoff_pr,
	std::map<Extended_cutoff,Pr> const& cmp_cutoff_pr,
	std::string const& title,
	std::string const& district_short,
	tba::Year year,
	int dcmp_size,
	std::map<tba::Team_key,std::tuple<std::vector<int>,int,int>> points_used
);

int make_spreadsheet(
	TBA_fetcher &f,
	std::map<tba::District_key,std::map<tba::Team_key,Pr>> const&,
	std::string const& output_dir
);

#endif
