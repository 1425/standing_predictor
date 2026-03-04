/*goal: for the given team, can they make the district championship, and if it's possible but not certain, what are their odds of making it in, and if they have events left, how many points do they need to earn to get to different levels of probability?

for each team:
	make probability distribution of how many points they might get
combine these to make probability distribution of where the points cutoff will be
foreach team:
	make listing of odds of dcmp w/ each possible # of points still to earn left

initial version could do assuming that all teams are equal
later version could assume that teams have some sort of skill quantified by either their first event, or ELO, or something else.

how to do worlds odds:
hold over some of the data from calculating the make DCMP cutoffs to get distribution for that
do the same distribution of points as a district event -> not good because have larger size & more chairmans winners, etc.
TODO: Find table of # of chairman's winners per DCMP
Engineering inspiration (#?)
Rookie All-Star (probability that given out?)
district championship winners -> just assume that they would have enough points anyway?

General outline of how to deal w/ California:
1) Teams become not just a # of points but also a tag of which dcmp they are eligible for
2) the point distributions after each event become two distributions, one for each tag

simple way: 
1) ignore going to cmp, just look at the halves as if they were individual w/ out of state plays
2) assume equal # of teams from each side of the DCMP makes it to cmp.
*/

#include<filesystem>
#include "../tba/data.h"
#include "../tba/tba.h"
#include "arguments.h"
#include "event.h"
#include "set.h"
#include "tba.h"
#include "util.h"
#include "status.h"
#include "run.h"
#include "outline.h"
#include "skill.h"
#include "skill_opr.h"
#include "print_r.h"
#include "vector_void.h"
#include "optional.h"
#include "plot.h"
#include "lock.h"
#include "rank_limits.h"
#include "award_limits.h"
#include "playoff_limits.h"
#include "timezone.h"
#include "venue.h"
#include "event_limits.h"
#include "event_partial.h"
#include "toggle.h"
#include "data_range.h"
#include "vector_fixed.h"
#include "output.h"

//start program-specific stuff.

using namespace std;

using District_key=tba::District_key;
using Team=tba::Team_key;
using Team_key=tba::Team_key;
using Year=tba::Year;
using Date=tba::Date;

using Points_used=map<tba::Team_key,Team_points_used>;

