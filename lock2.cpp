#include "event_limits.h"
#include "event.h"
#include "multiset_flat.h"
#include "multiset_compare.h"

using namespace std;
using Team=tba::Team_key;

#define FLOOR_RESULT(X)\
	X(TOO_HIGH)\
	X(TOO_LOW)\
	X(PLAUSIBLE)

ENUM_CLASS(Floor_result,FLOOR_RESULT)
ENUM_CLASS_PRINT(Floor_result,FLOOR_RESULT)

template<template<typename>typename MULTISET>
Floor_result find_floor(MULTISET<Interval<Rank_value>> teams,int slots,Rank_value unclaimed,Rank_value threshold){
	assert(slots>=0);
	assert(both_greater_eq(unclaimed,Rank_value()));
	assert(both_greater_eq(threshold,Rank_value()));

	if(unsigned(slots)>teams.size()){
		return Floor_result::TOO_HIGH;
	}

	auto compare_existing=[&]()->Floor_result{
		auto final_pts=MAP(min,teams);
		auto compare=mapf(
			[=](auto x){
				//curiously, std::strong_ordering is not ordered...
				auto cmp=(x<=>threshold);
				if(cmp>0){
					return 1;
				}else if(cmp==0){
					return 0;
				}else{
					return -1;
				}
			},
			final_pts
		);
		if(compare.count(1)>unsigned(slots)){
			return Floor_result::TOO_LOW;
		}
		slots-=compare.count(1);

		return (compare.count(0)>=unsigned(slots))?Floor_result::PLAUSIBLE:Floor_result::TOO_HIGH;
	};

	if(unclaimed==Rank_value()){
		return compare_existing();
	}

	if(slots==0 && teams.size()){
		return Floor_result::TOO_LOW;
	}

	for(auto team:teams){
		switch(compare(team,Interval<Rank_value>{threshold})){
			case Interval_compare::EQUAL:
				return find_floor(teams-team,slots,unclaimed,threshold);
			case Interval_compare::GREATER:{
				//assume that the max has the max for both of the elements.
				return find_floor(
					teams-team,
					slots-1,
					coerce(
						elementwise_max(Rank_value(),unclaimed-team.width()),
						(Rank_value*)0
					),
					threshold
				);
			}
			case Interval_compare::LESS:
				return find_floor(
					teams-team,
					slots,
					coerce(
						elementwise_max(Rank_value(),unclaimed-team.width()),
						(Rank_value*)0
					),
					threshold
				);
			case Interval_compare::INDETERMINATE:
				if(team.min<threshold){
					//PRINT(team.min);
					//PRINT(threshold);
					auto diff=threshold-team.min;
					//PRINT(diff);
					if(diff.second<0){
						diff.second=0;
					}
					//PRINT(diff);
					//PRINT(unclaimed);
					auto to_move=elementwise_min(diff,unclaimed);
					//PRINT(to_move);
					//we get to 0 when there are no more points left, but there are awards.
					//but the awards aren't required
					//once we get out of the loop, these teams will end up first in line for 
					//being assigned to award points.
					if(to_move!=Rank_value()){
						assert(to_move>Rank_value());
						auto t2=teams-team;
						t2|=Interval<Rank_value>{team.min+to_move,team.max};
						return find_floor(
							t2,
							slots,
							coerce(
								unclaimed-to_move,
								(Rank_value*)0
							),
							threshold
						);
					}
				}
				break;
			default:
				assert(0);
		}
	}

	if(unclaimed==Rank_value()){
		return compare_existing();
	}
	if(teams.empty()){
		return Floor_result::PLAUSIBLE;
	}

	auto max_width=elementwise_max(mapf([](auto x){ return x.width(); },teams));

	auto max_floor=max(mapf([](auto x){ return x.min; },teams));
	//PRINT(max_floor);
	auto with_max_floor=filter([=](auto x){ return x.min==max_floor; },teams);
	//PRINT(with_max_floor);
	//PRINT(with_max_floor.size());

	auto max_width_here=max(mapf([](auto x){ return x.width(); },with_max_floor));
	//PRINT(max_width_here);
	auto with_m2=filter([=](auto x){ return max_width_here==x.width(); },with_max_floor);
	assert(with_m2.size());

	auto chosen=car(with_m2);
	
	return find_floor(
		teams-chosen,
		slots-1,
		coerce(
			elementwise_max(Rank_value(),unclaimed-max_width),
			(Rank_value*)0
		),
		threshold
	);
}

