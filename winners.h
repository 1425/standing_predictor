#ifndef WINNERS_H
#define WINNERS_H

#include<set>

namespace tba{
	class Team_key;
	class Event_key;
}

class TBA_fetcher;

std::set<tba::Team_key> winners(TBA_fetcher&,tba::Event_key const&);
int winners_demo(TBA_fetcher&);

#endif
