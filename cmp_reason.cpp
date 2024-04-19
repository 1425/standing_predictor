#include "cmp_reason.h"
#include "util.h"
#include "set.h"
#include "event_status.h"
#include "event.h"
#include "status.h"
#include "print_r.h"
#include "tba.h"

using namespace std;

#define CMP_REASON_ITEMS(X)\
	X(PRE_QUALIFIED)\
	X(REGIONAL)\
	X(DISTRICT)\
	X(PRIORITY_WAITLIST)\
	X(WAITLIST)

std::ostream& operator<<(std::ostream& o,Cmp_reason a){
	#define X(A) if(a==Cmp_reason::A) return o<<""#A;
	CMP_REASON_ITEMS(X)
	#undef X
	assert(0);
}

static std::optional<tba::Team_key> team_winners(tba::Award_Recipient const& a){
	return a.team_key;
}

static std::vector<tba::Team_key> team_winners(std::vector<tba::Award_Recipient> const& a){
	return flatten(MAP(team_winners,a));
}

static std::vector<tba::Team_key> team_winners(tba::Award const& a){
	return team_winners(a.recipient_list);
}

static std::vector<tba::Team_key> team_winners(std::vector<tba::Award> const& a){
	return flatten(MAP(team_winners,a));
}

static bool dcmp(tba::Event_type a){
	using enum tba::Event_type;
	switch(a){
		case DISTRICT:
			return 0;
		case DISTRICT_CMP:
		case DISTRICT_CMP_DIVISION:
			return 1;
		default:
			PRINT(a);
			nyi
	}
}

static std::set<tba::Team_key> district_quals(
	TBA_fetcher &tba_fetcher,
	tba::District_key district,
	std::set<tba::Team_key> const& attending_teams
){
	//first, see if the district is done
	auto x=district_events(tba_fetcher,district);
	auto f=filter([](auto x){ return dcmp(x.event_type); },x);
	assert(f.size());
	auto cmp_events=keys(f);
	auto a=to_set(mapf([&](auto x){ return event_status(tba_fetcher,x); },cmp_events));
	if(a!=set{Event_status::COMPLETE}){
		return {};
	}
	auto rank=district_rankings(tba_fetcher,district);
	assert(rank);
	auto r=*rank;

	size_t slots=worlds_slots(district);

	auto aw=flatten(mapf([&](auto x){ return event_awards(tba_fetcher,x); },cmp_events));
	std::set<tba::Award_type> auto_qual{
		tba::Award_type::CHAIRMANS,
		tba::Award_type::ENGINEERING_INSPIRATION,
		tba::Award_type::ROOKIE_ALL_STAR
	};
	auto f2=filter([=](auto x){ return auto_qual.count(x.award_type); },aw);
	auto by_award=to_set(team_winners(f2));

	//if some of the award winners aren't attending, then more slots for teams by points
	auto extra_by_award=(by_award-attending_teams).size();
	slots+=extra_by_award;

	std::set<tba::Team_key> by_pts;
	for(size_t i=0;i<r.size() && by_pts.size()<slots;i++){
		auto team=r[i].team_key;
		if(!by_award.count(team)){
			if(!attending_teams.count(team)){
				//if not going, don't count it against the total.
				slots++;
			}
			by_pts|=team;
		}
	}
	return by_award|by_pts;
}

std::vector<tba::District_key> district_keys(TBA_fetcher &tba_fetcher,tba::Year year){
	return keys(districts(tba_fetcher,year));
}

static std::set<tba::Team_key> district_quals(
	TBA_fetcher &tba_fetcher,
	tba::Year year,
	std::set<tba::Team_key> const& attending_teams //so that can fill in for declines
){
	return or_all(mapf(
		[&](auto x){ return district_quals(tba_fetcher,x,attending_teams); },
		district_keys(tba_fetcher,year)
	));
}

static std::set<tba::Team_key> regional_quals(TBA_fetcher &tba_fetcher,tba::Year year){
	auto e=events(tba_fetcher,year);
	auto f=filter([](auto x){ return x.event_type==tba::Event_type::REGIONAL; },e);
	auto m=sorted(mapf([](auto x){ return make_pair(x.week,x.key); },f));
	auto m2=seconds(m);
	//print_r(m2);
	std::set<tba::Team_key> r;
	for(auto event:m2){
		auto aw=event_awards(tba_fetcher,event);
		//print_r(mapf([](auto x){ return x.award_type; },aw));
		std::set<tba::Award_type> advancing{
			tba::Award_type::CHAIRMANS,
			tba::Award_type::WINNER,
			tba::Award_type::WILDCARD,
			tba::Award_type::ENGINEERING_INSPIRATION
		};
		auto f=filter([=](auto x){ return advancing.count(x.award_type); },aw);
		r|=team_winners(f);
	}
	return r;
}