//returns 0=impossible, 1=possible; could make it also say whether it thinks it is too high or too low.
//Floor_result find_floor(std::multiset<Interval<Point>> teams,int slots,Point unclaimed,Point threshold){
template<template<typename>typename MULTISET>
Floor_result find_floor(MULTISET<Interval<Point>> teams,int slots,Point unclaimed,Point threshold){
	//cout<<"find floor("<<teams.size()<<" "<<slots<<" "<<unclaimed<<" "<<threshold<<")\n";

	assert(slots>=0);
	assert(unclaimed>=0);
	assert(threshold>=0);

	if(unsigned(slots)>teams.size()){
		return Floor_result::TOO_HIGH;
	}

	auto compare_existing=[&]()->Floor_result{
		auto final_pts=MAP(min,teams);
		std::multiset<int> compare;
		for(auto team:final_pts){
			if(team>threshold){
				compare|=1;
			}else if(team==threshold){
				compare|=0;
			}else{
				compare|=-1;
			}
		}
		//PRINT(compare);
		if(compare.count(1)>unsigned(slots)){
			//too many teams have to be in
			return Floor_result::TOO_LOW;
		}
		slots-=compare.count(1);

		//then there are enough teams to fill this; this is plausible
		//if there are not enough teams, then too many teams have to be out.
		return (compare.count(0)>=unsigned(slots))?Floor_result::PLAUSIBLE:Floor_result::TOO_HIGH;
	};

	if(unclaimed==0){
		return compare_existing();
	}

	if(slots==0 && teams.size()){
		return Floor_result::TOO_LOW;
	}

	//for the teams that are already above theshold, load them up with points, then recurse
	for(auto team:teams){
		switch(compare(team,Interval<Point>{threshold})){
			case Interval_compare::EQUAL:
				//cout<<"eq\n";
				return find_floor(teams-team,slots,unclaimed,threshold);
			case Interval_compare::GREATER:
				//cout<<"greater\n";
				return find_floor(teams-team,slots-1,max(0,unclaimed-team.width()),threshold);
			case Interval_compare::LESS:
				//cout<<"less\n";
				return find_floor(teams-team,slots,max(0,threshold-team.width()),threshold);
			case Interval_compare::INDETERMINATE:
				if(team.min<threshold){
					Point diff=threshold-team.min;
					Point to_move=min(diff,unclaimed);
					assert(to_move>0);
					auto t2=teams-team;
					t2|=Interval<Point>{Point(team.min+to_move),team.max};
					return find_floor(t2,slots,unclaimed-to_move,threshold);
				}
				break;
			default:
				assert(0);
		}
	}

	if(unclaimed==0){
		return compare_existing();
	}
	if(teams.empty()){
		//there are no teams left whose outcomes might force a move, so this threhold is ok.
		return Floor_result::PLAUSIBLE;
	}

	auto max_width=max(mapf([](auto x){ return x.width(); },teams));
	auto max_floor=max(mapf([](auto x){ return x.min; },teams));

	auto with_max_floor=filter([=](auto x){ return x.min==max_floor; },teams);
	assert(with_max_floor.size());

	auto max_width_here=max(mapf([=](auto x){ return x.width(); },with_max_floor));
	auto with_m2=filter([=](auto x){ return max_width_here==x.width(); },with_max_floor);
	assert(with_m2.size());

	auto chosen=car(with_m2);

	if(max_width!=max_width_here){
		//Note: If we never hit this state, then the result "plausible" should actually be "known good"
		//cout<<"approx\n";
	}
	return find_floor(teams-chosen,slots-1,max(0,unclaimed-max_width),threshold);

	//for the teams whose max is below the threshold, load them up with points, then recurse
	//for the teams whose min is lower than the threshold, assign them points to get to the threshold, then recurse
	//if at any point run out of points, doing those, then compare the distribution there to the number of slots

	//if don't run out of points, start assigning them to teams with the max pts already
	//but assume that the point capacity for all the teams is equal, and the highest that is left
	//keep doing this until only 1 slot left
	
	//if ran out of points, then compare the distribution to slots/threshold
	//if did not run out of points, then the threshold is too low.
}

template<typename T>
Interval<T> interval(std::vector<T> a){
	//maybe this should be called limits.
	assert(!a.empty());
	return Interval<T>{min(a),max(a)};
}

