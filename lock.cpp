#include "lock.h"
#include "../tba/data.h"
#include "util.h"
#include "io.h"
#include "rand.h"
#include "tba.h"
#include "print_r.h"
#include "rank_pts.h"
#include "interval.h"
#include "dates.h"
#include "../tba/tba.h"
#include "optional.h"
#include "skill_opr.h"
#include "int_limited.h"
#include "set_fixed.h"
#include "rank_limits.h"
#include "pick_points.h"

/*
 * would be good to check some of the assumptions made in here with data
 * also, would be nice to check that some of the the limits...
 *
 * TODO: Team ranking result limits
 * */

template<typename A,typename B>
std::pair<A,B> operator-(std::pair<A,B> const& a,std::pair<A,B> const& b){
	return std::make_pair(
		a.first-b.first,
		a.second-b.second
	);
}

template<typename A,typename B>
std::pair<A,B> operator+(std::pair<A,B> const& a,std::pair<A,B> const& b){
	return std::make_pair(
		a.first+b.first,
		a.second+b.second
	);
}

template<typename A,typename B>
std::pair<A,B>& operator+=(std::pair<A,B>& a,std::pair<A,B> const& b){
	a.first+=b.first;
	a.second+=b.second;
	return a;
}

using District_key=tba::District_key;
using Team=tba::Team_key;
using Event=tba::Event_key;
using Event_key=tba::Event_key;
using namespace std;

//using Rank=Int_limited<1,100>;

tba::Team_key rand(tba::Team_key const*){
	std::stringstream ss;
	ss<<"frc"<<rand()%1000;
	return tba::Team_key(ss.str());
}

tba::Event_key rand(tba::Event_key const*){
	std::stringstream ss;
	ss<<"2026";
	for(auto _:range(5)){
		(void)_;
		ss<<char('a'+rand()%26);
	}
	return tba::Event_key(ss.str());
}

using Age_bonus=Int_limited<0,10>;

using I2=Int_limited<0,200>;

//this order will produce inefficient packing.
#define TEAM_INFO(X)\
	X(bool,won_chairmans)\
	X(I2,district_event_points_earned)\
	X(Age_bonus,age_bonus)\
	X(bool,remaining_district_events)\

struct Team_info{
	TEAM_INFO(INST)

	auto operator<=>(Team_info const&)const=default;
};

std::ostream& operator<<(std::ostream& o,Team_info const& a){
	o<<"Team_info( ";
	#define X(A,B) o<<""#B<<":"<<a.B<<" ";
	TEAM_INFO(X)
	#undef X
	return o<<")";
}

auto rand(Team_info const*){
	return Team_info{
		#define X(A,B) rand((A*)0),
		TEAM_INFO(X)
		#undef X
	};
}

using Info_by_team=std::map<Team,Team_info>;

using Event_size=Int_limited<10,80>;

using Event_upcoming=Event_size;//# of teams attending

struct Event_finished{};

std::ostream& operator<<(std::ostream& o,Event_finished const&){
	return o<<"Event_finished";
}

auto rand(Event_finished const*){
	return Event_finished();
}

//obviously going to be possible to deal with events that are part-way through.
using Event_info=std::variant<Event_upcoming,Event_finished>;

using Info_by_event=std::map<Event,Event_info>;

#define LOCK_DATA(X)\
	X(Info_by_team,by_team)\
	X(Info_by_event,by_event)\
	X(unsigned,dcmp_size)

struct Lock_data{
	LOCK_DATA(INST)

	auto operator<=>(Lock_data const&)const=default;
};

std::ostream& operator<<(std::ostream& o,Lock_data const& a){
	o<<"Lock_data( ";
	#define X(A,B) o<<""#B<<":"<<a.B<<" ";
	LOCK_DATA(X)
	#undef X
	return o<<")";
}

auto rand(Lock_data const*){
	return Lock_data{
		#define X(A,B) rand((A*)0),
		LOCK_DATA(X)
		#undef X
	};
}

