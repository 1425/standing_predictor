#ifndef EVENT_STATUS_H
#define EVENT_STATUS_H

#include<map>
#include "io.h"

namespace tba{
	class Event_key;
	class Year;
}

class TBA_fetcher;

#define EVENT_STATUS_ITEMS(X)\
	X(FUTURE)\
	X(IN_PROGRESS)\
	X(COMPLETE)

ENUM_CLASS(Event_status,EVENT_STATUS_ITEMS)

Event_status event_status(TBA_fetcher&,tba::Event_key const&);
std::map<tba::Event_key,Event_status> event_status(TBA_fetcher&,tba::Year const&);

int event_points_multiplier(TBA_fetcher&,tba::Event_key const&);

#endif
