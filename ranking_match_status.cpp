#include "ranking_match_status.h"
#include "set_limited.h"

Ranking_match_status<tba::Team_key> ranking_match_status(TBA_fetcher &f,tba::Event_key const& event){
	//Note that at the moment this doesn't have incomplete events to look at
	//so it's only half tested.

	/* Also, it seems like there are quite a few matches where it doesn't understand
	 * what is going on already - so that would be good to work out.
	 *
	 * also, should look at events that have no matches listed that are in the future and fill in that
	 * everyone is effectively tied at 0.
	 * */

	Ranking_match_status<tba::Team_key> r(year(event));
	r.matches_completed=0;

	for(auto match:tba::event_matches(f,event)){
		if(match.comp_level!=tba::Competition_level::qm){
			continue;
		}

		auto f=[](auto x){
			auto found=to_set(x.team_keys)-to_set(x.surrogate_team_keys);
			if(found.size()>3){
				//this is invalid; skip the match
				//might want to note that this happened somehow
				//does actually occur in the data.  Specifically at
				//2024gaalb
			}
			//return Alliance(take(3,found));
			return Alliance(take<3>(found));
		};
		//match.alliances.red.team_keys/surrogate_team_keys
		auto m1=f(match.alliances.red);
		auto m2=f(match.alliances.blue);
		if( (m1&m2).size() ){
			//invalid match; skipping
			//might want to note this to caller or something.
			continue;
		}
		Match<tba::Team_key> match1({m1,m2});
		//r.schedule|=match1;

		auto rp_totals_m=rp(match);
		if(!rp_totals_m){
			//cout<<"RP? "<<match.key<<"\n";
			r.schedule|=match1;
		}else{
			r.matches_completed=1;
			auto rp_totals=*rp_totals_m;
			for(auto [teams,pts]:zip(match1,rp_totals)){
				for(auto team:teams){
					r.standings[team]+=pts;
				}
			}
		}
	}
	r.fill_standings(f,event);
	return r;
}


