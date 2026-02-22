#ifndef RANK_LIMITS_H
#define RANK_LIMITS_H

#include "pick_points.h"
#include "map_auto.h"
#include "probability.h"
#include "print_r.h"
#include "event_status.h"

class TBA_fetcher;

template<typename Team>
using Rank_range=map_auto<Team,Interval<Rank>>;

template<typename Team>
using Point_range=map_auto<Team,Interval<Point>>;

struct Qual_status_future{};

std::ostream& operator<<(std::ostream&,Qual_status_future);

struct Qual_status_in_progress{
	int matches_complete,matches_total;
};

std::ostream& operator<<(std::ostream&,Qual_status_in_progress const&);

struct Qual_status_complete{};

std::ostream& operator<<(std::ostream&,Qual_status_complete const&);

using Qual_status=std::variant<
	Qual_status_future,
	Qual_status_in_progress,
	Qual_status_complete
>;

#define QUAL_STATUS(X)\
	X(FUTURE)\
	X(COMPLETE)

#define RANK_RESULTS(X)\
	X(Rank_range<Team>,ranks)\
	X(Point_range<Team>,points)\
	X(unsigned,unclaimed_points)\
	X(Qual_status,status)\

template<typename Team>
struct Rank_results{
        RANK_RESULTS(INST)

	void check()const{
		auto a=check_rank_limits(ranks);
		assert(!a);

		auto s=sum(values(points));
		unsigned spread=s.max-s.min;
		assert(unclaimed_points<=spread);
	}
};

template<typename Team>
std::ostream& operator<<(std::ostream& o,Rank_results<Team> const& a){
        o<<"Rank_result( ";
        #define X(A,B) o<<""#B<<":"<<a.B<<" ";
        RANK_RESULTS(X)
        #undef X
        return o<<")";
}

template<typename Team>
void print_r(int n,Rank_results<Team> const& a){
        indent(n);
	std::cout<<"Rank_results\n";
        n++;
        #define X(A,B) indent(n); std::cout<<""#B<<"\n"; print_r(n+1,a.B);
        RANK_RESULTS(X)
        #undef X
}

Rank_results<tba::Team_key> rank_limits(TBA_fetcher&,tba::Event_key const&);

std::optional<std::map<tba::Team_key,Rank>> listed_ranks(TBA_fetcher&,tba::Event_key const&);

bool normal_ranking_expected(tba::Event_type);
bool normal_ranking_expected(tba::Event const&);

void rank_limits_demo(TBA_fetcher&);

template<template<typename,typename> class MAP,typename Team>
std::optional<std::string> check_rank_limits(MAP<Team,Interval<Rank>> const& a){
	//if string, it's an error

	auto v=values(a);

	if(v.empty()){
		return std::nullopt;
	}
	//first, check that all of the limits exists in the range expected for an event of this many teams
	auto overall_range=*or_all(v);
	const Interval<Rank> expected(1,v.size());
	if(!match(overall_range,expected)){
		/*print_r(a);
		PRINT(expected);
		PRINT(overall_range)*/
		return "out of range";
	}
	assert(match(overall_range,expected));

	//This counting the teams w/ each outcome range to try to avoid going O(N^3)
	//on the big "events" that are like all 2021 regional teams
	auto c=count(v);

	//second check that for each rank that should exist at this event, there is at least one team that can fill it
	for(auto rank:range_inclusive<Rank>(1u,v.size())){
		/*auto f=count_if([=](auto x){ return subset(rank,x); },v);
		if(f==0){
			return "unfilled slot";
		}*/
		//assert(f.size()>=1);
		auto found=[=](){
			for(auto [k,v]:c){
				if(subset(rank,k)){
					return 1;
				}
			}
			return 0;
		}();
		if(found==0){
			return "unfilled slot";
		}
	}

	//third: for any possible range of ranks, check that they number of teams that must exist in there
	//is possible (could be too many or too few)

	for(auto min:range_inclusive<Rank>(1,a.size())){
		for(auto max:range_inclusive<Rank>(min,a.size())){
			size_t size=max-min+1;
			Interval<Rank> i{min,max};
			/*auto always=count_if([=](auto x){ return subset(x,i); },v);
			assert(always<=size);

			auto possible=count_if([=](auto x){ return overlap(x,i); },v);
			assert(possible>=size);*/

			size_t always=0;
			for(auto [k,v]:c){
				if(subset(k,i)){
					always+=v;
				}
			}
			assert(always<=size);

			size_t possible=0;
			for(auto [k,v]:c){
				if(overlap(k,i)){
					possible+=v;
				}
			}
			assert(possible>=size);
		}
	}
	return std::nullopt;
}

template<template<typename,typename> typename MAP,typename Team>
std::optional<std::string> check_rank_limits_direct(MAP<Team,Rank> const& a){
	auto v=sorted(values(a));
	if(v==range_inclusive<Rank>(1,v.size())){
		return std::nullopt;
	}
	return "mismatch";
}

template<template<typename,typename> typename MAP,typename Team>
std::optional<std::string> check_rank_limits(MAP<Team,Rank> const& a){
	/*auto r1=check_rank_limits(map_values([](auto x){ return Interval(x); },a));
	auto r2=check_rank_limits_direct(a);
	if(!r1) assert(!r2);
	if(r1) assert(r2);
	return r1;*/
	return check_rank_limits_direct(a);
}

template<typename T>
auto check_rank_limits(std::optional<T> const& a){
	using E=decltype(check_rank_limits(*a));
	using R=std::optional<E>;
	if(a){
		return R(check_rank_limits(*a));
	}
	return R();
}

#endif
