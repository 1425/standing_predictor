#ifndef LOCK2_H
#define LOCK2_H

#include<map>

namespace tba{
	class Team_key;
	class District_key;
};

class TBA_fetcher;

int lock2_demo(TBA_fetcher&);

std::map<tba::Team_key,std::string> lock2(TBA_fetcher&,tba::District_key const&);

#endif
