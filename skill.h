#ifndef SKILL_H
#define SKILL_H

#include "probability.h"

namespace tba{
	class Team_key;
	class District_key;
}

class TBA_fetcher;

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

using Map_team_dist=std::map<tba::Team_key,Team_dist>;
using Map_point_dist=std::map<Point,Team_dist>;

#define SKILL_ESTIMATES(X)\
	X(Map_team_dist,pre_dcmp)\
	X(Map_point_dist,at_dcmp)\
	X(Map_point_dist,second_event)

struct Skill_estimates{
	SKILL_ESTIMATES(INST)
};

std::ostream& operator<<(std::ostream&,Skill_estimates const&);

Skill_estimates calc_skill(TBA_fetcher&,tba::District_key const&);
Skill_estimates skill_estimates(TBA_fetcher&,tba::District_key const&,Skill_method);

template<typename K>
std::array<K,5> quartiles(flat_map2<K,Pr> a){
	assert(!a.empty());
	std::array<K,5> r;
	r[0]=a.begin()->first;
	r[4]=(a.end()-1)->first;

	Pr total=0;
	auto it=a.begin();
	while(it!=a.end() && total<.25){
		total+=it->second;
		++it;
	}
	if(it!=a.end()){
		r[1]=it->first;
	}else{
		r[1]=r[4];
	}

	while(it!=a.end() && total<.5){
		total+=it->second;
		++it;
	}
	if(it!=a.end()){
		r[2]=it->first;
	}else{
		r[2]=r[4];
	}
	while(it!=a.end() && total<.75){
		total+=it->second;
		++it;
	}
	if(it!=a.end()){
		r[3]=it->first;
	}else{
		r[3]=r[4];
	}
	return r;
}

#endif
