#include "event_limits.h"
#include "pick_points.h"
#include "tba.h"
#include "playoff_limits.h"

using namespace std;

Rank_status event_limits(TBA_fetcher &f,tba::Event_key const& event){
	PRINT(event);
	auto ranks=rank_limits(f,event);
	auto picks=pick_limits(f,event,ranks.ranks);
	//p needs to bring both point totals for each of the teams
	//and also for each team whether they are on an alliance or not or don't know. interval<bool>?
	//and also how many selection points may be left

	//playoff_limits(f,in_playoffs);
	//just needs be team -> interval<Point>
	auto playoffs=playoff_limits(f,picks.picked);
	auto d=award_limits(f,event);
	//by_team ->(bool,int), unclaimed_pts, but should also say if unclaimed chairmans

	auto add_map=[&](auto x){
		for(auto [k,v]:x){
			d.by_team[k]+=Interval(Rank_value(0,v.min),Rank_value(0,v.max));
		}
	};
	add_map(ranks.points);
	d.unclaimed.second+=ranks.unclaimed_points;

	add_map(picks.points);
	d.unclaimed.second+=picks.unclaimed;

	add_map(playoffs.by_team);
	d.unclaimed.second+=playoffs.unclaimed_points;

	return d;
}

auto event_limits(TBA_fetcher &f,tba::Event const& e){
	return event_limits(f,e.key);
}

Rank_status operator+(Rank_status a,Rank_status const& b){
	for(auto [k,v]:b.by_team){
		a.by_team[k]+=v;
	}
	a.unclaimed+=b.unclaimed;
	return a;
}

auto district_limits(TBA_fetcher &f,tba::District_key const& district){
	auto e=events(f,district);
	auto m=mapf([&](auto const& x){ return event_limits(f,x); },e);
	return sum(m);
}

int event_limits_demo(TBA_fetcher &f){
	for(auto district:districts(f)){
		PRINT(district);
		auto a=district_limits(f,district);
		print_r(a);
	}
	for(auto const& event:events(f)){
		auto a=event_limits(f,event.key);
		(void)a;
	}
	return 0;
}
