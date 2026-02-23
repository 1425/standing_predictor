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

double entropy(Interval<Rank_value> const&);

using Team_rank_value=std::map<tba::Team_key,Interval<Rank_value>>;

#define RANK_STATUS(X)\
	X(Team_rank_value,by_team)\
	X(Rank_value,unclaimed)\
	X(Status,status)\

template<typename Status>
struct Rank_status{
	RANK_STATUS(INST)

	Rank_status& operator+=(Rank_status const&);
};

template<typename T>
Rank_status<T> rand(Rank_status<T> const*){
	return Rank_status<T>{
		rand((Team_rank_value*)0),
		rand((Rank_value*)0),
		rand((T*)0)
	};
}

//TODO: Move to util.
#define PRINT_STRUCT_INNER(A,B) o<<""#B<<":"<<a.B<<" ";

#define PRINT_STRUCT(NAME,ITEMS)\
        std::ostream& operator<<(std::ostream& o,NAME const& a){\
                o<<""#NAME<<"( ";\
                ITEMS(PRINT_STRUCT_INNER)\
                return o<<")";\
        }

template<typename Status>
PRINT_STRUCT(Rank_status<Status>,RANK_STATUS)

template<typename Status> 
double entropy(Rank_status<Status> const& a){
	return sum(MAP(entropy,values(a.by_team)));
}

//The set of teams passed in will make it give a rating to each of those even if it doesn't
//know that they exist for any other reason.
Rank_status<Event_status> award_limits(TBA_fetcher&,tba::Event_key const&,std::set<tba::Team_key> const&);

int award_limits_demo(TBA_fetcher&);

#endif
