#ifndef LOCK_H
#define LOCK_H

#include<optional>

namespace tba{
	class Year;
	class District_key;
};

class TBA_fetcher;

int lock_demo(TBA_fetcher&);

int run_lock(TBA_fetcher&,tba::Year const&,std::optional<tba::District_key> const&);

#endif
