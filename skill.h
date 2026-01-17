#ifndef SKILL_H
#define SKILL_H

#include "run.h"

std::map<tba::Team_key,Team_dist> calc_skill(TBA_fetcher&,tba::District_key const&);

#endif
