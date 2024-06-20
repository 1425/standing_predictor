#ifndef RUN_H
#define RUN_H

#include "../tba/data.h"
#include "output.h"
#include "flat_map2.h"

flat_map2<Point,Pr> convolve(flat_map2<Point,Pr> const&,flat_map2<Point,Pr> const&);

using Result_tuple=std::tuple<tba::Team_key,Pr,Point,Point,Point,Pr,Point,Point,Point>;

using Team_dist=flat_map2<Point,Pr>;

struct Run_result{
	std::vector<Result_tuple> result;
	flat_map2<std::pair<Point,double>,double> cutoff_pr,cmp_cutoff_pr;
	std::map<tba::Team_key,std::tuple<std::vector<int>,int,int>> points_used;
	std::map<tba::Team_key,std::pair<bool,Team_dist>> by_team;
};

struct Run_input{
	int dcmp_size;
	int worlds_slots;

	/*for each team, did they win a district chairmans's award and how
	many points are they expected to have by the time of the district
	championship
	*/
	std::map<tba::Team_key,std::pair<bool,Team_dist>> by_team;

	bool dcmp_played;
	flat_map2<Point,Pr> dcmp_distribution1;
	std::vector<tba::District_Ranking> d1;
	std::map<tba::Team_key,std::tuple<std::vector<int>,int,int>> points_used; //this is only passed through
};

Run_result run_calc(Run_input);

#endif
