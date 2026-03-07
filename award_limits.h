#ifndef AWARD_LIMITS_H
#define AWARD_LIMITS_H

#include "rank_limits.h"
#include "util.h"
#include "rand.h"

class TBA_fetcher;

namespace tba{
	class Team_key;
	class Event_key;
};

//first is # of chairmans awards still to be won
//second is how many district points there are to be won
using Rank_value=std::pair<Int_limited<0,255>,Point>;

double entropy(Interval<Rank_value> const&);

template<template<typename,typename>typename MAP=std::map>
using Team_rank_value=MAP<tba::Team_key,Interval<Rank_value>>;

template<template<typename,typename>typename MAP,typename T>
auto teams(MAP<tba::Team_key,Interval<T>> const& a){
	return keys(a)|teams(values(a));
}

#define RANK_STATUS(X)\
	X(Team_rank_value<MAP>,by_team)\
	X(Rank_value,unclaimed)\
	X(Status,status)\

template<typename Status,template<typename,typename>typename MAP=std::map>
struct Rank_status{
	RANK_STATUS(INST)

	template<template<typename,typename>typename MAP2>
	operator Rank_status<Status,MAP2>()const{
		return Rank_status<Status,MAP2>{
			#define X(A,B) B,
			RANK_STATUS(X)
			#undef X
		};
	}

	Rank_status& operator+=(Rank_status const&);

	auto operator<=>(Rank_status const&)const=default;
};

template<typename Status,template<typename,typename> typename MAP>
ELEMENTWISE_RAND(Rank_status<TBA_SINGLE_ARG(Status,MAP)>,RANK_STATUS)

template<typename Status,template<typename,typename> typename MAP>
PRINT_STRUCT(Rank_status<TBA_SINGLE_ARG(Status,MAP)>,RANK_STATUS)

template<typename Status,template<typename,typename>typename MAP>
void print_r(int n,Rank_status<Status,MAP> const& a){
	indent(n);
	std::cout<<"Rank_status\n";
	n++;

	indent(n);
	std::cout<<"by_team\n";
	print_r(n+1,a.by_team);

	indent(n);
	std::cout<<"unclaimed\n";
	print_r(n+1,a.unclaimed);

	indent(n);
	std::cout<<"status\n";
	print_r(n+1,a.status);
}

template<typename Status,template<typename,typename>typename MAP>
double entropy(Rank_status<Status,MAP> const& a){
	return sum(MAP(entropy,values(a.by_team)));
}

template<typename Status,template<typename,typename>typename MAP>
set_flat<tba::Team_key> teams(Rank_status<Status,MAP> const& a){
	return teams(a.by_team);
}

//The set of teams passed in will make it give a rating to each of those even if it doesn't
//know that they exist for any other reason.
Rank_status<Event_status> award_limits(TBA_fetcher&,tba::Event_key const&,std::set<tba::Team_key> const&,bool normal);

int award_limits_demo(TBA_fetcher&);

#endif
