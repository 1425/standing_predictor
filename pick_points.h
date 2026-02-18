#ifndef PICK_POINTS_H
#define PICK_POINTS_H

#include<set>
#include "../tba/data.h"
#include "int_limited.h"
#include "set_fixed.h"
#include "array.h"
#include "util.h"
#include "vector_void.h"
#include "interval.h"
#include "probability.h"

class TBA_fetcher;

//2016micmp has 106 teams
//2000cmp has 249 teams
//"202121reg" has 1477 teams
//very unfortunate that this could not at least stay under 1024 so you could pack 6 of them into 
//a 64-bit word.
static constexpr size_t MAX_TEAMS_PER_EVENT=1800;
using Rank=Int_limited<1,MAX_TEAMS_PER_EVENT>;

int pick_points_demo(TBA_fetcher&);

std::vector<tba::Team_key> teams(tba::Elimination_Alliance const&);

template<typename T,size_t N>
class set_limited;

template<size_t N>
set_limited<tba::Team_key,N> teams(set_limited<tba::Team_key,N> const&);

template<typename>
class Match;

template<typename Team>
set_limited<Team,6> teams(Match<Team> const&);

template<size_t N>
set_fixed<tba::Team_key,N> teams(set_fixed<tba::Team_key,N> const& a){
	return a;
}

template<size_t N>
auto teams(std::array<tba::Team_key,N> a){
	return std::move(a);
}

template<typename T,size_t N>
auto teams(std::array<T,N> const& a){
	return flatten(MAP(teams,a));
}

template<long long MIN,long long MAX>
auto teams(Int_limited<MIN,MAX> const&){
	return std::set<tba::Team_key>();
}

template<typename T>
auto teams(Interval<T> const& a){
	return teams(a.min)|teams(a.max);
}

template<typename T>
std::vector<tba::Team_key> teams(std::vector<T> const& a){
	return flatten(MAP(teams,a));
}

template<typename T>
auto teams(std::optional<T> const& a){
	if(!a){
		return std::vector<tba::Team_key>();
	}
	return teams(*a);
}

using Picked=std::map<tba::Team_key,Interval<bool>>;
using Pts=std::map<tba::Team_key,Interval<Point>>;

#define PICK_LIMITS(X)\
	X(Pts,points)\
	X(Picked,picked)\
	X(unsigned,unclaimed)

TBA_MAKE_INST(Pick_limits,PICK_LIMITS)

Pick_limits pick_limits(TBA_fetcher&,tba::Event_key const&,std::map<tba::Team_key,Interval<Rank>> const&);

#endif
