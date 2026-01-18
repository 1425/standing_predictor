#ifndef RUN_H
#define RUN_H

#include "../tba/data.h"
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
};

std::ostream& operator<<(std::ostream&,Team_status const&);

struct Run_result{
	std::vector<Output_tuple> result;
	using Cutoff=flat_map2<std::pair<Point,double>,double>;
	std::array<Cutoff,MAX_DCMPS> cutoff_pr;
	Cutoff cmp_cutoff_pr;
	std::map<tba::Team_key,std::tuple<std::vector<int>,int,int>> points_used;
	std::map<tba::Team_key,Team_status> by_team;
};

struct Run_input{
	std::vector<int> dcmp_size;
	int worlds_slots;

	/*for each team, did they win a district chairmans's award and how
	many points are they expected to have by the time of the district
	championship
	*/
	//std::map<tba::Team_key,std::pair<bool,Team_dist>> by_team;
	std::map<tba::Team_key,Team_status> by_team;

	bool dcmp_played;
	std::map<Point,Team_dist> dcmp_distribution1;
	std::vector<tba::District_Ranking> d1;
	std::map<tba::Team_key,std::tuple<std::vector<int>,int,int>> points_used; //this is only passed through
};

Run_result run_calc(Run_input);

#endif
