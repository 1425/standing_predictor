#ifndef TBA_H
#define TBA_H

#include<set>
#include "../tba/data.h"
#include "../tba/db.h"
#include "output.h"

tba::Cached_fetcher get_tba_fetcher(std::string const& auth_key_path,std::string const& cache_path);

std::set<tba::Team_key> chairmans_winners(tba::Cached_fetcher&,tba::District_key const&);
std::map<Point,Pr> dcmp_distribution(tba::Cached_fetcher&);
std::map<Point,Pr> historical_event_pts(tba::Cached_fetcher&);

#endif
