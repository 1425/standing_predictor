#ifndef RUN_H
#define RUN_H

#include "output.h"
#include "flat_map2.h"
#include "outline.h"
#include "output.h"

flat_map2<Point,Pr> convolve(flat_map2<Point,Pr> const&,flat_map2<Point,Pr> const&);

//using Result_tuple=std::tuple<tba::Team_key,Pr,Point,Point,Point,Pr,Point,Point,Point>;

using Team_dist=flat_map2<Point,Pr>;

struct Team_status{
	bool district_chairmans;
	Team_dist point_dist; //number of points expected pre-dcmp
	Dcmp_home dcmp_home;
	Point already_earned;
};

std::ostream& operator<<(std::ostream&,Team_status const&);

struct Run_result{
	std::vector<Output_tuple> result;
	using Cutoff=flat_map2<std::pair<Point,double>,double>;
	std::array<Cutoff,MAX_DCMPS> cutoff_pr;
	Cutoff cmp_cutoff_pr;
};

using By_team=std::map<tba::Team_key,Team_status>;

struct Run_input{
	std::vector<int> dcmp_size;
	int worlds_slots;

	By_team by_team;

	bool dcmp_played;
	std::map<Point,Team_dist> dcmp_distribution1;
};

Run_result run_calc(Run_input);

#endif
