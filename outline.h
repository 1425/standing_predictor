#ifndef OUTLINE_H
#define OUTLINE_H

#include "tba.h"
#include "skill.h"

struct Args{
	std::string output_dir=".";
	std::optional<tba::Year> year;
	std::optional<tba::District_key> district;
	TBA_fetcher_config tba;
	bool demo=0,rank_limits_demo=0;
	bool award_limits_demo=0;
	bool playoff_limits_demo=0;
	bool timezone_demo=0;
	bool lock=0;
	bool venue_demo=0;
	bool event_limits_demo=0;
	bool event_partial_demo=0;
	bool data_range_demo=0;
	bool plot=1;
	bool quick=0;
	Skill_method skill_method=Skill_method::POINTS;
	std::optional<bool> index;
	bool history_demo=0;
	bool fly=0;
	bool plot_demo=0;
};

void run_outer(TBA_fetcher&,Args);

#endif