std::tuple<Run_result,Points_used,By_team,Skill_estimates,Annotated,std::map<tba::Team_key,std::string>> run_inner(
	TBA_fetcher &f,
	bool ignore_chairmans,
	tba::District_key district,
	tba::Year year,
	Skill_method skill_method,
	bool quick
){
	bool dcmp_played=0;

	const auto pr=flat_map2(historical_event_pts(f));
	const auto chairmans=[&](){
		if(ignore_chairmans){
			return set<tba::Team_key>{};//chairmans.clear();
		}
		try{
			return chairmans_winners(f,district);
		}catch(tba::Decode_error const& a){
			//Known to be happening for the event described at:
			//https://www.thebluealliance.com/event/2025tempclone-356125237
			//Might want to have the error be returned somehow rather than going on.
			cerr<<"Warning: Failed to find list of chairmans winners for "<<district<<"\n";
			return set<tba::Team_key>{};
		}
	}();

	auto d1=[&](){
		auto d=district_rankings(f,district);
		assert(d);
		return *d;
	}();

	{
		//if the team isn't scheduled for any events this year, ignore them.
		set<tba::Team_key> not_going;
		for(auto team:d1){
			auto e=team_events_year_keys(f,team.team_key,year);
			if(e.empty()){
				not_going|=team.team_key;
			}
		}
		//PRINT(not_going);
		d1=filter([&](auto x){ return not_going.count(x.team_key)==0; },d1);
	}

	auto skills=skill_estimates(f,district,skill_method);

	By_team by_team;
	Points_used points_used;
	for(auto team:d1){
		auto max_counters=2-int(team.event_points.size());
		auto events_scheduled=team_events_year_keys(f,team.team_key,year);

		auto events_left=min(max_counters,int(events_scheduled.size())-int(team.event_points.size()));
		auto dist=[&]()->Team_dist{
			auto first_event_points=[=]()->double{
				if(team.event_points.size()){
					return team.event_points[0].total;
				}
				return 0;
			}();
			if(events_left<0){
				//then we are post-district championship
				//-1=played in a normal district championship or just 1 field of a multi-field one
				//-2=played in a multi-field DCMP & did something on the joint field
				dcmp_played=1;
				return Team_dist{{
					sum(::mapf([](auto x){ return x.total; },team.event_points)),
					1
				}};
			}
			if(events_left==0){
				return Team_dist{{
					first_event_points+[=]()->double{
						if(team.event_points.size()>1){
							return team.event_points[1].total;
						}
						return 0;
					}(),
					1
				}};
			}
			if(events_left==1){
				return Team_dist(mapf(
					[&](auto p){
						return make_pair(Point(p.first+first_event_points),p.second);
					},
					pr
				));
			}
			if(events_left==2){
				return get_key(skills.pre_dcmp,team.team_key);
			}
			PRINT(team);
			PRINT(events_left);
			nyi
		}()+team.rookie_bonus;

		{
			auto accounted_pts=sum(mapf([](auto x){ return x.total; },team.event_points))+team.rookie_bonus;
			if(accounted_pts!=team.point_total){
				std::cout<<"Discrepancy:"<<team.team_key<<" "<<accounted_pts<<" "<<team.point_total<<"\n";
			}
			//assert(accounted_pts==team.point_total);
		}

		points_used[team.team_key]=Team_points_used(
			::mapf([](auto x){ return Point(x.total); },team.event_points),
			team.rookie_bonus,
			events_left,
			skills.pre_dcmp[team.team_key]
		);
		Dcmp_home dcmp_home=calc_dcmp_home(f,team.team_key);
		by_team[team.team_key]=Team_status(chairmans.count(team.team_key),std::move(dist),dcmp_home,team.point_total);
	}

	//Run_input run_input=read_status(f,district,skill_method);
	auto [run_input,skill,annotated,extra]=read_status(f,district,skill_method);
	run_input.quick=quick;

	return make_tuple(
		run_calc(run_input),
		points_used,
		by_team,
		skill,
		annotated,
		extra
	);
}

struct Run_inputs{
	std::string output_dir; //output
	std::optional<tba::District_key> district; //data in
	std::optional<tba::Year> year; //data in & out
	std::vector<int> dcmp_size; //how
	string title; //output
	string district_short; //output
	std::string extra=""; //output
	bool ignore_chairmans=0;
	Skill_method skill_method=Skill_method::NONE;
	bool plot=0;
	bool quick=0;
};

map<tba::Team_key,Pr> run(
	TBA_fetcher& f,
	Run_inputs inputs
){
	//this function exists to separate the input & calculation from the output
	const auto district=*inputs.district;
	const auto year=*inputs.year;
	auto [results,points_used,by_team,skill,annotated,extra]=run_inner(f,inputs.ignore_chairmans,district,year,inputs.skill_method,inputs.quick);

	auto team_info=district_teams(f,district);
	{
		Gen_html_input ghi(year);
		ghi.result=results.result;
		ghi.team_info=team_info;
		ghi.dcmp_cutoff_pr=results.cutoff_pr;
		ghi.cmp_cutoff_pr=to_map(results.cmp_cutoff_pr);
		ghi.title=inputs.title;
		ghi.district_short=inputs.district_short;
		ghi.dcmp_size=inputs.dcmp_size;
		ghi.points_used=points_used;
		ghi.plot=inputs.plot;
		ghi.lock=::lock(f,district);
		ghi.skill=skill;
		ghi.worlds_slots=worlds_slots(district);
		ghi.extra=extra;
		ofstream file(inputs.output_dir+"/"+district.get()+inputs.extra+".html");
		gen_html(file,ghi,annotated);
	}

	return to_map(::mapf(
		[](auto x){
			return make_pair(x.team,x.dcmp_make);
		},
		results.result
	));
}

struct Args{
	string output_dir=".";
	tba::Year year{2022};
	optional<tba::District_key> district;
	TBA_fetcher_config tba;
	bool demo=0,rank_limits_demo=0;
	bool award_limits_demo=0;
	bool playoff_limits_demo=0;
	bool timezone_demo=0;
	bool lock=0;
	bool venue_demo=0;
	bool event_limits_demo=0;
	bool event_partial_demo=0;
	bool data_range_demo=0;
	bool plot=1;
	bool quick=0;
	Skill_method skill_method=Skill_method::POINTS;
};

