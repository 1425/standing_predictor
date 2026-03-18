#ifndef QUERY_H
#define QUERY_H

#include<set>
#include<map>
#include "probability.h"

namespace tba{
	class Team_key;
	struct Match;
}

class TBA_fetcher;

std::set<tba::Team_key> chairmans_winners(TBA_fetcher&,tba::District_key const&);
std::map<Point,Pr> dcmp_distribution(TBA_fetcher&);
std::map<Point,Pr> historical_event_pts(TBA_fetcher&);

tba::Year year(tba::District_key const&);
tba::Year year(tba::Event_key const&);
tba::Year year(tba::Event const&);

//tba::Team_key rand(tba::Team_key const*);
//tba::Event_key rand(tba::Event_key const*);

bool chairmans_expected(tba::Event_type);
bool chairmans_expected(TBA_fetcher&,tba::Event_key const&);

bool complete(tba::Match const&);
bool matches_complete(TBA_fetcher &,tba::Event_key const&);

bool won_chairmans(TBA_fetcher &,tba::Year,tba::Team_key const&);
bool event_timed_out(TBA_fetcher &,tba::Event_key const&);

std::vector<tba::Event> events(TBA_fetcher&);
std::vector<tba::Event_key> events_keys(TBA_fetcher&);
std::vector<tba::Event> events(TBA_fetcher&,tba::District_key const&);
std::vector<tba::Event_key> events_keys(TBA_fetcher&,tba::District_key const&);

std::vector<tba::Year> years();
std::vector<tba::Team> teams(TBA_fetcher&);
std::vector<tba::Team> teams(TBA_fetcher&,tba::Year);
std::vector<tba::Team_key> teams_keys(TBA_fetcher&,tba::Event_key const&);
std::vector<tba::Team_key> teams_keys(TBA_fetcher&,tba::Event const&);
std::vector<tba::District_key> districts(TBA_fetcher&);

std::vector<tba::Match> playoff_matches(TBA_fetcher&,tba::Event_key const&);
std::vector<tba::Match_Simple> playoff_matches_simple(TBA_fetcher&,tba::Event_key const&);

bool playoffs_started(TBA_fetcher&,tba::Event_key const&);
bool awards_done(TBA_fetcher&,tba::Event_key const&);
std::optional<tba::District_key> district(TBA_fetcher &,tba::Event_key const&);

bool complete(TBA_fetcher &,tba::Event_key const&);
tba::Event_type event_type(TBA_fetcher &f,tba::Event_key const&);
tba::Event_type event_type(tba::Event const&);
tba::Event_type event_type(TBA_fetcher&,tba::Event_points const&);

std::string link(tba::Event_key const&,std::string const&);
std::string link(tba::Event const&,std::string const&);

template<typename T,size_t N>
set_limited<T,N> to_set(tba::vector_fixed<T,N> const& a){
	set_limited<T,N> r;
	for(auto const& x:a){
		r|=x;
	}
	return r;
}

tba::Year current_season(TBA_fetcher&);
std::optional<tba::District_key> district(TBA_fetcher&,tba::Team_key const&,tba::Year const&);
tba::Date cmp_start(TBA_fetcher&,tba::Year);
tba::Date dcmp_start(TBA_fetcher&,tba::District_key const&);
std::optional<tba::Date> dcmp_start(TBA_fetcher&,std::optional<tba::District_key> const&);
tba::Date dcmp_end(TBA_fetcher&,tba::District_key const&);

#endif
