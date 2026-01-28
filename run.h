#ifndef RUN_H
#define RUN_H

#include "output.h"

flat_map2<Point,Pr> convolve(flat_map2<Point,Pr> const&,flat_map2<Point,Pr> const&);
flat_map<Point,Pr> convolve(std::map<Point,Pr> const&,std::map<Point,Pr> const&);

using Team_dist=flat_map2<Point,Pr>;

struct Team_status{
	bool district_chairmans;
	Team_dist point_dist; //number of points expected pre-dcmp
	Dcmp_home dcmp_home;
	Point already_earned;
};

std::ostream& operator<<(std::ostream&,Team_status const&);

void describe(std::ostream&,Team_status const*);

using By_team=std::map<tba::Team_key,Team_status>;

struct Run_input{
	std::vector<int> dcmp_size;
	int worlds_slots;

	By_team by_team;

	bool dcmp_played;
	std::map<Point,Team_dist> dcmp_distribution1;
};

using Cutoff_detail=flat_map2<std::pair<Point,double>,double>;
using Cutoff_details=std::array<Cutoff_detail,MAX_DCMPS>;

#define RUN_RESULT_ITEMS(X)\
	X(std::vector<Output_tuple>,result)\
	X(Cutoff_details,cutoff_pr)\
	X(Cutoff_detail,cmp_cutoff_pr)\

#define INST(A,B) A B;

struct Run_result{
	RUN_RESULT_ITEMS(INST)
};

std::ostream& operator<<(std::ostream&,Run_result const&);

Run_result run_calc(Run_input);

#endif