Args parse_args(int argc,char **argv){
	Args r;
	Argument_parser p{"Calculates odds of FRC teams advancing to their district championships and the championship event."};
	p.add(
		"--out",{"PATH"},
		"Directory in which to store result files",
		r.output_dir
	);
	p.add(
		"--year",{"YEAR"},
		"For which year the predictions should be made",
		r.year
	);
	p.add(
		"--district",{"KEY"},
		"Examine only a specific district",
		r.district
	);
	r.tba.add(p);
	p.add(
		"--demo",{},
		"Explore data for possible future use",
		r.demo
	);
	p.add(
		"--skill",{"METHOD"},
		"How to measure skill of individual teams.  Options:\n\t\tPOINTS (default)\n\t\tOPR\n\t\tNONE",
		r.skill_method
	);
	p.add(
		"--rank_limits_demo",{},
		"Attempt to find limits of pre-alliance selection ranks.  Experimental.",
		r.rank_limits_demo
	);
	p.add(
		"--award_limits_demo",{},
		"Attempt to predict award points at an event.  Experimental.",
		r.award_limits_demo
	);
	p.add(
		"--playoff_limits_demo",{},
		"Attempt to predict playoff pts.  Experimental.",
		r.playoff_limits_demo
	);
	p.add(
		"--timezone_demo",{},
		"Run timezone demo.  Experimental.",
		r.timezone_demo
	);
	p.add(
		"--lock",{},
		"Attempt to calculate with 100% certainty.  Experimental.",
		r.lock
	);
	p.add("--venue_demo",{},
			"Experimental",r.venue_demo
	);
	p.add("--event_limits_demo",{},"Experimental",r.event_limits_demo);
	p.add("--event_partial_demo",{},"Experimental",r.event_partial_demo);
	p.add("--data_range_demo",{},"Experimental",r.data_range_demo);
	p.add(
		"--plot",{"ENABLE"},
		"Include plots of point distributions in output.  Defauls to true.",
		r.plot
	);
	p.add(
		"--quick",{"ENABLE"},
		"Do smaller number of iterations",
		r.quick
	);
	p.parse(argc,argv);
	return r;
}

tba::Date cmp_end(TBA_fetcher& f,Year year){
	auto found=filter(
		[](auto x){
			return x.event_type==tba::Event_type::CMP_DIVISION
				|| x.event_type==tba::Event_type::CMP_FINALS;
		},
		events(f,year)
	);
	auto m=mapf([](auto x){ return x.end_date; },found);
	auto s=to_set(nonempty(m));
	if(s.size()==1){
		return *begin(s);
	}
	assert(!s.empty());
	return max(s);
	auto m1=count(nonempty(m));
	PRINT(m1);
	print_lines(found);
	nyi
}

void event_dates(TBA_fetcher &f,tba::Event const& e){
	//print_r(e);
	//PRINT(e.start_date)
	//PRINT(e.end_date)

	auto matches=event_matches(f,e.key);
	if(matches.empty()){
		return;
	}

	for(auto match:matches){
		print_r(match);
		//match.time;
		auto t=match.actual_time;
		PRINT(t);
		//match.predicted_time;
		auto t2=match.post_result_time;
		(void)t2;
	}
	nyi
}

void event_dates(TBA_fetcher &f,tba::Event_key const& e){
	auto data=event(f,e);
	print_r(data);
	nyi
}

auto identify_time(TBA_fetcher &f){
	std::map<District_key,std::set<Date>> r;
	for(auto [district_name,v]:normal_district_years(f)){
		for(auto year:v){
			tba::District_key k(::as_string(year)+district_name);
			//find events
			auto d=district_events(f,k);
			if(d.empty()){
				continue;
			}
			assert(!d.empty());

			for(auto x:d){
				event_dates(f,x);
			}

			auto g=group([](auto x){ return x.event_type; },d);
			const auto start_date=min(nonempty(mapf(
				[](auto x){ return x.start_date; },
				g[tba::Event_type::DISTRICT]
			)));

			auto event_ends=to_set(nonempty(mapf([](auto x){ return x.end_date; },g[tba::Event_type::DISTRICT])));
			
			auto dcmps=g[tba::Event_type::DISTRICT_CMP];
			auto dcmp_ends=to_set(nonempty(mapf([](auto x){ return x.end_date; },dcmps)));

			set<Date> interesting_dates;
			interesting_dates|=start_date;
			interesting_dates|=event_ends;
			interesting_dates|=dcmp_ends;

			auto cmp1=cmp_end(f,year);
			(void)cmp1;

			r[k]=interesting_dates;
		}
	}
	return r;
}

