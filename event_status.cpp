#include "event_status.h"
#include "status.h"
#include "../tba/tba.h"
#include "print_r.h"
#include "multiset_flat.h"
#include "tba.h"
#include "vector_void.h"

using namespace std;

ENUM_CLASS_PRINT(Event_status,EVENT_STATUS_ITEMS)

static unsigned int playoff_size(TBA_fetcher& tba_fetcher,tba::Event_key event){
	if(event=="2022waspo"){
		//the fact that this special case has to be here is total insanity.
		//who ran that event, anyway?
		return 6;
	}
	auto event_info=tba::event(tba_fetcher,event);
	if(event_info.playoff_type==std::nullopt){
		return 8;
	}
	auto t=*event_info.playoff_type;
	using enum tba::Playoff_type;
	switch(t){
		case BRACKET_8_TEAM: return 8;
		case AVG_SCORE_8_TEAM: return 8;
		case BRACKET_4_TEAM: return 4;
		case DOUBLE_ELIM_8_TEAM: return 8;
		case DE8: return 8;
		default:
			print_r(t);
			nyi
	}
}

int event_points_multiplier(TBA_fetcher &tba_fetcher,tba::Event_key const& event){
	auto e=tba::event(tba_fetcher,event);
	using enum tba::Event_type;
	switch(e.event_type){
		case DISTRICT:
			return 1;
		case DISTRICT_CMP_DIVISION:
		case DISTRICT_CMP:
			return 3;
		case REGIONAL:
		case CMP_DIVISION:
		case OFFSEASON:
		case PRESEASON:
			//no acutal points in this case; could be 0
			return 1;
		default:
			PRINT(e.event_type);
			assert(0);
	}
}

Event_status event_status(TBA_fetcher& tba_fetcher,tba::Event_key const& event){
	if(event.get().substr(0,6)=="202121"){
		//The "participation" events for 2021 are not in the future, despite no play having happened.
		return Event_status::COMPLETE;
	}

	//Doing this check early because a bunch of 2020 events did nothing but award the Chairman's award
	//but they are obviously not still in progress.
	{
		auto c=chairmans(tba_fetcher,event);
		if(!c.empty()){
			return Event_status::COMPLETE;
		}
	}

	auto e=event_district_points(tba_fetcher,event);
	if(!e){
		return Event_status::FUTURE;
	}
	assert(e);
	auto v=values(e->points);
	if(v.empty()){
		return Event_status::FUTURE;
	}

	auto f=[](auto x){
		cout<<x<<"\n";
		cout<<"min:"<<min(x)<<"\n";
		cout<<"max:"<<max(x)<<"\n";
		cout<<"mean:"<<mean(x)<<"\n";
		cout<<"median:"<<median(x)<<"\n";
		cout<<"sum:"<<sum(x)<<"\n";
		cout<<"size:"<<x.size()<<"\n";
	};
	(void)f;

	#define X(A,B) \
		auto B=multiset_flat(::mapf([](auto x){ return x.B; },v));
	TBA_POINTS(X)
	#undef X

	if(sum(qual_points)==0){
		return Event_status::FUTURE;
	}

	auto x1=sum(alliance_points)/event_points_multiplier(tba_fetcher,event);
	auto p1=playoff_size(tba_fetcher,event);
	int expected_selection_pts;
	switch(p1){
		case 8:
			expected_selection_pts=236;
			break;
		case 4:
			expected_selection_pts=124;
			break;
		case 6:
			expected_selection_pts=183;
			break;
		default:
			assert(0);
	}

	if(x1==expected_selection_pts){
		//alliances are complete
	}else if(x1==0){
		//alliance selection has not begun or no data
	}else{
		if(x1>expected_selection_pts){
			PRINT(event);
			PRINT(x1);
			PRINT(expected_selection_pts);
		}
		assert(x1<expected_selection_pts);
		//alliance selection in progress
		//or it's 202wasp and they ran with 6 alliances.
		return Event_status::IN_PROGRESS;
	}

	auto x3=sum(elim_points);
	if(x3==0){
		//then no playoffs yet
		//cout<<"no playoffs\n";
		return Event_status::IN_PROGRESS;
	}

	//because a bunch of districts in the Chesapeake district
	//in 2022 didn't give out a Chairman's Award???
	vector<string> no_chairmans={
		"2022dc305",
		"2022dc313",
		"2022dc320",
		"2022dc327",
		"2022on204",
		"2022on273",
		"2022on305",
		"2022on325",
		"2022on409",
		"2022va306",
		"2022va319"
	};

	for(auto x:no_chairmans){
		if(event==tba::Event_key(x)){
			return Event_status::COMPLETE;
		}
	}

	return Event_status::IN_PROGRESS;
}

std::map<tba::Event_key,Event_status> event_status(TBA_fetcher& tba_fetcher,tba::Year const& year){
	auto a=events(tba_fetcher,year);
	auto b=filter([](auto x){ return x.event_type==tba::Event_type::DISTRICT; },a);
	return to_map(::mapf(
		[&](auto x){ return make_pair(x,event_status(tba_fetcher,x)); },
		keys(b)
	));
}

