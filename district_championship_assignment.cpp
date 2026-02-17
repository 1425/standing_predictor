#include "district_championship_assignment.h"
#include "ca.h"
#include "../tba/tba.h"
#include "tba.h"

Dcmp_home calc_dcmp_home(TBA_fetcher &fetcher,tba::Team_key const& team_key){
	auto t=team(fetcher,team_key);
	if(t.state_prov!="California"){
		return 0;
	}
	auto c=california_region(t);
	return Dcmp_home(int(c));
}

