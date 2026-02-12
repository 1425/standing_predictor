#ifndef RP_H
#define RP_H

#include<optional>
#include<array>
#include "int_limited.h"

namespace tba{
	class Year;
	class Match;
};

class TBA_fetcher;

//actual max here is something like 72.
using RP=Int_limited<0,200>;

std::optional<std::array<RP,2>> rp(tba::Match const&);

RP max_rp_per_match(tba::Year const&);

void rp_distribution(TBA_fetcher&);

#endif
