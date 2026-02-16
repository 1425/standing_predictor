#ifndef OUTPUT_H
#define OUTPUT_H

#include "../tba/data.h"
#include "../tba/db.h"
#include "flat_map2.h"
#include "ca.h"

class TBA_fetcher;

using Pr=double; //probability

double entropy(Pr);

//a point total.
//Note that despite negative point totals being impossible under current
//rules, some of the code depends on this being signed.
using Point=short;

using Team_dist=flat_map2<Point,Pr>;

using Extended_cutoff=std::pair<Point,Pr>;

using Cutoff=std::map<Extended_cutoff,Pr>;
using Cutoff2=flat_map2<Extended_cutoff,Pr>;

struct Output_tuple{
	tba::Team_key team;
	Dcmp_home dcmp_home;

	Pr dcmp_make;
	std::array<Point,3> dcmp_interesting;

	Pr cmp_make;
	std::array<Point,3> cmp_interesting;

	auto operator<=>(Output_tuple const&)const=default;
};

std::ostream& operator<<(std::ostream&,Output_tuple const&);

#define TEAM_POINTS_USED(X)\
	X(std::vector<Point>,event_points_earned)\
	X(Point,rookie_bonus)\
	X(int,events_left)\
	X(Team_dist,pre_dcmp_dist)

struct Team_points_used{
	TEAM_POINTS_USED(INST)
};

std::ostream& operator<<(std::ostream&,Team_points_used const&);

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

template<typename T>
std::string join(std::string const& s,std::vector<T> const& v){
	if(v.empty()) return "";

	std::stringstream ss;
	auto at=v.begin();
	ss<<*at;
	at++;
	while(at!=v.end()){
		ss<<s<<*at;
		at++;
	}
	return ss.str();
}

#endif
