#ifndef RANK_LIMITS_H
#define RANK_LIMITS_H

#include "pick_points.h"
#include "map_auto.h"
#include "output.h"
#include "print_r.h"

class TBA_fetcher;

template<typename Team>
using Rank_range=map_auto<Team,Interval<Rank>>;

template<typename Team>
using Point_range=map_auto<Team,Interval<Point>>;

#define RANK_RESULTS(X)\
        X(Rank_range<Team>,ranks)\
        X(Point_range<Team>,points)\
        X(unsigned,unclaimed_points)

template<typename Team>
struct Rank_results{
        RANK_RESULTS(INST)
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

void rank_limits_demo(TBA_fetcher&);

#endif
