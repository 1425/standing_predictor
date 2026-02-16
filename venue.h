#ifndef VENUE_H
#define VENUE_H

#include<cstddef>

namespace tba{ class Event_key; };

class TBA_fetcher;

int venue_demo(TBA_fetcher&);

size_t event_size(TBA_fetcher&,tba::Event_key const&);

#endif
