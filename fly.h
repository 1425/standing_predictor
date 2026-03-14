#ifndef FLY_H
#define FLY_H

#include "../tba/data.h"

class TBA_fetcher;

using Days=std::chrono::duration<unsigned char,std::ratio<3600*24,1>>;

using Flight_info=std::map<tba::Team_key,std::tuple<tba::Date,Days,double>>;

Flight_info fly(TBA_fetcher&);

int fly_demo(TBA_fetcher&);

#endif
