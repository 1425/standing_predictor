#ifndef DATES_H
#define DATES_H

#include<map>
#include "io.h"
#include "../tba/data.h"

class TBA_fetcher;

using Day=int;

//From # of scheduled days to what time matches are expected on each day.
using Dates_result=std::map<
	Day,
	std::map<Day,std::pair<Time_ns,Time_ns>>
>;

Dates_result event_times(TBA_fetcher&);

int dates_demo(TBA_fetcher&);

std::vector<tba::Event> all_events(TBA_fetcher&);
std::vector<tba::Team> teams_year_all(TBA_fetcher&,tba::Year);

#endif
