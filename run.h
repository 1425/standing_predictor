#ifndef RUN_H
#define RUN_H

#include "output.h"

flat_map2<Point,Pr> convolve(flat_map2<Point,Pr> const&,flat_map2<Point,Pr> const&);
flat_map<Point,Pr> convolve(std::map<Point,Pr> const&,std::map<Point,Pr> const&);

#define TEAM_STATUS(X)\
	X(bool,district_chairmans)\
	X(Team_dist,point_dist)\
	X(Dcmp_home,dcmp_home)\
	X(Point,already_earned)

struct Team_status{
	TEAM_STATUS(INST)
};

std::ostream& operator<<(std::ostream&,Team_status const&);

void describe(std::ostream&,Team_status const*);

using By_team=std::map<tba::Team_key,Team_status>;

using Dcmp_dists=std::map<Point,Team_dist>;

#define RUN_INPUT_ITEMS(X)\
	X(std::vector<int>,dcmp_size)\
	X(int,worlds_slots)\
	X(By_team,by_team)\
	X(bool,dcmp_played)\
	X(Dcmp_dists,dcmp_distribution1)

struct Run_input{
	RUN_INPUT_ITEMS(INST)
};

std::ostream& operator<<(std::ostream&,Run_input const&);

using Cutoff_detail=flat_map2<std::pair<Point,double>,double>;
using Cutoff_details=std::array<Cutoff_detail,MAX_DCMPS>;

#define RUN_RESULT_ITEMS(X)\
	X(std::vector<Output_tuple>,result)\
	X(Cutoff_details,cutoff_pr)\
	X(Cutoff_detail,cmp_cutoff_pr)\

struct Run_result{
	RUN_RESULT_ITEMS(INST)
};

std::ostream& operator<<(std::ostream&,Run_result const&);
void print_r(int,Run_result const&);

Run_result run_calc(Run_input);

#endif
