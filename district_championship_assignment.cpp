#include "district_championship_assignment.h"
#include "ca.h"
#include "../tba/tba.h"
#include "tba.h"

using namespace std;

static map<tba::Team_key,std::optional<Dcmp_home>> calc_data(TBA_fetcher &f){
	map<tba::Team_key,std::optional<Dcmp_home>> r;
	for(auto const& team:teams(f)){
		r[team.key]=[&]()->Dcmp_home{
			if(team.state_prov!="California"){
				return 0;
			}
			return int(california_region(team));
		}();
	}
	return r;
}

std::optional<Dcmp_home> calc_dcmp_home(TBA_fetcher &fetcher,tba::Team_key const& team_key,tba::Year const& year){
	if(year==2021){
		return std::nullopt;
	}

	//Doing it this way to avoid asking API about each individual team.
	static map<tba::Team_key,std::optional<Dcmp_home>> data=calc_data(fetcher);
	return data[team_key];

	/*auto t=team(fetcher,team_key);
	if(t.state_prov!="California"){
		return 0;
	}
	auto c=california_region(t);
	return Dcmp_home(int(c));*/
}

