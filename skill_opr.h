#ifndef SKILL_OPR_H
#define SKILL_OPR_H

#include<set>
#include "skill.h"

Skill_estimates calc_skill_opr(TBA_fetcher&,tba::District_key const&);

std::vector<tba::Year> years();
std::map<tba::District_abbreviation,std::set<tba::Year>> normal_district_years(TBA_fetcher&);

#endif
