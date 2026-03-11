#ifndef STATUS_H
#define STATUS_H

#include<set>

namespace tba{
	class Team_key;
	class Event_key;
}

class TBA_fetcher;

int demo(TBA_fetcher&);

std::set<tba::Team_key> chairmans(TBA_fetcher&,tba::Event_key const&);

#endif
