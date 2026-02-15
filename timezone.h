#ifndef TIMEZONE_H
#define TIMEZONE_H

#include<chrono>

namespace tba{
	struct Event;
};

class TBA_fetcher;

//Offset to UTC
std::chrono::hours get_timezone(tba::Event const&);

int timezone_demo(TBA_fetcher&);

#endif