static std::optional<tba::Team_key> ei(TBA_fetcher &tba_fetcher,tba::Event_key event_key){
	auto a=tba::event_awards(tba_fetcher,event_key);
	//PRINT(event_key);
	auto f=filter([](auto x){ return x.award_type==tba::Award_type::ENGINEERING_INSPIRATION; },a);
	if(f.empty()){
		return std::nullopt;
	}
	assert(f.size()==1);
	auto x=f[0].recipient_list;
	assert(x.size()==1);
	auto x2=x[0].team_key;
	assert(x2);
	return *x2;
}

static std::set<tba::Team_key> cmp_ei(TBA_fetcher &tba_fetcher,tba::Year year){
	auto events=events_year(tba_fetcher,year);
	auto f=filter([](auto x){ return x.event_type==tba::Event_type::CMP_DIVISION; },events);
	auto event_keys=keys(f);
	//TODO: if nothign from the fields, then look for something from the main field.
	auto found=to_set(nonempty(mapf([&](auto x){ return ei(tba_fetcher,x); },event_keys)));
	if(found.size()){
		return found;
	}
	auto f2=filter([](auto x){ return x.event_type==tba::Event_type::CMP_FINALS; },events);
	auto m=mapf([&](auto x){ return ei(tba_fetcher,x.key); },f2);
	return to_set(flatten(m));
}

static std::set<tba::Team_key> cmp_chairmans(TBA_fetcher &tba_fetcher,tba::Year year){
	auto events=events_year(tba_fetcher,year);
	auto f=filter([](auto x){ return x.event_type==tba::Event_type::CMP_FINALS; },events);
	auto event_keys=keys(f);
	auto m=mapf([&](auto x){ return chairmans(tba_fetcher,x); },event_keys);
	return to_set(flatten(m));
}

static set<tba::Team_key> chairmans_pre_qualifying(TBA_fetcher &tba_fetcher,tba::Year current_year){
	//the ones besides last year's.
	set<tba::Team_key> r;
	auto at=current_year-2;
	for(int years_found=0;years_found<=10 && at>tba::Year{1992};--at){
		auto found=cmp_chairmans(tba_fetcher,at);
		if(found.size()){
			//cout<<at<<":"<<found<<"\n";
			r|=found;
			years_found++;
		}
	}
	return r;
}

set<tba::Team_key> pre_qualifying(TBA_fetcher &tba_fetcher,tba::Year current_year){
	tba::Year year=current_year-1;
	//PRINT(ei(tba_fetcher,year));

	auto e1=tba::events_year(tba_fetcher,year);
	//print_r(e1);
	auto m=mapf([](auto x){ return x.event_type; },e1);
	//PRINT(to_set(m));

	auto f1=filter(
		[](auto x){
			return x.event_type==tba::Event_type::CMP_FINALS;
		},
		e1
	);
	/*if(f1.size()!=1){
		PRINT(year);
		print_r(f1);
		assert(0);
	}*/
	assert(f1.size()==1 || f1.size()==2);
	//auto f=f1[0];
	std::set<tba::Team_key> teams;
	for(auto f:f1){
		std::set<tba::Award_type> prequalified_award_types{
			tba::Award_type::WINNER,
			tba::Award_type::CHAIRMANS,
			tba::Award_type::CHAIRMANS_FINALIST
		};

		auto key=f.key;
		//PRINT(key);
		auto e=tba::event_awards(tba_fetcher,key);
		auto e2=filter([=](auto x){ return prequalified_award_types.count(x.award_type); },e);
		//print_r(e2);
		auto m2=to_set(nonempty(flatten(mapf(
			[](auto x){
				return mapf([](auto y){ return y.team_key; },x.recipient_list);
			},
			e2
		))));
		teams|=m2;
	}
	auto c=chairmans_pre_qualifying(tba_fetcher,current_year);
	return c|teams|cmp_ei(tba_fetcher,year);
	//print_r(m2);
	//want to find:
	//-championship winners
	//-impact award finalists
	//-EI award winners
	//-impact award winner
	//
	//chairmans from the last 10 FRC championships
	//nyi
}