void print_r(int n,Lock_data const& a){
	indent(n);
	cout<<"Lock_data\n";
	n++;
	#define X(A,B) indent(n); cout<<""#B<<"\n"; print_r(n+1,a.B);
	LOCK_DATA(X)
	#undef X
}

int event_points(size_t event_size){
	//ranking
	auto rank=rank_pts(event_size);
	//PRINT(rank);

	//alliance selection
	const size_t selection=[&](){
		size_t r=0;
		for(auto i:range_inclusive(1,8)){
			r+=(17-i);
			r+=(17-i);
		}
		for(auto i:range_inclusive(1,8)){
			r+=i;
		}
		return r;
	}();

	//awards
	//10=chairmans
	//8=EI, rookie all star
	//5=other judged team awards
	/*others are:
	 * 1) Industrial design
	 * 2) Quality
	 * 3) Innovation in Control
	 * 4) Creativity
	 * 5) Imagery
	 * 6) Team spirit
	 * 7) GP
	 * 8) Judges'
	 * 9) Autonomous
	 * 10) Team sustainability
	 * 11) rising all-star
	 * */
	const auto award_points=86;//10+8*2+5*11;
	//max award pts per team =15
	
	//playoff results
	const auto playoff_pts=[&](){
		vector<int> alliance_playoff_pts{30,20,13,7};
		size_t r=0;
		for(auto x:alliance_playoff_pts){
			r+=3*x;
		}
		return r;
	}();

	return rank+selection+award_points+playoff_pts;
}

void run(Lock_data const& data){
	//print_r(data);

	using Rank_item=std::pair<unsigned short,Point>;

	Rank_item left_to_claim;
	for(auto [k,v]:data.by_event){
		if(std::holds_alternative<Event_upcoming>(v)){
			auto a=std::get<Event_upcoming>(v);
			left_to_claim.first+=1;
			left_to_claim.second+=event_points(a);
		}
	}
	//PRINT(left_to_claim);

	/*
	TODO: Figure out how we want to deal with the fact that we're expecting to get a non-zero number of points
	even if you come in last at the event.

	This means we need to look up from the event size what the limit on pts is
	Might also want to calculate the maximum pts that can get but theoretically a team could win the chairmans award if there are an events left
	which effectively means infinite points.
	*/
	vector<tuple<Rank_item,Team,Team_info>> pts;
	for(auto [team,d]:data.by_team){
		auto points=d.district_event_points_earned+d.age_bonus;
		pts|=make_tuple(
			Rank_item(d.won_chairmans,points),
			team,
			d
		);
	}
	//std::sort(pts.begin(),pts.end());
	pts=reversed(sorted(pts));

	/*cout<<"Current lineup:\n";
	print_lines(pts);
	cout<<"\n";*/

	using Marker=std::string;
	map<Team,Marker> markers;

	//starting by ignoring awards.
	for(auto [i,t]:enumerate(pts)){
		//PRINT(i)
		//PRINT(t);

		auto [n,team,info]=t;

		if(i<data.dcmp_size){
			//currently in range to go
			//PRINT(info);
			unsigned needed_passes=data.dcmp_size-i;
			//PRINT(needed_passes);
			unsigned found=0;
			Rank_item rank_total;
			for(size_t j=i+1;j<pts.size() && found<needed_passes;j++){
				if(!get<2>(pts[j]).remaining_district_events){
					continue;
				}
				auto rank_next=get<0>(pts[j]);
				auto diff=n-rank_next;
				if(diff.second<0){
					diff.second=0;
				}
				//PRINT(diff);
				rank_total+=diff;
				found++;
			}
			if(found<needed_passes){
				//then locked in!
				//because there aren't enough teams to fill all the slots
				markers[team]="in";
			}else{
				//PRINT(rank_total);
				if(rank_total.first>left_to_claim.first || rank_total.second>left_to_claim.second){
					//then locked in
					markers[team]="in";
				}else{
					//not locked in

					auto x=float(rank_total.second)/left_to_claim.second;
					std::stringstream ss;
					ss<<"in range "<<rank_total<<" "<<left_to_claim<<" "<<x;
					markers[team]=ss.str();
				}
			}
		}else{
			//currently in range to miss out
			if(info.remaining_district_events){
				//then always to possibility to win
				//could calculate minimum of what would be needed to get in range
				//and also could calculate what it would take to get to a lock.
				markers[team]="possible";
			}else{
				//then you're out
				markers[team]="out";
			}
		}
	}
	print_lines(markers);

}

