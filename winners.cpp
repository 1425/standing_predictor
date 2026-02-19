#include "winners.h"
#include "../tba/tba.h"
#include "util.h"
#include "pick_points.h"
#include "tba.h"
#include "event_status.h"

using namespace std;
using Team=tba::Team_key;
using Event=tba::Event_key;

set<Team> winners_awards(TBA_fetcher &f,Event const& a){
	auto found=filter(
		[](auto x){ return x.award_type==tba::Award_type::WINNER; },
		event_awards(f,a)
	);
	return to_set(teams(found));
}

bool won(tba::M_Elimination_Alliance_status const&);

template<typename T>
bool won(std::optional<T> const& a){
	if(!a){
		return 0;
	}
	return won(*a);
}

bool won(tba::Elimination_Alliance_status const& a){
	return a.status=="won";
}

bool won(tba::M_Elimination_Alliance_status const& a){
	if(std::holds_alternative<tba::Unknown>(a)){
		nyi
	}
	if(std::holds_alternative<tba::Elimination_Alliance_status>(a)){
		return won(std::get<tba::Elimination_Alliance_status>(a));
	}
	assert(0);
}

bool won(tba::Elimination_Alliance const& a){
	if(!a.status){
		return 0;
	}
	return won(a.status);
}

set<Team> winners_alliance(TBA_fetcher &f,Event const& a){
	//could make the won() things differential between yes/no/maybe not just yes and don't know
	auto e=event_alliances(f,a);
	if(!e){
		return set<Team>();
	}

	auto found=FILTER(won,*e);
	if(found.size()>1){
		//error, so say that we don't know.
		return set<Team>();
	}
	return to_set(flatten(mapf([](auto x){ return x.picks; },found)));
}

set<Team> finish_district_points(TBA_fetcher &f,Event const& a,int target){
	//could make this distinguish between no data available vs no winners found...
	//also, this will give a subset of the winners because teams that get substituted in as a backup
	//will not get the full set of points.

	auto d=district(f,a);
	if(!d){
		return set<Team>();
	}

	auto d2=tba::district_rankings(f,*d);
	if(!d2){
		return set<Team>{};
	}

	set<Team> r;
	for(auto team_info:*d2){
		for(auto event:team_info.event_points){
			if(event.event_key!=a){
				continue;
			}
			if(event.elim_points==event_points_multiplier(f,a)*target){
				r|=team_info.team_key;
			}
		}
	}
	return r;
}

set<Team> winners_district_points(TBA_fetcher &f,Event const& a){
	return finish_district_points(f,a,30);
}

std::set<Team> winners(TBA_fetcher &f,Event const& e){
	auto a=winners_awards(f,e);
	auto b=winners_alliance(f,e);
	auto c=winners_district_points(f,e);

	if(b.empty() && c.empty()){
		return a;
	}

	if(b.empty() && a==c){
		return a;
	}

	if(b.empty() && subset(c,a)){
		return a;
	}

	if(a==b && c.empty()){
		return a;
	}

	if(a.empty() && c.empty()){
		//2010iri gets here.
		return b;
	}

	if(c.empty() && subset(b,a)){
		//can get here via 2010ny
		return a;
	}

	if(a==b && subset(c,a)){
		//happens at 2011oc
		return a;
	}

	if(b==c && b.size()==3 && subset(b,a)){
		//for 2024txama
		return b;
	}

	if(subset(c,a) && subset(b,a)){
		//2012migl
		return a;
	}

	if(c.empty() && !subset(a,b) && !subset(b,a)){
		//2014nyli: data is inconsistent, and alliance data is wrong.
		return a;
	}

	bool ok=(a==b && a==c);
	if(ok){
		return a;
	}

	//when events are actually in progress, we may get here.
	if(!ok){
		PRINT(e);
		PRINT(a)
		PRINT(b)
		PRINT(c)
	}

	assert(a==b);
	assert(b==c);

	return a;
}

set<Team> finalists_awards(TBA_fetcher &f,Event const& a){
	auto found=filter(
		[](auto x){ return x.award_type==tba::Award_type::FINALIST; },
		event_awards(f,a)
	);
	return to_set(teams(found));
}

bool finalists(int);

template<typename ...Ts>
auto finalists(std::variant<Ts...> const&);

template<typename T>
auto finalists(std::optional<T> const& a){
	if(!a){
		return false;
	}
	return finalists(*a);
}

bool finalists(tba::Unknown){
	return 0;
}

bool finalists(tba::Elimination_Alliance_status const& a){
	if(a.status=="eliminated"){
		return 0;
	}
	if(a.status=="won"){
		return 0;
	}
	if(a.status=="playing"){
		return 0;
	}
	PRINT(a);
	nyi
}

template<typename ...Ts>
auto finalists(std::variant<Ts...> const& a){
	return std::visit([](auto x){ return finalists(x); },a);
}

auto finalists(tba::Elimination_Alliance const& a){
	return finalists(a.status);
}

set<Team> finalists_alliance(TBA_fetcher &f,Event const& a){
	auto e=event_alliances(f,a);
	if(!e){
		return set<Team>();
	}

	set<Team> r;
	for(auto alliance:*e){
		if(finalists(alliance)){
			r|=alliance.picks;
		}
	}
	return r;
}

auto finalists_district_points(TBA_fetcher &f,auto e){
	//at least that's what the point total used to be.
	return finish_district_points(f,e,20);
}

set<Team> finalists(TBA_fetcher& f,Event const& e){
	auto a=finalists_awards(f,e);
	auto b=finalists_alliance(f,e);
	auto c=finalists_district_points(f,e);

	if(b.empty() && c.empty()){
		return a;
	}

	if(b.empty() && a==c){
		return a;
	}

	if(b.empty() && subset(c,a)){
		return a;
	}

	if(a.empty() && b.empty()){
		//example: 2010gt gets here.
		return c;
	}

	if(b.empty() && subset(a,c)){
		//2012migl hits here
		//1677 hits the right point total but not because of being a finalist.
		return a;
	}

	if(b.empty() && a.size()>=3){
		//example: 2017micmp & 2018micmp
		return a;
	}

	bool ok=(a==b && a==c);

	if(!ok){
		PRINT(e);
		PRINT(a);
		PRINT(b);
		PRINT(c);
	}

	assert(a==b);
	assert(a==c);
	return a;
}

int winners_demo(TBA_fetcher &f){
	//for each event, look at winners via awards, via alliance status, and via district points
	//and see if they are consistent
	
	for(auto event:events_keys(f)){
		//PRINT(event);
		auto w=winners(f,event);
		//PRINT(w);
		//2004wpi has 0 listed!
		assert(w.size()<=5);//can have 5; see 2001arc

		auto f2=finalists(f,event);
		if(f2.empty()){
			//PRINT(f2);
		}
		//assert(f2.size()>0);
	}
	//cout<<"done\n";
	return 0;
}