int identify_time_demo(TBA_fetcher &f){
	auto x=identify_time(f);
	print_r(x);

	/* For each date:
	 * what events are in the past, current and future
	 * and what season is it?
	 *
	 * */

	return 0;
}

int main1(int argc,char **argv){
	auto args=parse_args(argc,argv);
	std::filesystem::create_directories(args.output_dir);
	auto tba_fetcher=args.tba.get();

	//return identify_time_demo(tba_fetcher);
	//return dates_demo(tba_fetcher);
	//return lock_demo(tba_fetcher);

	if(args.venue_demo){
		return venue_demo(tba_fetcher);
	}

	if(args.rank_limits_demo){
		rank_limits_demo(tba_fetcher);
		//pick_points_demo(tba_fetcher);
		return 0;
	}

	if(args.award_limits_demo){
		return award_limits_demo(tba_fetcher);
	}

	if(args.playoff_limits_demo){
		return playoff_limits_demo(tba_fetcher);
	}

	if(args.timezone_demo){
		return timezone_demo(tba_fetcher);
	}

	if(args.demo){
		return demo(tba_fetcher);
	}

	if(args.lock){
		return run_lock(tba_fetcher,args.year,args.district);
	}

	if(args.event_limits_demo){
		return event_limits_demo(tba_fetcher);
	}

	if(args.event_partial_demo){
		return toggle_demo();
		return event_partial_demo(tba_fetcher);
	}

	if(args.data_range_demo){
		return data_range_demo(tba_fetcher);
	}

	auto d=districts(tba_fetcher,args.year);

	map<tba::District_key,map<tba::Team_key,Pr>> dcmp_pr;

	for(auto year_info:d){
		auto district=year_info.key;
		if(args.district && district!=args.district){
			continue;
		}
		//PRINT(district);
		auto title=year_info.display_name+" District Championship Predictions "+::as_string(args.year);
		Run_inputs run_inputs;
		run_inputs.output_dir=args.output_dir;
		run_inputs.district=district;
		run_inputs.year=args.year;
		run_inputs.dcmp_size=dcmp_size(district);
		run_inputs.title=title;
		run_inputs.district_short=year_info.abbreviation;
		run_inputs.skill_method=args.skill_method;
		run_inputs.plot=args.plot;
		run_inputs.quick=args.quick;
		//dcmp_pr[district]=run(tba_fetcher,args.output_dir,district,args.year,dcmp_size(district),title,year_info.abbreviation);
		dcmp_pr[district]=run(tba_fetcher,run_inputs);

		if(district=="2022ne"){
			run_inputs.dcmp_size=[](){
				std::vector<int> r;
				r|=16;
				return r;
			}();
			run_inputs.title="New England Championship Pre-Qualify";
			run_inputs.extra="_cmp";
			run_inputs.ignore_chairmans=1;
			run(tba_fetcher,run_inputs);
		}
	}

	make_spreadsheet(tba_fetcher,dcmp_pr,args.output_dir);

	return 0;
}

int main(int argc,char **argv){
	try{
		return main1(argc,argv);
	}catch(std::string const& s){
		cerr<<"Caught:"<<s<<"\n";
		return 1;
	/*}catch(std::invalid_argument const& e){
		cerr<<"Caught:"<<e<<"\n";
		return 1;*/
	}catch(std::vector<std::string> const& v){
		cerr<<"Caught:"<<v<<"\n";
		return 1;
	}catch(const char *s){
		cerr<<"Caught:"<<s<<"\n";
		return 1;
	}catch(tba::Decode_error const& a){
		cerr<<"Caught:"<<a<<"\n";
		return 1;
	}
}
