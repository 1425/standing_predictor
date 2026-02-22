#include "pick_points.h"
#include "../tba/tba.h"
#include "interval.h"
#include "set.h"
#include "vector_void.h"
#include "rand.h"
#include "tba.h"
#include "print_r.h"
#include "rank_limits.h"
#include "vector_fixed.h"
#include "vector_fixed2.h"
#include "set_limited.h"

/*For the picking:
 * can put bounds on the number of points earned by each team
 * lower bound is that they don't get picked, which is 0 points unless they are in a rank where they'd become a captain
 *  in which case they get the points of that captain
 *  upper bound is that they get picked immediately unless it's over, in which case they have 0.
 *
 *
 * would also be interesting to be able to go from alliance selection results to limits on the rankings of the teams involved.
 * (and could assume no declines if not listed, or not)
 * */

using Team=tba::Team_key;
using Event=tba::Event_key;
using namespace std;

void print_r(int n,tba::Event const& a){
	indent(n);
	cout<<"Event\n";
	n++;
#define X(A,B) indent(n); std::cout<<""#B<<"\n"; print_r(n+1,a.B);
	TBA_EVENT(X)
#undef X
}

std::vector<Team> teams(tba::Elimination_Alliance const& a){
	if(a.backup){
		//print_r(a);
		//PRINT(a.backup);
		return a.picks|a.backup->in;
	}
	return a.picks;
}

auto teams(tba::Award_Recipient const& a){
	return a.team_key;
}

