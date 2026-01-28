#ifndef OUTLINE_H
#define OUTLINE_H

#include "output.h"
#include "flat_map.h"
#include "ca.h"

Dcmp_home calc_dcmp_home(TBA_fetcher&,tba::Team_key const&);
California_region california_region(tba::Team const&);

#endif
