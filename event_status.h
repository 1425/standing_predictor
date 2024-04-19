#ifndef EVENT_STATUS_H
#define EVENT_STATUS_H

#include<iosfwd>
#include<map>

namespace tba{
	class Event_key;
	class Year;
}

class TBA_fetcher;

#define EVENT_STATUS_ITEMS(X)\
	X(FUTURE)\
	X(IN_PROGRESS)\
	X(COMPLETE)

enum class Event_status{
	#define X(A) A,
	EVENT_STATUS_ITEMS(X)
	#undef X
};

std::ostream& operator<<(std::ostream& o,Event_status);

Event_status event_status(TBA_fetcher&,tba::Event_key const&);
std::map<tba::Event_key,Event_status> event_status(TBA_fetcher&,tba::Year const&);

#endif
