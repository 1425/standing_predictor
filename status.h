#ifndef STATUS_H
#define STATUS_H

#include<set>
#include "../tba/data.h"

class TBA_fetcher;

int demo(TBA_fetcher&);

std::set<tba::Team_key> chairmans(TBA_fetcher &tba_fetcher,tba::Event_key event_key);

#endif
