#include "lock.h"
#include "../tba/data.h"
#include "../tba/tba.h"
#include "util.h"
#include "io.h"
#include "rand.h"
#include "tba.h"
#include "print_r.h"
#include "rank_pts.h"
#include "interval.h"
#include "optional.h"
#include "int_limited.h"
#include "set_fixed.h"
#include "vector_fixed.h"
#include "event.h"
#include "venue.h"
#include "names.h"
#include "map_auto.h"

/*
 * would be good to check some of the assumptions made in here with data
 * also, would be nice to check that some of the the limits...
 *
 * TODO: Team ranking result limits
1) make the row highlighting work
2) see about cleaning up how the data is pulled
3) overall point totals avaible always 0
4) pts available for finished events should be 0.
 * */

using District_key=tba::District_key;
using Team=tba::Team_key;
using Event=tba::Event_key;
using Event_key=tba::Event_key;
using Year=tba::Year;

using namespace std;

using Age_bonus=Int_limited<0,10>;//not encoding that it can only be 0,5,10

//using I2=Int_limited<0,200>;
///using I2=vector_fixed<Int_limited<0,200>,2>;
using I2=vector_fixed<Point,2>;

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

using Info_by_team=map_auto<Team,Team_info>;

using Event_size=Int_limited<0,80>;

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

Event_info read_event_info(TBA_fetcher &f,tba::Event_key const& event){
	auto aw=event_awards(f,event);
	auto c=count_if([](auto x){ return x.award_type==tba::Award_type::CHAIRMANS; },aw);
	if(c!=0){
		return Event_finished();
	}
	if(!chairmans_expected(f,event)){
		//then look at the matches and see if they look done.
		if(matches_complete(f,event)){
			return Event_finished();
		}
	}
	
	if(event_timed_out(f,event)){
		return Event_finished();
	}

	return event_size(f,event);
}

bool done(TBA_fetcher &f,tba::Event_key const& event){
	auto a=read_event_info(f,event);
	return std::holds_alternative<Event_finished>(a);
}

bool done(TBA_fetcher &f,tba::Event const& event){
	return done(f,event.key);
}

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

//returns a list with each item representing the status of one of the district events.
vector<Lock_data> read_lock_data(TBA_fetcher &f,tba::District_key const& district){
	map<Dcmp_home,Lock_data> r1;
	//Lock_data r;
	auto a=district_rankings(f,district);
	assert(a);
	for(auto x:*a){
		//print_r(x);
		Team_info team_info;

		//could change this to look only at chairmans awards that are at the counting district events
		team_info.won_chairmans=won_chairmans(f,year(district),x.team_key);

		auto& at_event=team_info.district_event_points_earned;
		for(auto y:x.event_points){
			if(!y.district_cmp && at_event.size()<2){
				at_event|=y.total;
			}
		}
		//team_info.district_event_points_earned=at_event;
		team_info.age_bonus=x.rookie_bonus;

		//obviously, want to actually look this up later.
		team_info.remaining_district_events=at_event.size()<2;

		auto dcmp=calc_dcmp_home(f,x.team_key);
		r1[dcmp].by_team[x.team_key]=team_info;
	}

	for(auto event:district_events(f,district)){
		if(
			event.event_type==tba::Event_type::DISTRICT_CMP
			|| event.event_type==tba::Event_type::DISTRICT_CMP_DIVISION
		){
			continue;
		}
		//PRINT(event.event_type);
		assert(event.event_type==tba::Event_type::DISTRICT);

		for(auto & [k,r]:r1){
			r.by_event[event.key]=read_event_info(f,event.key);
		}
	}
	
	auto d=dcmp_size(district);
	/*Notes on dealing with California 2026:
	For now, just going super-conservative and calculating it twice, each 
	with half the teams but 2x the number of events

	 -could restrict the number of pts available at each event in this case based on the number of teams
	   from that half of the district that are attending it -> something like min(total pts,83*# of teams)
	 * */
	for(auto [i,v]:enumerate(d)){
		auto f=r1.find(i);
		assert(f!=r1.end());
		f->second.dcmp_size=v;
	}
	return values(r1);
}

