#include "pick_points.h"
#include "../tba/tba.h"
#include "output.h"
#include "interval.h"
#include "set.h"
#include "vector_void.h"
#include "rand.h"
#include "tba.h"
#include "skill_opr.h"

/*For the picking:
 * can put bounds on the number of points earned by each team
 * lower bound is that they don't get picked, which is 0 points unless they are in a rank where they'd become a captain
 *  in which case they get the points of that captain
 *  upper bound is that they get picked immediately unless it's over, in which case they have 0.
 *
 * */

using Team=tba::Team_key;
using Event=tba::Event_key;
using namespace std;

std::vector<Team> teams(tba::Elimination_Alliance const& a){
	return a.picks;
}

/*template<size_t N>
set_fixed<tba::Team_key,N> teams(set_fixed<tba::Team_key,N> const& a){
	return a;
}

template<size_t N>
auto teams(std::array<Team,N> a){
	return a;
}

template<typename T,size_t N>
auto teams(std::array<T,N> const& a){
	return flatten(MAP(teams,a));
}

template<typename T>
vector<Team> teams(std::vector<T> a){
	return flatten(MAP(teams,a));
}

template<typename T>
auto teams(std::optional<T> const& a){
	if(!a){
		return std::vector<Team>();
	}
	return teams(*a);
}*/

using Picks_complete=map<Team,Point>;
using Picks_in_progress=map<Team,Interval<Point>>;

struct Picks_no_data{};

std::ostream& operator<<(std::ostream& o,Picks_no_data){
	return o<<"Picks_no_data";
}

using Pick_points=std::variant<Picks_no_data,Picks_in_progress,Picks_complete>;

#define PICK_STATUS(X)\
	X(COMPLETE)\
	X(IN_PROGRESS)\
	X(NO_DATA)

Pick_points pick_points(TBA_fetcher& f,Event const& event,std::map<Team,Interval<Rank>> const& ranks){
//Pick_points pick_points(TBA_fetcher& f,Event const& event,Rank_range const& ranks){
	auto e=tba::event_alliances(f,event);
	if(!e){
		return Picks_no_data{};
	}

	auto t=teams(e);
	{
		auto teams_without_rank_data=t-keys(ranks);
		if(!teams_without_rank_data.empty()){
			PRINT(teams_without_rank_data);
		}
		assert( teams_without_rank_data.empty() );
	}

	//see if all the spaces are filled in
	auto complete=[=](){
		auto m=mapf([](auto x){ return x.picks.size()==3; },*e);
		return m.size()==8 && all(m);
	}();

	if(complete){
		Picks_complete r;
		for(auto [i,a]:enumerate_from(1,*e)){
			auto const& teams=a.picks;
			auto pts=17-i;
			for(auto team:take(2,teams)){
				r[team]=pts;
			}
			r[teams[2]]=9-i;
		}
		//could put the other teams in here as 0
		return r;
	}

	using Alliance=Int_limited<1,8>;

	using Position=tuple<Alliance,size_t,Point>;

	const auto positions=[&](){
		vector<Position> r;
		for(auto a:options((Alliance*)0)){
			for(auto i:range(2)){
				r|=make_tuple(a,i,17-a);
			}
		}
		for(auto a:options((Alliance*)0)){
			r|=make_tuple(a,2,9-a);
		}
		print_lines(r);
		return r;
	}();

	//fill up all the slots
	//look through the items until find one not filled
	//also, look to see if there are any captain slots not filled
	//for the lower bounds
	//need to look at team rankings to see that

	auto find_team=[&](Position p)->optional<Team>{
		auto [a,index,pts]=p;
		auto n=*e;
		if(a>n.size()){
			return std::nullopt;
		}
		auto n1=n[a-1].picks;
		if(index<n1.size()){
			return n1[index];
		}
		return std::nullopt;
	};

	vector<tuple<Alliance,int,Point>> still_left;
	map<Team,Interval<Point>> r;
	for(auto p:positions){
		auto f=find_team(p);
		if(f){
			r[*f]=get<2>(p);
		}else{
			still_left|=p;
		}
	}
	const auto max_pts_left=max(mapf([](auto x){ return get<2>(x); },still_left));
	const auto captains_left=filter([](auto x){ return get<1>(x)==0; },still_left);

	const auto unpicked_teams=keys(ranks)-keys(r);

	for(auto [team,rank]:ranks){
		auto f=r.find(team);
		if(f!=r.end()){
			continue;
		}
		auto best_case=max_pts_left;
		//figure out how many teams left could be ranked ahead of this team's best
		//find worst case
		auto could_be_ahead=filter(
			[&](auto x){
				return get(ranks,x).min<=rank.max;
			},
			unpicked_teams
		).size();
		auto capt_left_here=skip(could_be_ahead,captains_left);
		auto min_pts=[&]()->Point{
			if(capt_left_here.empty()){
				return 0;
			}
			return get<2>(capt_left_here[0]);
		}();

		r[team]=Interval<Point>{min_pts,best_case};
	}
	return r;
}

int pick_points_demo(TBA_fetcher &f){
	auto data=rand((std::map<Team,Interval<Rank>>*)0);
	auto p=pick_points(f,Event("2025orwil"),data);
	PRINT(p);
	return 0;
}
