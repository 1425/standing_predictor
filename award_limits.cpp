#include "award_limits.h"
#include "../tba/tba.h"
#include "vector_void.h"
#include "rank_limits.h"
#include "dates.h"
#include "tba.h"
#include "cmp_reason.h"

#define PRINT_STRUCT_INNER(A,B) o<<""#B<<":"<<a.B<<" ";

#define PRINT_STRUCT(NAME,ITEMS)\
	std::ostream& operator<<(std::ostream& o,NAME const& a){\
		o<<""#NAME<<"( ";\
		ITEMS(PRINT_STRUCT_INNER)\
		return o<<")";\
	}

using Team=tba::Team_key;
using namespace std;

//Unless otherwise specified, this works with normal points not after the 3x multiplier from dcmp

PRINT_STRUCT(Award_limits,AWARD_LIMITS)
PRINT_R_ITEM(Award_limits,AWARD_LIMITS)

using Points_by_team=map<Team,Point>;

#define AWARD_POINTS(X)\
	X(Points_by_team,by_team)\
	X(bool,done)

struct Award_points{
	//done=chairmans has been given out.
	AWARD_POINTS(INST)
};

PRINT_STRUCT(Award_points,AWARD_POINTS)
PRINT_R_ITEM(Award_points,AWARD_POINTS)

Point points(tba::Award_type a){
	#define X(A,B) if(a==tba::Award_type::A) return B;
	X(CHAIRMANS,10)
	X(WINNER,0)
	X(SPORTSMANSHIP,0)
	X(FINALIST,0)
	X(CREATIVITY,5)
	X(SPIRIT,5)
	X(BEST_OFFENSIVE_ROUND,0)
	X(BEST_CRAFTSMANSHIP,0)
	X(BEST_DEFENSIVE_MATCH,0)
	X(PLAY_OF_THE_DAY,0)
	X(QUALITY,5)
	X(NUM_1_SEED,0)
	X(MOST_PHOTOGENIC,0)
	X(ROOKIE_ALL_STAR,5)
	X(BEST_PLAY_OF_THE_DAY,0)
	X(LEADERSHIP_IN_CONTROL,5)
	X(OUTSTANDING_DEFENSE,0)
	X(CHAIRMANS_HONORABLE_MENTION,0)
	X(VISUALIZATION,0)
	X(JUDGES,5)
	X(WOODIE_FLOWERS,0)
	X(FEATHERWEIGHT_IN_THE_FINALS,0)
	X(AGAINST_ALL_ODDS,0)
	X(POWER_TO_SIMPLIFY,0)
	X(DESIGN_YOUR_FUTURE,0)
	X(DESIGN_YOUR_FUTURE_HONORABLE_MENTION,0)
	X(TEACHER_PIONEER,0)
	X(DRIVING_TOMORROWS_TECHNOLOGY,5)
	X(CONTENT_COMMUNICATION_HONORABLE_MENTION,0)
	X(TECHNICAL_EXECUTION_HONORABLE_MENTION,0)
	X(SPECIAL_RECOGNITION_CHARACTER_ANIMATION,0)
	X(HIGH_SCORE,0)
	X(IMAGERY,5)
	X(HIGHEST_ROOKIE_SEED,0)
	X(INDUSTRIAL_DEESIGN,5) //sic
	X(INCREDIBLE_PLAY,0)
	X(ENTREPRENEURSHIP,5)
	X(RISING_STAR,5)
	X(REALIZATION,5)
	X(REALIZATION_HONORABLE_MENTION,5)
	X(PEOPLES_CHOICE_ANIMATION,0)
	X(ENGINEERING_INSPIRATION,8)
	X(AUTODESK_INVENTOR,5)
	X(WEBSITE,5)
	X(VOLUNTEER,0)
	X(ROOKIE_INSPIRATION,5)
	X(SAFETY,5)
	X(VISUALIZATION_RISING_STAR,0)
	X(INNOVATION_IN_CONTROL,5)
	X(FOUNDERS,5)
	X(GRACIOUS_PROFESSIONALISM,5)
	X(OUTSTANDING_CART,0)
	X(WSU_AIM_HIGHER,0)
	X(COOPERTITION,0) //assuming this is objective, not judged
	X(ENGINEERING_EXCELLENCE,5)
	X(EXCELLENCE_IN_DESIGN,5)
	X(DEANS_LIST,0)
	X(EXCELLENCE_IN_DESIGN_CAD,0)
	X(EXCELLENCE_IN_DESIGN_ANIMATION,0)
	X(FUTURE_INNOVATOR,0)
	X(MEDIA_AND_TECHNOLOGY,0)
	X(BART_KAMEN_MEMORIAL,5)
	X(MAKE_IT_LOUD,5)
	X(PROGRAMMING,5)
	X(PROFESSIONALISM,5)
	X(GOLDEN_CORNDOG,0)
	X(MOST_IMPROVED_TEAM,5)
	X(WILDCARD,0) //not judged
	X(OTHER,0) //TODO: Figure out what these are & when they occur
	X(CHAIRMANS_FINALIST,0)
	X(AUTONOMOUS,5)
	X(ROOKIE_DESIGN,5)
	X(ENGINEERING_DESIGN,5)
	X(DESIGNERS,5)
	X(CONCEPT,5)
	X(GAME_DESIGN_CHALLENGE_WINNER,5)
	X(GAME_DESIGN_CHALLENGE_FINALIST,0)
	X(INNOVATION_CHALLENGE_SEMI_FINALIST,0)
	X(ROOKIE_GAME_CHANGER,5)
	X(SKILLS_COMPETITION_WINNER,0)
	X(SKILLS_COMPETITION_FINALIST,0)
	X(SUSTAINABILITY,5)
	X(RISING_ALL_STAR,5)
	#undef X
	PRINT(a)
	nyi
}