int event_points(size_t event_size){
	//This is for a normal district event, not a district championship, or a district championship division.

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

struct Status_prequalified{
	//could put a reason in here
	//this isn't meaningful until cmp anyway.

	auto operator<=>(Status_prequalified const&)const=default;
};

std::ostream& operator<<(std::ostream& o,Status_prequalified const&){
	return o<<"Prequalified";
}

struct Status_award{
	tba::Award_type data;

	auto operator<=>(Status_award const&)const=default;
};

std::ostream& operator<<(std::ostream& o,Status_award a){
	return o<<"In via "<<a.data;
}

struct Status_in{
	auto operator<=>(Status_in const&)const=default;
};

std::ostream& operator<<(std::ostream& o,Status_in){
	return o<<"in";
}

struct Status_out{
	auto operator<=>(Status_out const&)const=default;
};

std::ostream& operator<<(std::ostream& o,Status_out){
	return o<<"out";
}

struct Status_in_range{
	std::string data;

	auto operator<=>(Status_in_range const&)const=default;
};

std::ostream& operator<<(std::ostream& o,Status_in_range const& a){
	return o<<a.data;
}

struct Status_out_of_range{
	auto operator<=>(Status_out_of_range const&)const=default;
};

std::ostream& operator<<(std::ostream& o,Status_out_of_range){
	return o<<"Out of range";
}

using Status=std::variant<
	Status_prequalified, //not used yet
	Status_award, //not used yet
	Status_in,
	Status_out,
	Status_in_range,
	Status_out_of_range
>;

#define LOCK_STATUS(X)\
	X(Status_prequalified,"#ff00ff")\
	X(Status_award,"#4444ff")\
	X(Status_in,"#00aa00")\
	X(Status_out,"#777777")\
	X(Status_in_range,"#aaffaa")\
	X(Status_out_of_range,"#ff4444")\

std::string color(Status const& a){
	#define X(A,B) if(std::holds_alternative<A>(a)) return B;
	LOCK_STATUS(X)
	#undef X
	assert(0);
}

using Status_by_team=map<Team,Status>;
using Points_by_event=map<tba::Event_key,Point>;

#define LOCK_RESULT(X)\
	X(Status_by_team,by_team)\
	X(Point,pre_dcmp_points_remaining)\
	X(Points_by_event,by_event)

struct Lock_result{
	LOCK_RESULT(INST)
};

std::ostream& operator<<(std::ostream& o,Lock_result const& a){
	o<<"Lock_result(";
	#define X(A,B) o<<""#B<<":"<<a.B<<" ";
	LOCK_RESULT(X)
	#undef X
	return o<<")";
}

void print_r(int n,Lock_result const& a){
	indent(n);
	cout<<"Lock_result\n";
	n++;
#define X(A,B) indent(n); cout<<""#B<<"\n"; print_r(n+1,a.B);
	LOCK_RESULT(X)
#undef X
}

Lock_result run(Lock_data const& data){
	//print_r(data);

	using Rank_item=std::pair<unsigned short,Point>;
	Lock_result r;

	Rank_item left_to_claim;
	for(auto [k,v]:data.by_event){
		if(std::holds_alternative<Event_upcoming>(v)){
			auto a=std::get<Event_upcoming>(v);
			left_to_claim.first+=1;
			left_to_claim.second+=event_points(a);
			r.by_event[k]=event_points(a);
		}else{
			r.by_event[k]=0;
		}
	}
	//PRINT(left_to_claim);

	/*
	TODO: Figure out how we want to deal with the fact that we're expecting to get a non-zero number of points
	even if you come in last at the event.

	This means we need to look up from the event size what the limit on pts is
	Might also want to calculate the maximum pts that can get but theoretically a team could win the chairmans award if there are an events left
	which effectively means infinite points.

	TODO: Group the teams by # of points so far, that way don't accedentily treat teams differently based on 
	where they happen to sit when they have equal points.
	*/
	map<Rank_item,vector<tuple<Team,Team_info>>> by_pts;
	for(auto [team,d]:data.by_team){
		auto points=sum(d.district_event_points_earned)+d.age_bonus;
		Rank_item ri(d.won_chairmans,points);
		by_pts[ri]|=make_tuple(team,d);
	}
	//std::sort(pts.begin(),pts.end());
	//vector<tuple<Rank_item,std::vector<tuple<Team,Team_info>>>> pts;
	auto pts=reversed(sorted(by_pts));

	//print_r(pts);

	/*cout<<"Current lineup:\n";
	print_lines(pts);
	cout<<"\n";*/

	//using Marker=std::string;
	map<Team,Status> markers;

	//starting by ignoring awards.
	size_t teams_ranked=0;
	for(auto [i,t]:enumerate(pts)){
		//PRINT(i)
		//PRINT(t);

		auto [n,teams_here]=t;
		//cout<<"\t"<<n<<"\t"<<teams_here.size()<<"\n";

		int needed_passes=(int)data.dcmp_size-(int)teams_ranked-(int)teams_here.size()+1;

		if(needed_passes>0){
			//currently in range to go
			int found=0;//teams found that might be able to pass/tie
			Rank_item rank_total;
			//PRINT(rank_total)

			enum Reason{NONE,FOUND_ENOUGH,OUT_OF_POINTS};
			int reason=NONE;
			for(
				size_t j=i+1;
				j<pts.size() //found<needed_passes && both_less_eq(rank_total,left_to_claim)
				;j++
			){
				auto [rank_next,teams]=pts[j];
				auto diff=n-rank_next;
				if(diff.second<0){
					diff.second=0;
				}
				//PRINT(diff);

				size_t found_here=0;
				for(auto [team,team_info]:teams){
					if(team_info.remaining_district_events){
						found_here++;
					}
				}
				
				while(found_here && found<needed_passes && both_less_eq(rank_total+diff,left_to_claim)){
					rank_total+=diff;
					found++;
					found_here--;
				}

				if(found>=needed_passes){
					reason=FOUND_ENOUGH;
					break;
				}

				if(!both_less_eq(rank_total+diff,left_to_claim)){
					//Would require too many points.
					reason=OUT_OF_POINTS;
					break;
				}

				if(!found_here){
					//need to go look for the next set of teams
					continue;
				}

				assert(0);

				/*rank_total+=diff*found_here;
				found+=found_here;*/
				//need to calculate effort to have at least 1 pass vs the effort to have all of them pass
			}

			Status status=[&]()->Status{
				switch(reason){
					case NONE:
						if(found){
							nyi
						}else{
							//ran out of teams to pass
							return Status_in{};
						}
					case FOUND_ENOUGH:{
						std::stringstream ss;
						double x=0;
						if(left_to_claim.second){
							x=100*float(rank_total.second)/left_to_claim.second;
						}
						//ss<<rank_total<<" "<<left_to_claim;
						ss<<"In range: "<<x<<"%";
						return Status_in_range{ss.str()};
					}
					case OUT_OF_POINTS:
						return Status_in{};
					default:
						assert(0);
				}
			}();
			for(auto team:firsts(teams_here)){
				markers[team]=status;
			}
		}else{
			int unclaimed_slots=(int)data.dcmp_size-(int)teams_ranked;
			for(auto [team,info]:teams_here){
				if(unclaimed_slots>0){
					//there are slots available that could theoretically be claimed
					//without earning any more points.
					markers[team]=Status_in_range{"0"};
				}else{
					//currently in range to miss out
					if(info.remaining_district_events && left_to_claim.second){
						if(left_to_claim.second){
							PRINT(left_to_claim);
							nyi
						}
						nyi
						//then always to possibility to win
						//could calculate minimum of what would be needed to get in range
						//and also could calculate what it would take to get to a lock.
						markers[team]=Status_out_of_range();
					}else{
						//then you're out
						markers[team]=Status_out{};
					}
				}
			}
		}
		teams_ranked+=teams_here.size();
	}
	//PRINT(markers);
	r.by_team=markers;
	r.pre_dcmp_points_remaining=left_to_claim.second;
	return r;
}

#define EVENT_DISPLAY(X)\
	X(std::string,name)\
	X(std::string,status)\
	X(size_t,teams)\
	X(std::string,pts_available)\
	X(Interval<tba::Date>,date)\

struct Event_display{
	EVENT_DISPLAY(INST)
};

std::ostream& operator<<(std::ostream& o,Event_display const& a){
	o<<"Event_display( ";
	#define X(A,B) o<<""#B<<":"<<a.B<<" ";
	EVENT_DISPLAY(X)
	#undef X
	return o<<")";
}

using Team_district_display=vector_fixed<Point,2>;

#define TEAM_DISPLAY(X)\
	X(std::string,team)\
	X(Team_district_display,districts)\
	X(Point,age_bonus)\
	X(Point,dcmp_pts)\
	X(Point,total_pts)\
	X(Status,locked)

struct Team_display{
	TEAM_DISPLAY(INST)
};

std::ostream& operator<<(std::ostream& o,Team_display const& a){
	o<<"Team_display( ";
	#define X(A,B) o<<""#B<<":"<<a.B<<" ";
	TEAM_DISPLAY(X)
	#undef X
	return o;
}

#define LOCK_DISPLAY(X)\
	X(std::string,district)\
	X(int,pre_dcmp_points_remaining)\
	X(int,dcmp_size)\
	X(int,total_points_remaining)\
	X(int,available_champs_spots)\
	X(std::vector<Event_display>,event_status)\
	X(std::vector<Team_display>,team_display)\

struct Lock_display{
	LOCK_DISPLAY(INST)
};

std::ostream& operator<<(std::ostream& o,Lock_display const& a){
	o<<"Lock_display( ";
	#define X(A,B) o<<""#B<<":"<<a.B<<" ";
	LOCK_DISPLAY(X)
	#undef X
	return o<<")";
}

PRINT_R_ITEM(Lock_display,LOCK_DISPLAY)

void page(std::ostream& o,Lock_display const& a){
	o<<"<html>\n";
	o<<"<head>\n";
	auto title1=a.district+" locks";
	o<<title(title1);
	o<<"</head>\n";
	o<<"<body>\n";
	o<<h1(title1);
	o<<tag("table border",
		tr(td("Pre-DCMP points remaining")+td(a.pre_dcmp_points_remaining))+
		tr(td("District championship size")+td(a.dcmp_size))+
		tr(td("Total points remaining")+td(a.total_points_remaining))+
		tr(td("Available champs spots")+td(a.available_champs_spots))
	);
	o<<h2("Events");
	o<<tag("table border",
		tr(th("Name")+th("Status")+th("Date")+th("Teams")+th("Points availble"))+
		join(mapf(
			[](auto const& x){
				return tr(td(x.name)+td(x.status)+td(x.date)+td(x.teams)+td(x.pts_available));
			},
			a.event_status
		))
	);
	o<<h2("Teams");
	o<<tag("table border",
		tr(
			th("Rank")+
			th("Team")+
			th("District 1")+
			th("District 2")+
			th("Age bonus")+
			th("DCMP pts")+
			th("Total pts")+
			th("Locked?")
		)+
		join(mapf(
			[](auto p){
				auto [i,x]=p;
				auto event=[=](size_t i)->string{
					if(i>=x.districts.size()){
						return "-";
					}
					return as_string(x.districts[i]);
				};
				return tag("tr bgcolor=\""+color(x.locked)+"\"",
					td(i)+
					td(x.team)+
					td(event(0))+
					td(event(1))+
					td(x.age_bonus)+
					td(x.dcmp_pts)+
					td(x.total_pts)+
					td(x.locked)
				);
			},
			enumerate_from(1,a.team_display)
		))
	);
	o<<"</body>\n";
	o<<"</html>\n";
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

int lock_demo(TBA_fetcher& f){
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

Point event_pts(TBA_fetcher &f,tba::Event_key const& event){
	//PRINT(event);
	auto e=tba::event(f,event);
	//print_r(e);
	//PRINT(e.event_type);

	assert(e.event_type==tba::Event_type::DISTRICT_CMP || e.event_type==tba::Event_type::DISTRICT_CMP_DIVISION);

	//TODO: Make it so that looks at the number of teams that are expected at the event rather than the
	//number that are currently qualified

	auto size=tba::event_teams(f,event).size();
	return event_points(size)*3;
}

Point event_pts(TBA_fetcher &f,tba::Event const& event){
	return event_pts(f,event.key);
}

Point dcmp_points(TBA_fetcher &f,tba::District_key const& district){
	auto found=filter(
		[&](auto x){
			return (x.event_type==tba::Event_type::DISTRICT_CMP || 
				x.event_type==tba::Event_type::DISTRICT_CMP_DIVISION)
				&& !done(f,x);
		},
		tba::district_events(f,district)
	);
	return sum(mapf([&](auto const& x){ return event_pts(f,x); },found));
}

void show_lock_data(TBA_fetcher &f,tba::District_key const& district,Dcmp_home dcmp_home,Lock_data const& in,Lock_result out){
	Lock_display data;
	//data.district=parse_event_name(f,display_name(f,district))+" "+as_string((int)dcmp_home);
	data.district=name(f,district)+" "+as_string((int)dcmp_home);
	data.pre_dcmp_points_remaining=out.pre_dcmp_points_remaining;
	data.total_points_remaining=data.pre_dcmp_points_remaining+dcmp_points(f,district);
	data.available_champs_spots=worlds_slots(district);

	data.dcmp_size=in.dcmp_size;

	for(auto x:district_events(f,district)){
		//print_r(ii);
		Event_display h;
		//print_r(x);
		h.name=nice_name(x);
		h.status=[&]()->string{
			auto it=in.by_event.find(x.key);
			if(it!=in.by_event.end()){
				auto ii=it->second;
				return std::holds_alternative<Event_finished>(ii)?"done":"not done";
			}
			return ::as_string(read_event_info(f,x.key));
			//return "no data";
		}();

		h.teams=event_teams(f,x.key).size();

		h.pts_available=[&]()->string{
			auto it=out.by_event.find(x.key);
			if(it==out.by_event.end()){
				//dcmp or something.
				if(done(f,x.key)){
					return "0";
				}else{
					return "?"+::as_string(event_pts(f,x.key));
				}
			}
			return ::as_string(it->second);
		}();

		assert(x.start_date);
		assert(x.end_date);
		h.date=Interval<tba::Date>{*x.start_date,*x.end_date};

		data.event_status|=h;
	}
	data.event_status=sort_by(data.event_status,[](auto x){ return x.date.max; });

	for(auto [team,mark]:out.by_team){
		Team_display here;
		//here.lock_status=Lock_status::Clinched;//TODO: Set this correctly
		here.team=::as_string(team);
		auto ih=in.by_team[team];
		here.districts=ih.district_event_points_earned;
		here.age_bonus=ih.age_bonus;
		here.dcmp_pts=0;
		here.total_pts=sum(here.districts)+here.age_bonus+here.dcmp_pts;
		here.locked=mark;
		data.team_display|=here;
	}
	data.team_display=sort_by(data.team_display,[](auto x){ return -x.total_pts; });
	
	//PRINT(data);
	{
		ofstream file(string()+"lock_"+::as_string(district)+::as_string((int)dcmp_home)+".html");
		page(file,data);
	}

}

int run_lock(TBA_fetcher &f,tba::District_key const& district){
	//PRINT(district);

	//it might be cleaner if all the data lookups happened here before calling run.
	vector<Lock_data> in=read_lock_data(f,district);
	for(auto [i,in1]:enumerate(in)){
		auto out=run(in1);
		show_lock_data(f,district,i,in1,out);
	}

	return 0;
}

int run_lock(TBA_fetcher &f,tba::Year const& year,std::optional<tba::District_key> const& district){
	if(district){
		return run_lock(f,*district);
	}
	for(auto district:keys(districts(f,year))){
		int r=run_lock(f,district);
		if(r){
			return r;
		}
	}
	return 0;
}
