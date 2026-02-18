#include "playoff_limits.h"
#include "../tba/tba.h"
#include "interval.h"
#include "print_r.h"
#include "tba.h"
#include "vector_void.h"
#include "pick_points.h"

template<typename T>
bool disjoint(std::vector<std::set<T>> const& a){
	(void)a;
	nyi
}

template<typename T>
bool disjoint(std::vector<std::vector<T>> const& a){
	auto f=flatten(a);
	return to_set(f).size()==f.size();
}

/*
 * this is O(N*log(N)+M*log(M)+N*log(M))
 * basic impl is O(N*M)
 * template<typename T>
bool subset(std::vector<T> const& a,std::vector<T> const& b){
	auto s1=to_set(a);
	auto s2=to_set(b);
	return subset(s1,s2);
}*/

using namespace std;
using Team=tba::Team_key;
using Team_key=tba::Team_key;

std::ostream& operator<<(std::ostream& o,Playoff_limits const& a){
	o<<"Playoff_limits( ";
#define X(A,B) o<<""#B<<":"<<a.B<<" ";
	PLAYOFF_LIMITS(X)
#undef X
	return o<<")";
}

auto alliances(tba::Match const& a){
	auto f=[](auto x){ return x.team_keys; };
	return array{
		f(a.alliances.red),
		f(a.alliances.blue)
	};
}

auto alliances(tba::Match_Simple const& a){
	auto f=[](auto x){ return x.team_keys; };
	return array{
		f(a.alliances.red),
		f(a.alliances.blue)
	};
}

//auto alliances(vector<tba::Match> const& a){
template<typename T>
auto alliances(vector<T> const& a){
	return to_set(flatten(MAP(alliances,a)));
}

auto key(tba::Event const& a){
	return a.key;
}

bool playoff(tba::Competition_level a){
	return a!=tba::Competition_level::qm;
}

auto playoff_matches(TBA_fetcher &f,tba::Event_key const& event){
	return filter(
		[](auto x){ return playoff(x.comp_level); },
		tba::event_matches(f,event)
	);
}

auto playoff_matches_simple(TBA_fetcher &f,tba::Event_key const& event){
	return filter(
		[](auto x){ return playoff(x.comp_level); },
		tba::event_matches_simple(f,event)
	);
}

Playoff_limits playoff_limits(TBA_fetcher&,std::map<Team_key,Interval<bool>> const& a){
	auto s=sum(values(a));
	(void)s;//at some point might want to change how this sum works to seperate the halves and do two sums.
	//PRINT(s);
	if(s.min!=s.max){
		//if it looks like it's impossible to have a 24-team playoff, something went wrong earlier...
		//this can happen at things like an Einstein field.
		//or 2018oncmp
		/*if(!subset(24u,s)){
			print_r(a);
			PRINT(s);
		}
		assert(subset(24u,s));*/
	}
	//assert(subset(24,s)); //if it's not possible to have 24 teams, then this is not a case we're 
			      //going to handle yet

	//going to start by just doing the dumbest thing and ignore all the match results
	//this can be refined later.
	Playoff_limits r;
	r.by_team=to_map_auto(map_values([&](auto v){
		if(v.max){
			return Interval<Point>{0,30};
		}else{
			return Interval<Point>{0,0};
		}
	},a));
	std::vector<int> finish_points;
	for(auto x:{30,20,10,10}){
		for(auto _:range(3)){
			finish_points|=x;
		}
	}
	//r.unclaimed_points=3*(30+20+10*2);
	r.unclaimed_points=sum(take(a.size(),finish_points));
	return r;
}