vector<int> thresholds(int);

template<typename A,typename B,typename C>
vector<int> thresholds(std::tuple<A,B,C>){
	return range(300);
}

template<typename Status>
auto thresholds(Rank_status<Status>){
	vector<Rank_value> r;
	for(auto awards:range(20)){
		for(auto pts:range(300)){
			r|=Rank_value(awards,pts);
		}
	}
	return r;
}

/*template<typename T>
auto consolidate(T a){
	return a;
}*/

template<typename T>
auto consolidate_inner(std::vector<T> a){
	a=sorted(a);
	std::vector<std::vector<T>> r;
	std::vector<T> here;
	for(auto elem:a){
		if(here.empty()){
			here|=elem;
		}else{
			if(last(here)+1==elem){
				here|=elem;
			}else{
				r|=here;
				here.clear();
				here|=elem;
			}
		}
	}
	if(here.size()){
		r|=here;
	}
	return MAP(interval,r);
}

template<typename T>
auto consolidate(std::vector<T> a){
	return ::as_string(consolidate_inner(a));
}

template<typename A,typename B>
auto consolidate(std::vector<std::pair<A,B>> a){
	a=sorted(a);
	auto g=group([](auto x){ return x.first; },a);
	auto g2=map_values([](auto x){ return consolidate(seconds(x)); },g);
	return g2;
}

//Point find_floor(std::pair<std::map<tba::Team_key,Interval<Point>>,Point> status,int slots){
auto find_floor_loop(auto status,int slots){
	//might need to iteratively look for the threshold
	//like start at 0 and go up until it doesn't work anymore

	//how to iteratively make this work:
	//1) set all the max values to the same number which should be looser so that don't have a dilemma about 
	//which team to choose
	//partition the space by teams always above/below the set threshold first
	//and then some the subproblem

	//auto [teams,unclaimed]=status;
	auto [teams,unclaimed,status2]=status;

	/*auto max_width=max(mapf([](auto x){ return x.width(); },values(team)));
	auto wide_teams=filter([](auto x){ return x.second.width()==max_width; },teams);*/

	//bool find_floor(std::multiset<Interval<Point>> teams,int slots,Point unclaimed,Point threshold){

	using D=ELEM(thresholds(status));

	map<Floor_result,vector<D>> found;
	for(auto threshold:thresholds(status)){
		auto v=find_floor(to_multiset_flat(values(teams)),slots,unclaimed,threshold);
		//auto v=find_floor(to_multiset(values(teams)),0,unclaimed,threshold);
		//cout<<threshold<<": "<<v<<"\n";
		found[v]|=threshold;
	}

	for(auto [k,v]:found){
		cout<<k<<" "<<consolidate(v)<<"\n";
	}

	assert(found[Floor_result::TOO_LOW].size()>0);
	assert(found[Floor_result::TOO_HIGH].size()>0);

	auto fl=interval(found[Floor_result::TOO_LOW]);
	auto fe=interval(found[Floor_result::PLAUSIBLE]);
	auto fg=interval(found[Floor_result::TOO_HIGH]);

	assert(compare(fl,fe)==Interval_compare::LESS);
	assert(compare(fe,fg)==Interval_compare::LESS);

	auto f=found[Floor_result::PLAUSIBLE];
	assert(f.size());
	return min(f);
}

template<typename Status>
auto find_floor(Rank_status<Status> const& a,int slots){
	//go through and see whether or not there are so many unclaimed points that you have to start giving them to teams at the bottom
	//this might be true at the begining of the season because essentailly all the teams will be at 0 points to start with.
	//or towards the end when there are fewer teams whose totals can be altered.
	//a first pass in this direction is just to give as many points as possible to teams that have high current
	//standings
	//but if the top of their ranges is not the max then they might not be the right choice
	//could go for the teams that can take the most points first
	//and if went that way then can see if get to the points threshold or not
	//
	//can try to do the reverse of the locks:
	//1) Assume that the team of interest will get the maximum number of points
	//2) See if it's possible to bump them to where that total 

	//in order to be able to tighten the lower bound, you need to be able to prove that there is not an ordering
	//which has more points
	/*
	To that end:
	1) assign max points to teams that are locked in
	2) assign pts to the threshold to all teams below it
	3) if you've used all the points, then you the threshold does not get reached
	4) if there are more points, assign as many as possible to teams in range, max current standing first
	5) if used all the points, then done and nothing changes
	6) start assigning points raising the floor until out of points or teams that can receive them.
	this is likely to do nothing in most cases
	but is more likely to do something as the number of slots is set lower
	 */

	cout<<"Initial floor:"<<min(mapf([](auto x){ return min(x); },values(a.by_team)))<<"\n";

	auto f1=find_floor_loop(points_only(a),slots);

	//Floor_result find_floor(std::multiset<Interval<Rank_value>> teams,int slots,Rank_value unclaimed,Rank_value threshold){
	auto f2=find_floor_loop(a,slots);

	PRINT(f1);
	PRINT(f2);
	/*if(f1!=f2){
		nyi
	}*/
	return f2;
}