static std::vector<tba::Team> teams(TBA_fetcher &tba_fetcher){
	std::vector<tba::Team> r;
	for(int i=0;;++i){
		auto t=tba::teams(tba_fetcher,i);
		if(t.empty()){
			break;
		}
		r|=t;
	}
	return r;
}

static std::set<tba::Team_key> original_teams(TBA_fetcher &tba_fetcher){
	auto f=filter(
		[](auto const& team){ return team.rookie_year==tba::Year{1992}; },
		teams(tba_fetcher)
	);
	return to_set(keys(f));
}

static bool cmp_event(tba::Event_type e){
	switch(e){
		case tba::Event_type::CMP_FINALS:
		case tba::Event_type::CMP_DIVISION:
			return 1;
		case tba::Event_type::REGIONAL:
		case tba::Event_type::PRESEASON:
		case tba::Event_type::DISTRICT_CMP:
		case tba::Event_type::DISTRICT:
		case tba::Event_type::DISTRICT_CMP_DIVISION:
		case tba::Event_type::OFFSEASON:
			return 0;
		default:
			PRINT(e);
			nyi
	}
}

static bool cmp_event(tba::Event const& a){
	return cmp_event(a.event_type);
}

std::set<tba::Team_key> cmp_teams(TBA_fetcher &tba_fetcher,tba::Year year){
	auto f=filter(
		[](auto const& x){ return cmp_event(x); },
		tba::events(tba_fetcher,year)
	);
	auto m=mapf([&](auto x){ return event_teams_keys(tba_fetcher,x.key); },f);
	return to_set(flatten(m));
}

static std::set<tba::Team_key> hof(TBA_fetcher &tba_fetcher,tba::Year current_year){
	//teams that were in the hall of fame at the beginning of the specified year
	auto years=range(tba::Year{1992},current_year);
	return or_all(mapf(
		[&](auto x){ return cmp_chairmans(tba_fetcher,x); },
		years
	));
}

static auto regionals(TBA_fetcher &tba_fetcher,tba::Year year){
	auto f=filter(
		[](auto x){ return x.event_type==tba::Event_type::REGIONAL; },
		events_year(tba_fetcher,year)
	);
	return keys(f);
}

static std::vector<tba::Team_key> winners(TBA_fetcher &tba_fetcher,tba::Event_key event){
	auto aw=event_awards(tba_fetcher,event);
	auto f=filter([](auto x){ return x.award_type==tba::Award_type::WINNER; },aw);
	return team_winners(f);
}

static std::optional<tba::Team_key> winning_alliance_second_pick(TBA_fetcher &tba_fetcher,tba::Event_key event){
	auto w=winners(tba_fetcher,event);
	if(w.empty()){
		return std::nullopt;
	}

	auto ea=event_alliances(tba_fetcher,event);

	if(!ea || ea->empty()){
		//no data about alliances, hope that the order of the awards preserves pick order!

		if(w.size()<3){
			//happened w/ the data for 2005fl.
			return std::nullopt;
		}
		return w[2];
	}

	auto f=filter(
		[=](auto x){ return (to_set(x.picks)&to_set(w)).size(); },
		*ea
	);

	if(f.size()!=1){
		PRINT(event);
		PRINT(w);
		print_r(*ea);
		print_r(f);
		nyi
	}

	auto fu=f[0];
	assert(fu.picks.size()>=3);
	return fu.picks[2];

	#if 0
	PRINT(w);
	nyi

	auto aw=event_awards(tba_fetcher,event);
	if(aw.empty()){
		return std::nullopt;
	}
	PRINT(event);
	print_r(aw);
	nyi

	//can't just do it this way because sometimes the last match result is missing
	//even though the event is over, so need to look at awards to figure out who won.
	auto e=event_alliances(tba_fetcher,event);

	//probably because the event has not yet happened.
	if(!e) return std::nullopt;

	auto f1=filter([](auto x){ return winning(x.status); },*e);
	if(f1.empty()){
		return std::nullopt;
	}
	assert(f1.size()==1);
	auto f=f1[0];
	/*if(f.picks.size()!=3){
		PRINT(event);
		print_r(f);
	}*/
	assert(f.picks.size()>=3);
	return f.picks[2];
#endif
}

