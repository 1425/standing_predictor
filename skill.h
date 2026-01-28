#ifndef SKILL_H
#define SKILL_H

#include "run.h"

//First item in return is pre-dcmp point distribution
//Second item in return is map from (# of pre-dcmp points) to distribution of pts at dcmp

struct Skill_estimates{
	std::map<tba::Team_key,Team_dist> pre_dcmp;
	std::map<Point,Team_dist> at_dcmp;
	std::map<Point,Team_dist> second_event;
};

Skill_estimates calc_skill(TBA_fetcher&,tba::District_key const&);

#define SKILL_METHOD(X)\
	X(POINTS)\
	X(OPR)\
	X(NONE)

enum class Skill_method{
	#define X(A) A,
	SKILL_METHOD(X)
	#undef X
};

std::ostream& operator<<(std::ostream&,Skill_method);

Skill_method decode(std::span<char*>,Skill_method const*);

Skill_estimates skill_estimates(TBA_fetcher&,tba::District_key const&,Skill_method);

#endif