int lock_demo(TBA_fetcher& f,District_key district){
	(void)f;
	(void)district;
	run(rand((Lock_data*)0));

	/*Steps:
	 * 1) Calculate how many total points exist in the district
	 *   -how many points exist at each event
	 *   -what is the current status of each event
	 * 2) Calculate which teams are automatically qualified by awards
	 * 3) 
	 *
	 * should try to calculate a lock%
	 *
	 * also, they have some sort of a mode for the championship event
	 *
	 *
	 * display on page
	 * points remaingin in the district
	 * available worlds champs spots
	 *
	 * events
	 * (name) (status) (#teams) (pts available)
	 *
	 * main display:
	 * colors: clinched/in wcmp range/qualifying award/out of wcmp range/prequalified
	 *
	 * rank
	 * team #
	 * districts (points earned)
	 * age bonus (pts)
	 * dcmp pts
	 * total pts
	 * locked? (winner / percentage / - / award / prequalified)
	 *
	 * strategically, might want to make up some data first to test the algorithm 
	 * and then make sure we can fetch all the data.
	 *
	 * for teams that are in the range:
	 * # of teams that need to pass this team
	 * # of teams taht can pass this team
	 *
	 * for teams that are out of the range:
	 * # of teams that need to be passed
	 * # of teams that can be passed
	 *
	 * points to pass:
	 * team#/rank/pts/max pts/pts to tie
	 * */
	//tba::district_rankings
	//
	nyi
}

std::optional<std::pair<optional<int>,int>> award_pts_demo(TBA_fetcher &f,tba::Event_key e){
	auto found=tba::event_district_points(f,e);
	if(!found){
		return std::nullopt;
	}
	auto ap=mapf([](auto x){ return x.award_points; },values(found->points));
	return make_pair(maybe_max(ap),int(sum(ap)));
}

int award_pts_demo(TBA_fetcher &f){
	auto e=all_events(f);
	auto g=group([](auto x){ return make_pair(x.event_type,x.year); },e);
	for(auto [k,v]:g){
		PRINT(k);
		auto m=mapf([&](auto x){ return award_pts_demo(f,x.key); },v);
		auto m2=nonempty(m);
		//print_lines(m2);
		auto c=count(m);
		//print_lines(c);
		auto f=firsts(m2);
		cout<<"max:\n";
		print_lines(count(f));

		cout<<"total\n";
		auto s=seconds(m2);
		print_lines(count(s));
	}

	//auto a=award_pts_demo(f,Event_key("2025orwil"));
	//PRINT(a);

	//for every event, look at the selection of awards given out and figure out how many points total 
	//also, which ones overlap with each other (probably just safety)
	//nyi
	return 0;
}

int lock_demo(TBA_fetcher& f){
	rank_limits_demo(f);
	return 0;
	//return pick_points_demo(f);

	//TODO: Look at alliance selection
	{
		auto x=event_alliances(f,Event("2025orwil"));
		if(!x)nyi
		auto y=*x;
		print_lines(y);
		return 0;
		//return whether picking is complete
		//in progress
	}

	award_pts_demo(f);
	return 0;

	(void)f;
	auto data=rand((Lock_data*)0);
	for(auto _:range(530)){
		data.by_team[rand((Team*)0)]=rand((Team_info*)0);
	}
	for(auto _:range(25*2)){
		//data.by_event[rand((Event_key*)0)]=rand((Event_info*)0);
		data.by_event.insert(make_pair(rand((Event_key*)0),rand((Event_info*)0)));
	}
	data.dcmp_size=40;
	run(data);
	return 0;
}

