#ifndef VENUE_H
#define VENUE_H

#include<cstddef>
#include<string>

namespace tba{
	class Event_key;
	struct Event;
};

class TBA_fetcher;

int venue_demo(TBA_fetcher&);

size_t event_size(TBA_fetcher&,tba::Event_key const&);

std::string nice_name(tba::Event const&);

#endif