Point points(tba::Award const& a){
	return points(a.award_type);
}

bool includes_chairmans(std::vector<tba::Award> a){
	auto f=filter([](auto x){ return x.award_type==tba::Award_type::CHAIRMANS; },a);
	if(f.empty()){
		return 0;
	}
	auto t=team_winners(f);
	return !t.empty();
}

Award_points listed_award_points(TBA_fetcher &f,tba::Event_key event){
	auto found=tba::event_awards(f,event);
	auto m=mapf(
		[](auto x){
			return make_pair(team_winners(x),points(x));
		},
		found
	);
	Award_points r;
	for(auto [teams,pts]:m){
		if(!pts) continue;
		for(auto team:teams){
			r.by_team[team]+=pts;
		}
	}

	r.done=(includes_chairmans(found) || event_timed_out(f,event));

	return r;
}

Point max_award_points(int event_size){
	vector<int> v;
	v|=15;
	v|=8;
	for(auto _:range(13)){
		v|=5;
	}
	auto subset=take(event_size,v);
	return min(86,int(sum(subset)));
}

Award_limits award_limits(TBA_fetcher &f,tba::Event_key event,map<Team,Point> already_given){
	//1) calculate the total points already awarded
	//2) calculate total theoretical points at this event
	//this gives unclaimed total points 
	//then calculate limits per team
	//max is 10+5 (chairmans+safety)

	const auto teams=teams_keys(f,event);
	
	int points_left=max_award_points(teams.size())-sum(values(already_given));
	assert(points_left>=0);

	Award_limits r;
	r.unclaimed=points_left;

	for(auto team:teams){
		auto f=already_given.find(team);
		if(f==already_given.end()){
			already_given[team]=0; 
		}
	}

	r.by_team=map_values(
		[=](auto x)->Interval<Point>{
			using R=Interval<Point>;
			std::vector<pair<Point,Point>> n{
				{0,15},
				{5,10},
				{8,0},
				{10,0},
				{13,0},
				{15,0}
			};
			for(auto [a,b]:n){
				if(x==a){
					return R(x,x+min(int(b),points_left));
				}
			}
			nyi
		},
		already_given
	);
	return r;
}

auto event_type(TBA_fetcher &f,tba::Event_key event){
	auto x=tba::event(f,event);
	return x.event_type;
}

Award_limits award_limits(TBA_fetcher &f,tba::Event_key const& event){
	auto b=listed_award_points(f,event);
	if(b.done){
		Award_limits r;
		r.unclaimed=0;
		for(auto [k,v]:b.by_team){
			r.by_team[k]=v;
		}
		return r;
	}
	return award_limits(f,event,b.by_team);
}

void fill_pct(Award_limits const& a){
	auto s=sum(values(a.by_team));
	size_t spread=s.max-s.min;
	assert(a.unclaimed<=spread);
	double p=[=]()->double{
		if(spread){
			return double(a.unclaimed)/spread;
		}
		return 0;
	}();
	(void)p;
	//cout<<s<<" "<<a.unclaimed<<" "<<p<<"\n";
}

int award_limits_demo(TBA_fetcher &f){
	for(auto const& event:events(f)){
		auto a=award_limits(f,event.key);
		fill_pct(a);
	}
	return 0;
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
	auto e=events(f);
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


