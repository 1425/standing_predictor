#ifndef SKILL_H
#define SKILL_H

#include "run.h"

//First item in return is pre-dcmp point distribution
//Second item in return is map from (# of pre-dcmp points) to distribution of pts at dcmp
std::pair<std::map<tba::Team_key,Team_dist>,std::map<Point,Team_dist>> calc_skill(TBA_fetcher&,tba::District_key const&);

#endif