std::set<Team> teams(tba::Award const& a){
	return or_all(MAP(teams,a.recipient_list));
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

//using Picks_in_progress=map<Team,Interval<Point>>;

#define PICKS_IN_PROGRESS(X)\
	X(Point_range<tba::Team_key>,by_team)\
	X(unsigned,unclaimed)\
	X(bool,started)

struct Picks_in_progress{
	PICKS_IN_PROGRESS(INST)
	
	auto operator<=>(Picks_in_progress const&)const=default;
};

std::ostream& operator<<(std::ostream& o,Picks_in_progress const& a){
	o<<"Picks_in_progress( ";
	#define X(A,B) o<<""#B<<":"<<a.B<<" ";
	PICKS_IN_PROGRESS(X)
	#undef X
	return o<<")";
}

PRINT_R_ITEM(Picks_in_progress,PICKS_IN_PROGRESS)

struct Picks_no_data{};

std::ostream& operator<<(std::ostream& o,Picks_no_data){
	return o<<"Picks_no_data";
}

struct Picks_known_empty{};

std::ostream& operator<<(std::ostream& o,Picks_known_empty){
	return o<<"Picks_known_empty";
}

using Pick_points=std::variant<
	Picks_no_data,
	Picks_in_progress,
	Picks_complete,
	Picks_known_empty
>;

#define PICK_STATUS(X)\
	X(COMPLETE)\
	X(IN_PROGRESS)\
	X(NO_DATA)

using Alliance_number=Int_limited<1,8>;

using Position=tuple<Alliance_number,Int_limited<0,2>,Point>;

//this is of course assuming the 3-team era
constexpr auto positions=[](){
	vector_fixed2<Position,24> r;
	for(auto a:options((Alliance_number*)0)){
		for(auto i:range_st<2>()){
			r|=make_tuple(a,i,17-a);
		}
	}
	for(auto a:options((Alliance_number*)0)){
		r|=make_tuple(a,2,9-a);
	}
	//print_lines(r);
	return r;
}();

using Alliance=vector_fixed<Team,4>;//4th exists if you pick your own backup or if a backup was used.
using Selection_results=std::array<Alliance,8>;

template<size_t N>
auto teams(vector_fixed<Team,N> const& a){
	return a;
}

auto teams(Selection_results const& a){
	return or_all(MAP(to_set,a));
}

#define FROM_ALLIANCES(X)\
	X(Rank_range<tba::Team_key>,by_team)\
	X(std::optional<Interval<Rank>>,not_picked)

struct From_alliances{
	FROM_ALLIANCES(INST)
};

std::ostream& operator<<(std::ostream& o,From_alliances const& a){
	o<<"From_alliances( ";
	#define X(A,B) o<<""#B<<":"<<a.B<<" ";
	FROM_ALLIANCES(X)
	#undef X
	return o<<")";
}

From_alliances ranks_from_alliances(Selection_results const& alliances,int event_size){
	//going to try the no-declines case first.
	
	/*if wanted to have a go at seeing who it might be with declines:
	 * 1) a1 capt did not decline
	 * 2) a2 captain could have been ranked below anyone who isn't picked later
	 *    so if there are N teams, a2 capt could have been ranked N-(8+7+6) assuming
	 *    that all the slots are filled.
	 *
	 * TODO Have something identify the real events that have incomplete rank data but
	 * do have alliance selection data.
	 * */
	map_auto<Team,Interval<Rank>> r;

	/* Ways to check this:
	 * read alliance selection of event
	 * convert to rank limits
	 * read rankings
	 * read rank limits if rankings not available
	 *
	 *
	 *
	 * */

	if(teams(alliances).size()>unsigned(event_size)){
		auto t=teams(alliances);
		PRINT(t);
		PRINT(t.size());
		PRINT(event_size);
		throw std::invalid_argument("event size not consistent with picks");
	}

	assert(teams(alliances).size()<=size_t(event_size));

	Interval<int> top_taken(0);

	auto not_picked=[&]()->std::optional<Interval<Rank>>{
		if(top_taken.min>=event_size){
			return std::nullopt;
		}
		return Interval<Rank>{Rank(top_taken.min+1),Rank(event_size)};
	};

	auto to_return=[&](){
		From_alliances a;
		a.by_team=r;
		a.not_picked=not_picked();
		return a;
	};

	for(auto [alliance_number,position,pts]:positions){
		auto this_alliance=alliances[alliance_number-1];
		if(this_alliance.size()<=position){
			//at this point, any teams that are not already classified should be set to
			//the range that is still possible
			auto left=teams(alliances)-keys(r);
			for(auto team:left){
				r[team]=Interval<Rank>{Rank(top_taken.min+1),Rank(event_size)};
			}
			return to_return();
		}
		auto team=this_alliance[position];
		if(position==0){
			top_taken.min++;
			top_taken.max++;

			int must_be_lower=[=](){
				//all the 3rd+picks and all the captains and first picks below here
				auto third_plus=sum(mapf([](auto x){ return skip(2,x).size(); },alliances));
				auto later_captains=count_if(
					[](auto x){ return !x.empty(); },
					skip(alliance_number,alliances)
				);
				auto later_p1=later_captains+1;
				//PRINT(alliance_number)
				//PRINT(later_captains);
				return third_plus+later_captains+later_p1;
			}();
			if(event_size<=must_be_lower){
				PRINT(alliances);
				PRINT(event_size);
			}
			assert(event_size>must_be_lower);

			//This is what you get if you assume no declines
			//r[team]=Interval<Rank>(top_taken.min,top_taken.max);

			//PRINT(must_be_lower);
			//If you might have declines
			r[team]=Interval<Rank>(top_taken.min,event_size-must_be_lower);
		}else{
			top_taken.max++;
			r[team]=Interval<Rank>{Rank(top_taken.min+1),Rank(event_size)};
		}
	}
	return to_return();
}

bool normal_selection(tba::Playoff_type a){
	using enum tba::Playoff_type;
	switch(a){
		case BRACKET_8_TEAM:
		case AVG_SCORE_8_TEAM:
			return 1;
		case BRACKET_16_TEAM:
		case BRACKET_4_TEAM://would probably work just fine in most of the code though.
			return 0;
		default:
			PRINT(a);
			nyi
	}
}

bool normal_selection(std::optional<tba::Playoff_type> a){
	if(!a) return 1;
	return normal_selection(*a);
}

std::optional<Selection_results> selection_results(TBA_fetcher &f,tba::Event const& event){
	/*if(!normal_selection(event.playoff_type)){
		print_r(event);
		return std::nullopt;
	}*/

	auto e=tba::event_alliances(f,event.key);
	if(!e){
		return std::nullopt;
	}
	auto e1=*e;
	if(e1.size()>8){
		//this is invalid; expected 8-alliance playoff
		return std::nullopt;
		/*print_r(event);
		PRINT(event.playoff_type);
		print_r(e);*/
	}
	//print_r(e1);
	assert(e1.size()<=8);//see 2010hi
	try{
		auto m=mapf([](auto x){ return Alliance(x.picks); },e1);
		Selection_results r;
		for(auto i:range(e1.size())){
			r[i]=m[i];
		}
		return r;
	}catch(...){
		PRINT(event);
		print_r(e1);
		throw;
	}
	//auto const& p=e->picks;

	//print_r(e);
	//nyi
}

//using RR=iRank_range<Team>;
using RR=map<Team,Interval<Rank>>;

bool rankings_consistent(RR a,RR b);

optional<bool> rankings_consistent(optional<RR>,RR);

bool rankings_consistent(map<Team,Rank> const& a,RR const& b){
	for(auto [k,v]:join(a,b)){
		auto [a1,b1]=v;
		if(!a1 || !b1){
			cout<<"Missing: "<<k<<v<<"\n";
			return 0;
		}
		if(!subset(*a1,*b1)){
			cout<<k<<"\t"<<a1<<"\t"<<b1<<"\n";
			return 0;
		}
	}
	return 1;
}

std::optional<bool> rankings_consistent(std::optional<map<Team,Rank>> const& a,RR const& b){
	if(a){
		return rankings_consistent(*a,b);
	}
	return std::nullopt;
}

auto rankings_consistent(map<Team,Rank> const& a,From_alliances b){
	for(auto team:keys(a)){
		auto f=b.by_team.find(team);
		if(f==b.by_team.end()){
			assert(b.not_picked);
			b.by_team[team]=*b.not_picked;
		}
	}
	return rankings_consistent(a,b.by_team);
}

template<typename A,typename B>
auto rankings_consistent(std::optional<A> const& a,B const& b){
	using R=std::optional<decltype(rankings_consistent(*a,b))>;
	if(!a){
		return R(std::nullopt);
	}
	return R(rankings_consistent(*a,b));
}

int ranks_from_demo(TBA_fetcher &f){
	//print_lines(positions);
	Selection_results a;
	/*a[0]|=Team("frc1");
	a[0]|=Team("frc2");
	a[0]|=Team("frc3");
	a[1]|=Team("frcb");*/
	a=rand((Selection_results*)0);
	auto r=ranks_from_alliances(a,30);
	//print_r(r);

	auto e=FILTER(normal_ranking_expected,events(f));
	PRINT(e.size());
	//e=take(600,e);
	for(auto event:e){
		if(
			event.key==tba::Event_key("2006nh")
			|| event.key==tba::Event_key("2009arc") //where did 348 come from?
			|| event.key==tba::Event_key("2010arc") //how does team at rank 3 captain alliance 4?
			|| event.key==tba::Event_key("2010il") //rank 1 becomes a pick...lol
			|| event.key==tba::Event_key("2010la") //256 appers only in elims
			|| event.key==tba::Event_key("2010mi") //394 appers only in elims
			|| event.key==tba::Event_key("2010oc") //2150 appers only in elims
			|| event.key==tba::Event_key("2010wor") //rank 7 became a8 capt
			|| event.key==tba::Event_key("2011ca") //2619 appears only in elims
			|| event.key==tba::Event_key("2011ct") //rank 2 becomes alliance 3 cpt
			|| event.key==tba::Event_key("2011dmn") //rank 5 is listed as a 2nd pick
			|| event.key==tba::Event_key("2011ga") //2665 only shows up in elims
			|| event.key==tba::Event_key("2011gg") //rank 4 becomes a5 cpt
			|| event.key==tba::Event_key("2011hi") //2439 appears in elims but not in alliances
			|| event.key==tba::Event_key("2011is") //rank 8 appears as second pick
			|| event.key==tba::Event_key("2011li") //547 appears from nowhere
			|| event.key==tba::Event_key("2011mo") //rank 6 is alliance 8 cpt
			|| event.key==tba::Event_key("2011nv") //1485 appears
			|| event.key==tba::Event_key("2011on2") //rank 3 as cpt 4
			|| event.key==tba::Event_key("2011wa2") //rank2 shows up as a2 p1
			|| event.key==tba::Event_key("2012md") //team in elims does not appear in alliance
			|| event.key==tba::Event_key("2012migl") //mystery team in alliance
			|| event.key==tba::Event_key("2012mn") //alliance 1 captain is wrong
			|| event.key==tba::Event_key("2012tn") //mystery team
			|| event.key==tba::Event_key("2013azch") //order wrong
			|| event.key==tba::Event_key("2013calb") //rank 4 becomes 2nd pick
			|| event.key==tba::Event_key("2013casd") //missing from alliance
			|| event.key==tba::Event_key("2013ctha") //order wrong
			|| event.key==tba::Event_key("2013flor") //appears
			|| event.key==tba::Event_key("2013lake") //3606 listed twice on its alliance
			|| event.key==tba::Event_key("2013ncre") //appears
			|| event.key==tba::Event_key("2013pahat") //order
			|| event.key==tba::Event_key("2013txda") //appears
			|| event.key==tba::Event_key("2013utwv") //order
			|| event.key==tba::Event_key("2013wimi") //wrong a1 capt
			|| event.key==tba::Event_key("2014miesc") //2851 wrong
			|| event.key==tba::Event_key("2014misou") //appears
			|| event.key==tba::Event_key("2014onto2") //8th as pick2
			|| event.key==tba::Event_key("2015inind") //8th does not appear
			|| event.key==tba::Event_key("2018tuis") //2nd rank does not appear
			|| event.key==tba::Event_key("2020waspo") //appears
			|| event.key==tba::Event_key("2022arli") //7-8 alliances are ficticious
			|| event.key==tba::Event_key("2022bcvi") //a8 ficticious
			|| event.key==tba::Event_key("2022hiho") //9985 appears
			|| event.key==tba::Event_key("2022mokc3") //appears
			|| event.key==tba::Event_key("2022wayak") //alliance 8 is full of made-up teams
			|| event.key==tba::Event_key("2023gaalb") //alliance 7-8 made-up teams
			|| event.key==tba::Event_key("2024vapor") //alliance 8 is full of made-up teams
			|| event.key==tba::Event_key("2025ncash") //alliance 8 is full of made-up teams
		  ){
			//elim alliances are at least in the wrong order here!
			continue;
		}
		auto l1=listed_ranks(f,event.key);
		if(!l1) continue;
		auto l2=rank_limits(f,event.key);
		auto sr=selection_results(f,event);
		if(!sr){
			continue;
			print_r(event);
			nyi
		}
		//print_r(event.key);
		auto event_size=l2.ranks.size();
		auto l3=ranks_from_alliances(*sr,event_size);
		auto c=rankings_consistent(l1,l3);
		assert(c);
		if(!*c){
			PRINT(event);
			//print_r(l1);
			print_r(l3);
			assert(0);
		}
		//PRINT(c);
		//nyi
	}
	return 0;
}

std::optional<map<Team,Point>> listed_pick_points(TBA_fetcher &f,Event const& event){
	auto d=district(f,event);
	if(!d){
		return std::nullopt;
	}
	auto dr=tba::district_rankings(f,*d);
	if(!dr){
		return std::nullopt;
	}
	map<Team,Point> r;
	for(auto team_info:*dr){
		for(auto event_info:team_info.event_points){
			if(event_info.event_key!=event) continue;
			r[team_info.team_key]=event_info.alliance_points;
		}
	}
	return r;
}

bool picks_expected(TBA_fetcher &f,Event const& event){
	auto e=event_type(f,event);
	switch(e){
		case tba::Event_type::DISTRICT:
		case tba::Event_type::DISTRICT_CMP_DIVISION:
			return 1;
		case tba::Event_type::DISTRICT_CMP:{
			//If there are no divisions in this district. 
			auto d=district(f,event);
			assert(d);
			auto ev=events(f,*d);
			auto f=count_if([](auto x){ return x.event_type==tba::Event_type::DISTRICT_CMP_DIVISION; },ev);
			return f==0;
		}
		case tba::Event_type::CMP_FINALS:
			return 0;
		case tba::Event_type::REGIONAL:
		case tba::Event_type::CMP_DIVISION:
		case tba::Event_type::OFFSEASON:
		case tba::Event_type::PRESEASON:
			return 1;
		case tba::Event_type::FOC:
		case tba::Event_type::REMOTE:
			return 0;
		default:
			PRINT(event);
			PRINT(e);
			nyi
	}
}

Pick_points pick_points(TBA_fetcher& f,Event const& event,std::map<Team,Interval<Rank>> const& ranks){
//Pick_points pick_points(TBA_fetcher& f,Event const& event,Rank_range const& ranks){

	/* 1) Calculate alliances via tba::event_allinace
	 * 2) Calculate alliances via district points
	 * 3) Attempt to reconsile them
	 * 4) Attempt to fill in the rest via the ranks that are passed in
	 * */

	if(!picks_expected(f,event)){
		return Picks_known_empty();
	}

	using Slot=std::set<Team>;
	static constexpr auto ALLIANCE_SLOTS=3;
	using Alliance=std::array<Slot,ALLIANCE_SLOTS>;
	static constexpr auto ALLIANCES=8;
	using Full=std::array<Alliance,ALLIANCES>;

	const auto by_alliances=[&]{
		Full full;
		auto e=tba::event_alliances(f,event);
		if(e){
			auto alliances_to_use=take(ALLIANCES,*e);
			if(alliances_to_use!=*e){
				//something wrong with the data
				//could report this somehow
				//this is known to occur at 2015bc where they had 19 alliances
			}
			for(auto [ai,elim_alliance]:enumerate(alliances_to_use)){
				assert(ai<ALLIANCES);
				for(auto [pi,team]:enumerate(elim_alliance.picks)){
					if(pi>=ALLIANCE_SLOTS){
						//backup; no points
					}else{
						full[ai][pi]|=team;
					}
				}
			}
		}
		return full;
	}();

	using Pick_location=Int_limited<0,ALLIANCE_SLOTS-1>;
	using Location=pair<Alliance_number,Pick_location>;

	const auto mult=event_points_multiplier(f,event);

	auto find_locations=[=](Point p){
		p/=mult;

		vector<Location> r;
		for(auto i:range_inclusive(1,8)){
			if(p==17-i){
				r|=Location(i,0);
				r|=Location(i,1);
			}
			if(p==i){
				r|=Location(i,2);
			}
		}
		auto ok=(p==0 || !r.empty());
		if(!ok){
			PRINT(event);
			PRINT(ranks);
			PRINT(p);
			PRINT(r);
		}
		assert(p==0 || !r.empty());
		return r;
	};

	const auto by_pts=[&]{
		Full full;
		auto a=listed_pick_points(f,event);
		if(a){
			for(auto [team,pts]:*a){
				auto l=find_locations(pts);
				for(auto x:l){
					full[x.first-1][x.second]|=team;
				}
			}
		}
		return full;
	}();

	//now reconsile the two sets of data
	//at any locaton, if only one team appears, then it's that team
	//at any location if a team appears in both it's that team

	Full combo;
	for(auto ai:range(ALLIANCES)){
		auto a1=by_alliances[ai];
		auto a2=by_pts[ai];
		auto &a_out=combo[ai];
		for(auto p:range(ALLIANCE_SLOTS)){
			auto p1=a1[p];
			auto p2=a2[p];
			auto &p_out=a_out[p];

			auto both=p1&p2;
			if(both.size()==1){
				p_out=both;
			}else if(both.size()==0){
				auto either=p1|p2;
				p_out=either;
			}else{
				assert(0);
			}
		}
	}

	//for each of the slots, if there is a unique team, then assume it's there
	//and if that team appears elsewhere, remove that option.
	//this happens in 2010mi.

	std::set<tba::Team_key> unique;
	for(auto a1:combo){
		for(auto slot:a1){
			if(slot.size()==1){
				unique|=car(slot);
			}
		}
	}

	for(auto& a:combo){
		for(auto &slot:a){
			if(slot.size()!=1){
				slot-=unique;
			}
		}
	}

	//You can have places where placements and pts disagree.  See 2020dc305, where there were only 4 alliances.
	//Could addd a mode specifically for 4-team alliances
	//But instead, going to filter extra mentions of teams.

	std::set<tba::Team_key> taken;
	for(auto [alliance,pick,pts]:positions){
		auto &here=combo[alliance-1][pick];
		here-=taken;
		taken|=here;
	}

	/*PRINT(combo);
	nyi

	auto e=tba::event_alliances(f,event);
	if(!e){
		auto a=listed_pick_points(f,event);
		PRINT(a);
		if(a){
			return *a;
		}
		return Picks_no_data{};
	}*/

	auto t=teams(combo);

	if(t.size()!=to_set(t).size()){
		print_lines(count(t));
	
		cout<<"By alliances:\n";
		print_lines(by_alliances);
		cout<<"\n";

		cout<<"By points:\n";
		print_lines(by_pts);
		cout<<"\n";

		print_lines(combo);
	}

	assert(t.size()==to_set(t).size());

	//PRINT(t);
	//PRINT(ranks);
	if(0){
		auto teams_without_rank_data=t-keys(ranks);
		if(!teams_without_rank_data.empty()){
			print_r(event);
			PRINT(teams_without_rank_data);
		}
		assert( teams_without_rank_data.empty() );
	}

	//see if all the spaces are filled in
	auto complete=[=]()->bool{
		return 0;//if(
		nyi/*auto m=mapf([](auto x){ return x.picks.size()==3; },*e);
		return m.size()==8 && all(m);*/
	}();

	//we might also know that they are complete if we see playoff matches
	//this occurs when there's an event with fewer than eight alliances.  See 2022waspo.
	if(playoffs_started(f,event)){
		complete=1;
	}

	if(complete){
		Picks_complete r;
		for(auto [alliance,pick,points]:positions){
			auto s=combo[alliance-1][pick];
			if(s.size()>1){
				//print_lines(combo);
				//PRINT(s);
				//there is something wrong with the data coming in; just make a guess
				s=take(1,s);
			}
			assert(s.size()<=1);
			if(!s.empty()){
				r[car(s)]=points;
			}
		}
		//could put the other teams in here as 0
		return r;
	}
	
	//fill up all the slots
	//look through the items until find one not filled
	//also, look to see if there are any captain slots not filled
	//for the lower bounds
	//need to look at team rankings to see that

	auto find_team=[&](Position p)->optional<Team>{
		auto [a,index,pts]=p;
		auto n=combo;
		if(a>n.size()){
			return std::nullopt;
		}
		auto n1=n[a-1];
		if(index<n1.size()){
			//return n1[index];
			auto here=n1[index];
			//print_lines(combo);
			//cout<<"\n";

			assert(here.size()<2);
			if(here.empty()){
				return std::nullopt;
			}
			return car(here);
		}
		return std::nullopt;
	};

	vector<tuple<Alliance_number,int,Point>> still_left;
	//map<Team,Interval<Point>> r;
	Picks_in_progress r;
	r.started=0;

	for(auto p:positions){
		auto f=find_team(p);
		if(f){
			r.by_team[*f]=get<2>(p);
			r.started=1;
		}else{
			still_left|=p;
		}
	}

	const auto unpicked_teams=keys(ranks)-keys(r.by_team);

	//Limit still_left to the number of teams that are eligible to be picked still
	//because otherwise it will look like there are extra points to grab
	//but it is not possible for anyone to get them.
	/*if(still_left.size()>unpicked_teams.size()){
		PRINT(still_left.size());
		PRINT(unpicked_teams.size());
	}*/
	still_left=take(unpicked_teams.size(),still_left);

	const auto max_pts_left=max_else(
		mapf([](auto x){ return get<2>(x); },still_left),
		Point(0)
	);
	const auto captains_left=filter([](auto x){ return get<1>(x)==0; },still_left);

	for(auto [team,rank]:ranks){
		auto f=r.by_team.find(team);
		if(f!=r.by_team.end()){
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

		r.by_team[team]=Interval<Point>{min_pts,best_case};
	}

	//this could be tightened up because some teams will have to be captains
	//so those points are not technically up for grabs.
	Point direct=sum(mapf([](auto x){ return get<2>(x); },still_left));
	Point by_team_lim=sum(mapf([](auto x){ return x.max-x.min; },values(r.by_team)));

	r.unclaimed=min(direct,by_team_lim);
	return r;
}

template<typename Team>
auto teams(Rank_range<Team> const& a){
	return keys(a);
}

template<typename Team>
auto teams(Point_range<Team> const& a){
	return keys(a);
}

template<typename Team>
auto teams(Rank_results<Team> const& a){
	auto t=teams(a.ranks);
	auto t2=teams(a.points);
	assert(t==t2);
	return t;
}

Pick_limits pick_limits(TBA_fetcher &f,tba::Event_key const& event,std::map<tba::Team_key,Interval<Rank>> const& ranks){
	auto p=pick_points(f,event,ranks);
	if(std::holds_alternative<Picks_known_empty>(p)){
		Pick_limits r;
		for(auto team:keys(ranks)){
			r.points[team]=0;
			r.picked[team]=0;
		}
		r.unclaimed=0;
		r.status=Event_status::COMPLETE;
		return r;
	}
	if(std::holds_alternative<Picks_no_data>(p)){
		//cout<<"picks: no data\n";
		Pick_limits r;
		//just going to leave everything blank.
		//could try to fill this in by looking at what teams play in the finals
		//but for now, just say that any of the teams might have been picked.
		for(auto team:keys(ranks)){
			r.points[team]=Interval<Point>{0,16};
			r.picked[team]=Interval<bool>{0,1};
		}
		std::vector<int> values;
		for(auto i:reversed(range_inclusive(9,16))){
			values|=i;
			values|=i;
		}
		values|=reversed(range_inclusive(8,1));
		r.unclaimed=sum(take(ranks.size(),values));

		r.status=[&](){
			if(playoffs_started(f,event) || awards_done(f,event) || event_timed_out(f,event)){
				return Event_status::COMPLETE;
			}
			return Event_status::FUTURE;
		}();
		return r;
	}
	if(std::holds_alternative<Picks_complete>(p)){
		auto a=std::get<Picks_complete>(p);
		//cout<<"picks complete\n";
		//print_r(a);
		Pick_limits r;
		for(auto k:keys(ranks)){
			r.points[k]=0;
			r.picked[k]=0;
		}
		for(auto [k,v]:a){
			r.points[k]=v;
			r.picked[k]=(v!=0);
		}
		r.unclaimed=0;
		r.status=Event_status::COMPLETE;
		return r;
	}
	if(std::holds_alternative<Picks_in_progress>(p)){
		//cout<<"picks: in progress\n";

		auto a=std::get<Picks_in_progress>(p);

		if(complete(f,event)){
			Pick_limits r;
			r.points=map_values([](auto x){ return Interval{min(x)}; },a.by_team);

			for(auto [k,v]:r.points){
				r.picked[k]=!!v.min;
			}

			r.unclaimed=0;
			r.status=Event_status::COMPLETE;
			return r;
		}

		Pick_limits r;
		r.points=a.by_team;
		r.unclaimed=a.unclaimed;
		r.status=[&](){
			if(a.started){
				return Event_status::IN_PROGRESS;
			}
			return Event_status::FUTURE;
		}();

		for(auto [k,v]:r.points){
			if(min(v)>0){
				r.picked[k]=1;
			}else if(max(v)>0){
				r.picked[k]=Interval<bool>(0,1);
			}else{
				r.picked[k]=0;
			}
		}
		return r;
	}
	(void)f;
	(void)event;
	(void)ranks;
	nyi
}

int pick_points_demo(TBA_fetcher &f){
	auto run=[&](auto event){
		PRINT(event.key);
		auto in=rank_limits(f,event.key);
		in.check();

		auto t1=teams(in);
		auto t2=teams_keys(f,event);

		//if the TBA call is missing teams, that's fine
		//but we do care about the other direction.
		auto missing=t2-t1;
		assert(missing.empty());

		auto out=pick_points(f,event.key,in.ranks);
		//using Pick_points=std::variant<Picks_no_data,Picks_in_progress,Picks_complete>;
		if(std::holds_alternative<Picks_no_data>(out)){
			return;
		}
		if(std::holds_alternative<Picks_in_progress>(out)){
			auto a=std::get<Picks_in_progress>(out);
			auto s=sum(values(a.by_team));
			auto width=s.max-s.min;
			auto ok=(unsigned(width)>=a.unclaimed);
			if(!ok){
				print_r(a);
				PRINT(width);
				PRINT(a.unclaimed);
				//print_r(a);
				nyi
			}
		}else if(std::holds_alternative<Picks_complete>(out)){
			auto a=std::get<Picks_complete>(out);
			(void)a;
			//print_r(a);
			//nyi
		}else{
			print_r(out);
			nyi
		}
	};
	
	for(auto const& event:events(f)){
		//if(event.key=="202121reg"){
		run(event);
		//}
	}
	return 0;

	//return ranks_from_demo(f);

	Event event("2025orwil");
	auto in=rank_limits(f,event);
	in.check();
	PRINT(entropy(in.ranks));
	for(auto s:sorted(MAP(as_string,reverse_pairs(in.ranks)))){
		cout<<s<<"\n";
		//cout<<v<<"\t"<<k<<"\n";
	}
	//auto data=rand((std::map<Team,Interval<Rank>>*)0);
	auto p=pick_points(f,event,in.ranks);
	PRINT(p);
	return 0;
}