template<typename Status>
auto lock2(Rank_status<Status> const& a,int dcmp_size){
	std::map<Team,string> r;
	//a.by_team
	//a.unclaimed

	auto g=reversed(sorted(group(
		[](auto x){
			return x.second.min;
		},
		a.by_team
	)));

	Rank_value min_threshold=[=](){
		//might be able to find a tighter threshold by assigning values to teams.
		int found=0;
		for(auto [value,teams]:g){
			found+=teams.size();
			if(found>=dcmp_size){
				return value;
			}
		}
		//everyone gets in
		return Rank_value();
	}();

	int already_ranked=0;
	for(auto i:range(g.size())){
		auto [min_value,teams_here]=g[i];

		auto slots_left=dcmp_size-already_ranked;
		if(slots_left<=0){
			//then these teams are not in range
			for(auto [team,info]:teams_here){
				if(info.max>=min_threshold){
					r[team]="out of range";
				}else{
					r[team]="out";
				}
			}
			continue;
		}

		if(slots_left<(int)teams_here.size()){
			for(auto [team,info]:teams_here){
				r[team]="in range, but already tied";
			}
			continue;
		}

		Rank_value budget=a.unclaimed;
		size_t found=0;
		for(size_t j=i+1;j<g.size();j++){
			auto [min2,teams2]=g[j];
			for(auto [team,info]:teams2){
				if(min_value>info.max){
					//this team can't get high enough; skip them
					//would be intersting to know how often this happens because this is
					//one of the key differences compared to the other method.
					continue;
				}
				auto cost=min_value-info.min;
				if(cost.second<0){
					cost.second=0;
				}
				if(cost<=budget){
					budget-=cost;
					found++;
				}else{
					//actually could break out of two layers of loops here
					goto done;
				}
			}
		}
		done:

		const auto leeway=slots_left-teams_here.size();
		if(found<=leeway){
			for(auto [team,info]:teams_here){
				r[team]="in";
			}
		}else{
			for(auto [team,info]:teams_here){
				std::stringstream ss;
				ss<<budget<<" "<<a.unclaimed;
				if(a.unclaimed.second){
					ss<<"\t"<<(budget.second/a.unclaimed.second);
				}
				r[team]="in range";
			}
		}
	}
	return r;
}

void lock2_demo(TBA_fetcher &f,tba::District_key district){
	auto in=district_limits(f,district);
	
	PRINT(in);
	PRINT(entropy(in));

	auto d=[=](){
		auto x=dcmp_size(district);
		assert(x.size()==1);
		return x[0];
	}();

	{
		find_floor(in,d);
		//PRINT(f);
		nyi
	}

	auto out=lock2(in,d);
	//print_r(out);
	PRINT(in.unclaimed);
	PRINT(count(values(out)));
}

int multiset_test(){
	multiset_compare<int> a;
	a|=4;
	a|=5;
	PRINT(a.a);
	PRINT(a.b);
	a=a-4;
	PRINT(a.a);
	PRINT(a.b);

	auto v1=to_vec(a.a);
	auto v2=to_vec(a.b);

	PRINT(v1);
	PRINT(v2);

	cout<<"from a:\n";
	print_lines(a.a);
	cout<<"from b:\n";
	print_lines(a.b);

	return 0;
}

int lock2_demo(TBA_fetcher &f){
	//return multiset_test();

	//lock2_demo(f,tba::District_key("2022chs"));
	//lock2_demo(f,tba::District_key("2022pnw"));
	lock2_demo(f,tba::District_key("2026pnw"));

	return 0;
}