static std::set<tba::Team_key> rookie_all_star(TBA_fetcher &tba_fetcher,tba::Event_key const& event){
	auto aw=event_awards(tba_fetcher,event);
	auto f=filter([](auto x){ return x.award_type==tba::Award_type::ROOKIE_ALL_STAR; },aw);
	//This can be >1 team.  Example: 2004on
	return to_set(team_winners(f));
}

static std::set<tba::Team_key> regional_rookie_all_stars(TBA_fetcher &tba_fetcher,tba::Year year){
	return or_all(mapf(
		[&](auto x){ return rookie_all_star(tba_fetcher,x); },
		regionals(tba_fetcher,year)
	));
}

static std::set<tba::Team_key> regional_winning_alliance_second_pick(TBA_fetcher &tba_fetcher,tba::Year year){
	auto m=mapf(
		[&](auto x){ return winning_alliance_second_pick(tba_fetcher,x); },
		regionals(tba_fetcher,year)
	);
	return to_set(nonempty(m));
}

static std::optional<tba::Team_key> winning_backups(TBA_fetcher &tba_fetcher,tba::Event_key event){
	auto w=winners(tba_fetcher,event);
	if(w.size()<4){
		return std::nullopt;
	}

	auto ea=event_alliances(tba_fetcher,event);

	if(!ea || ea->empty()){
		//in case don't have the alliance directly recorded.
		return w[3];
	}

	assert(ea);
	auto e=*ea;
	assert(e.size());
	auto f=filter_unique([=](auto x){ return (to_set(w)&to_set(x.picks)).size(); },e);

	if(f.picks.size()==3){
		auto r=to_set(w)-to_set(f.picks);
		assert(r.size()==1);
		return *r.begin();
	}

	//PRINT(event);
	//PRINT(f.picks);
	assert(f.picks.size()>=4);
	return f.picks[3];
}

static std::set<tba::Team_key> regional_winning_backups(TBA_fetcher& tba_fetcher,tba::Year year){
	return to_set(nonempty(mapf(
		[&](auto x){ return winning_backups(tba_fetcher,x); },
		regionals(tba_fetcher,year)
	)));
}

static std::set<tba::Team_key> priority_waitlist(TBA_fetcher &tba_fetcher,tba::Year year){
	std::set<tba::Team_key> r;

	//Criteria for the priority waitlist:
	//1) HOF teams not from the last 10 years
	r|=hof(tba_fetcher,year)-chairmans_pre_qualifying(tba_fetcher,year);

	//2) original and sustaining teams
	r|=original_teams(tba_fetcher);

	//3) regional winning alliance second pick
	r|=regional_winning_alliance_second_pick(tba_fetcher,year);

	//4) regional regional rookie all star
	r|=regional_rookie_all_stars(tba_fetcher,year);

	//5) regional winning alliance backup teams
	r|=regional_winning_backups(tba_fetcher,year);

	return r;
}

map<tba::Team_key,Cmp_reason> cmp_reasons(TBA_fetcher &tba_fetcher,tba::Year year){
	auto teams=cmp_teams(tba_fetcher,year);
	map<tba::Team_key,Cmp_reason> r;

	auto f=[&](Cmp_reason reason,std::set<tba::Team_key> team_list){
		for(auto elem:team_list){
			r[elem]=reason;
		}
		teams-=team_list;
	};

	f(Cmp_reason::PRE_QUALIFIED,pre_qualifying(tba_fetcher,year));
	f(Cmp_reason::REGIONAL,regional_quals(tba_fetcher,year));
	f(Cmp_reason::DISTRICT,district_quals(tba_fetcher,year,teams));
	f(Cmp_reason::PRIORITY_WAITLIST,priority_waitlist(tba_fetcher,year));

	for(auto t:teams){
		r[t]=Cmp_reason::WAITLIST;
	}

	return r;
#if 0

	auto f=filter(
		//[](auto x){ return x.event_type==tba::Event_type::CMP_FINALS; },
		//[](auto x){ return cmp_event(x.event_type==tba::Event_type::CMP_FINALS; },
		[](auto x){ return cmp_event(x); },
		tba::events(tba_fetcher,year)
	);
	auto teams=flatten(mapf([&](auto x){ return teams(tba_fetcher,x); },f));
	print_r(m);
	/*for(auto event:f){
		auto t=to_set(event_teams_keys(tba_fetcher,event.key));
		nyi
	}*/
#endif
}

