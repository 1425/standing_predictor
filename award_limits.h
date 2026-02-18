#ifndef AWARD_LIMITS_H
#define AWARD_LIMITS_H

#include "rank_limits.h"
#include "util.h"

class TBA_fetcher;

namespace tba{
	class Team_key;
	class Event_key;
};

//first is # of chairmans awards still to be won
//second is how many district points there are to be won
using Rank_value=std::pair<Int_limited<0,255>,Point>;

using Team_rank_value=std::map<tba::Team_key,Interval<Rank_value>>;

#define RANK_STATUS(X)\
	X(Team_rank_value,by_team)\
	X(Rank_value,unclaimed)\

struct Rank_status{
	RANK_STATUS(INST)

	Rank_status& operator+=(Rank_status const&);
};

std::ostream& operator<<(std::ostream&,Rank_status const&);

//The set of teams passed in will make it give a rating to each of those even if it doesn't
//know that they exist for any other reason.
Rank_status award_limits(TBA_fetcher&,tba::Event_key const&,std::set<tba::Team_key> const&);

int award_limits_demo(TBA_fetcher&);

#endif
