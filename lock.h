#ifndef LOCK_H
#define LOCK_H

#include<optional>
#include<map>
#include<string>

namespace tba{
	class Year;
	class District_key;
	class Team_key;
};

class TBA_fetcher;

int lock_demo(TBA_fetcher&);

std::map<tba::Team_key,std::string> lock(TBA_fetcher&,tba::District_key const&);

int run_lock(TBA_fetcher&,tba::Year const&,std::optional<tba::District_key> const&);

#endif