std::variant<vector<vector<Team>>,std::string> listed_alliances(TBA_fetcher& f,tba::Event_key const& event){
	auto e=tba::event_alliances(f,event);
	if(!e){
		return "no data";
	}

	auto found=MAP(teams,*e);

	//now do some sanity checking
	
	if(found.size()!=8){
		//not necessarily invalid, but not a case that we're looking for immediately.
		return "wrong number of alliances";
	}
	assert(found.size()==8);

	for(auto const& alliance:found){
		if(alliance.size()==0){
			//don't have a complete alliance
			return "incomplete";
		}
		//PRINT(alliance.size());
		assert(alliance.size()>=3 && alliance.size()<=4);
	}

	if(!disjoint(found)){
		//print_r(found);
		//print_r(count(flatten(found)));
		return "duplicates";
	}
	assert(disjoint(found));

	return found;
}

bool match(std::vector<std::vector<Team>> const& a,std::set<vector<Team>> const& b){
	auto s1=MAP(to_set,a);
	auto s2=MAP(to_set,b);
	return to_set(s1)==s2;
	nyi
}

std::optional<string> playoff_limits_demo(TBA_fetcher& f,tba::Event_key const& event){
	//PRINT(event);
	auto al0=listed_alliances(f,event);
	if(al0.index()){
		//in the future might want to handle this case.
		return "allainces unknown:"+get<1>(al0);
	}
	auto al=get<0>(al0);
	auto all_teams=event_teams_keys(f,event);
	auto matches=playoff_matches_simple(f,event);
	auto seen_alliances=alliances(matches);
	
	auto find_alliance=[=](auto const& in_match){
		for(auto teams:al){
			if(subset(in_match,teams)){
				return 1;
			}
		}
		return 0;
	};

	//print_lines(enumerate(a));

	for(auto const& in_match:seen_alliances){
		auto f=find_alliance(in_match);
		if(!f){
			/*cout<<"Looking for:"<<in_match<<"\n";
			cout<<"Options:\n";
			print_r(*al);*/
			return "teams seen in matches don't match alliance selection";
		}
		//assert(f);
	}

	//should also check that each of the alliances appears in at least one playoff match
	for(auto const& alliance:al){
		auto f=filter([=](auto x){ return subset(x,alliance); },seen_alliances);
		if(f.empty()){
			//obviously this will not be a problem when we start dealing w/ in-progress events.
			return "alliance without matches";
			PRINT(event);
			PRINT(alliance);
			PRINT(f);
			PRINT(f.size());
			PRINT(seen_alliances);
			PRINT(seen_alliances.size());
		}
		assert(f.size()>=1);
	}

	//assert(match(*al,a));

	//check that listed alliances are consistent w/ the list of teams at the event
	//check that teams listed in elimination matches is consistent with listed in alliances
	//and could check that the alliances appear in the matches that they're supposed to

	/*
	 * basic ideas:
	 * 1) figure out how many points have already been earned
	 * 2) if you have been picked you're stuck on the alliance
	 * 	and you get however many pts that alliance gets (unless backup involved)
	 * 	if team is not on an alliance, then could get on the team that's going to win the rest
	 * */

	//also need to know all the teams
	//because one could come in as a backup bot at any time
	//even before the first match and get the full 30 points
	return std::nullopt;
}

int playoff_limits_demo(TBA_fetcher& f){
	//TODO: Loop through all the events that are expected to be good.
	playoff_limits_demo(f,tba::Event_key("2025orwil"));

	/*for(auto event:events(f)){
		playoff_limits_demo(f,event.key);
	}*/

	auto e=take(1000,events(f));
	auto k=MAP(key,e);
	auto g=group(
		[&](auto x){ return playoff_limits_demo(f,x); },
		k
	);
	for(auto [k,v]:g){
		cout<<k<<"\t"<<v.size()<<"\n";
	}
	//print_r(g);

	/*two different modes to look at:
	 * 1) limits on how many points could be earned given that we know the alliances
	 * 2) expand to allow for backup robots
	 * 3) allow uncertainty in who is on what alliance
	 *
	 * also, the points here change based on the rules for the year
	 * to start with, going to use the "5 pts per playoff match one in successful series"?
	 * or the current pts based on how far you get in double elimination?
	 *
	 * going to start with assuming that none of the matches have been played yet
	 * might want to look at the listing of which events we know have problems with their 
	 * alliance listings.
	 * */
	return 0;
}
