#ifndef TABLE_H
#define TABLE_H

#include "annotated_complex.h"

struct Skill_estimates;

std::string as_table(Skill_estimates const&);
std::string as_table(Annotated const&);

#endif
