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

#include<fstream>
#include<sstream>
#include<set>
#include<span>
#include<filesystem>
#include "../tba/db.h"
#include "../tba/data.h"
#include "../tba/tba.h"
#include "arguments.h"
#include "event.h"
#include "map.h"
#include "output.h"
#include "set.h"
#include "tba.h"
#include "util.h"
#include "flat_map.h"
#include "flat_map2.h"
#include "multiset_flat.h"
#include "status.h"
#include "run.h"
#include "ca.h"
#include "outline.h"
#include "skill.h"
#include "skill_opr.h"
#include "print_r.h"

//start generic stuff

template<typename K,typename V,typename H>
std::vector<std::pair<K,V>> sorted(std::unordered_map<K,V,H> const& a){
	std::vector<std::pair<K,V>> v(a.begin(),a.end());
	return sorted(v);
}

struct hash_pair{
	template<typename A,typename B>
	size_t operator()(std::pair<A,B> const& a)const{
		auto h1=std::hash<A>{}(a.first);
		auto h2=std::hash<B>{}(a.second);
		return h1^h2;
	}
};

template<typename K,typename V>
auto get_key(std::map<K,V> const& a,K const& k){
	auto f=a.find(k);
	assert(f!=a.end());
	return f->second;
}

//start program-specific stuff.

using namespace std;

California_region california_region(tba::Team const& team){
	if(team.postal_code){
		return california_region(Zipcode(*team.postal_code));
	}
	if(team.city){
		return california_region(City(*team.city));
	}
	assert(0);
}

auto california_region(tba::Event const& event){
	if(event.postal_code){
		return california_region(Zipcode(*event.postal_code));
	}
	PRINT(event);
	nyi
}

std::map<Point,Pr> operator+(std::map<Point,Pr> a,int i){
	std::map<Point,Pr> r;
	for(auto [k,v]:a){
		r[k+i]=v;
	}
	return r;
}

flat_map<Point,Pr> operator+(flat_map<Point,Pr> const& a,int i){
	flat_map<Point,Pr> r;
	for(auto [k,v]:a){
		r[k+i]=v;
	}
	return r;
}

flat_map2<Point,Pr> operator+(flat_map2<Point,Pr> const& a,int i){
	flat_map2<Point,Pr> r;
	//this is not an efficient way to do this with this data structure
	//should make a copy and then modify each of the keys
	for(auto [k,v]:a){
		r[k+i]=v;
	}
	return r;
}

/*using Result_tuple=tuple<tba::Team_key,Pr,Point,Point,Point,Pr,Point,Point,Point>;

using Team_dist=flat_map2<Point,Pr>;

struct Run_result{
	std::vector<Result_tuple> result;
	flat_map2<std::pair<Point,double>,double> cutoff_pr,cmp_cutoff_pr;
	std::map<tba::Team_key,std::tuple<std::vector<int>,int,int>> points_used;
	map<tba::Team_key,pair<bool,Team_dist>> by_team;
};*/

using Points_used=map<tba::Team_key,tuple<vector<int>,int,int>>;

std::tuple<Run_result,Points_used,By_team> run_inner(
	TBA_fetcher &f,
	bool ignore_chairmans,
	tba::District_key district,
	tba::Year year,
	std::vector<int> dcmp_size,
	Skill_method skill_method
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

	auto skills=[&]()->std::optional<Skill_estimates>{
		switch(skill_method){
			case Skill_method::POINTS:
				return calc_skill(f,district);
			case Skill_method::OPR:
				return calc_skill_opr(f,district);
			case Skill_method::NONE:
				return std::nullopt;
			default:
				assert(0);
		}
	}();

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
				if(skills){
					return get_key(skills->pre_dcmp,team.team_key);
				}else{
					return Team_dist{convolve(pr,pr)};
				}
			}
			PRINT(team);
			PRINT(events_left);
			nyi
		}()+team.rookie_bonus;

		{
			auto accounted_pts=sum(mapf([](auto x){ return x.total; },team.event_points))+team.rookie_bonus;
			if(accounted_pts!=team.point_total){
				std::cout<<"Discrepancy:"<<team.team_key<<"\n";
			}
			//assert(accounted_pts==team.point_total);
		}

		points_used[team.team_key]=make_tuple(
			::mapf([](auto x){ return int(x.total); },team.event_points),
			team.rookie_bonus,
			events_left
		);
		Dcmp_home dcmp_home=calc_dcmp_home(f,team.team_key);
		by_team[team.team_key]=Team_status(chairmans.count(team.team_key),std::move(dist),dcmp_home,team.point_total);
	}

	map<Point,Pr> by_points; //# of teams expected to end at each # of points
	for(auto [team,data1]:by_team){
		auto [cm,data,dcmp_home,point_total]=data1;
		(void)team;
		for(auto [pts,pr]:data){
			auto f=by_points.find(pts);
			if(f==by_points.end()){
				by_points[pts]=pr;
			}else{
				f->second+=pr;
			}
		}
	}

	auto dcmp_distribution1=[&](){
		if(skills){
			return skills->at_dcmp;
		}else{
			std::map<Point,Team_dist> r;
			for(auto i:range(500)){
				r[i]=Team_dist(dcmp_distribution(f));
			}
			return r;
		}
	}();

	bool sum_display=0;
	if(sum_display){
		for(auto [pts,pr]:by_points){
			cout<<pts<<","<<pr<<"\n";
		}
	}

	bool cdf_display=0;
	if(cdf_display){
		map<Point,Pr> cdf;
		{
			double d=0;
			for(auto [pts,pr]:by_points){
				d+=pr;
				cdf[pts]=d;
			}
		}

		for(auto [pts,pr]:cdf){
			cout<<pts<<","<<pr<<"\n";
		}
	}

	return make_tuple(
		run_calc(Run_input{
			dcmp_size,
			worlds_slots(district),
			by_team,
			dcmp_played,
			dcmp_distribution1
		}),
		points_used,
		by_team
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
};

map<tba::Team_key,Pr> run(
	TBA_fetcher& f,
/*	TBA_fetcher &f,
	std::string const& output_dir, //output
	tba::District_key district, //data in
	tba::Year year, //data in & out
	std::vector<int> dcmp_size, //how
	string const& title, //output
	string const& district_short, //output
	std::string extra="", //output
	bool ignore_chairmans=0, //how
	Skill_method skill_method=Skill_method::NONE*/
	Run_inputs inputs
){
	//this function exists to separate the input & calculation from the output
	auto district=*inputs.district;
	auto year=*inputs.year;
	auto [results,points_used,by_team]=run_inner(f,inputs.ignore_chairmans,district,year,inputs.dcmp_size,inputs.skill_method);

	//print_lines(by_team);
	bool by_team_csv=0;
	if(by_team_csv){
		cout<<"team,";
		for(auto i:range_st<140>()){
			cout<<i<<",";
		}
		for(auto [team,data1]:by_team){
			auto [cmd,data,dcmp_home,already_earned]=data1;
			cout<<team<<",";
			for(auto i:range_st<140>()){
				auto f=data.find(i);
				if(f==data.end()){
					cout<<"0,";
				}else{
					cout<<f->second<<",";
				}
			}
			cout<<"\n";
		}
	}

	cout<<"Championship cutoff\n";
	for(auto x:results.cmp_cutoff_pr){
		cout<<"\t"<<x<<"\n";
	}

	auto team_info=district_teams(f,district);
	{
		auto g=gen_html(
			results.result,
			team_info,
			results.cutoff_pr,
			to_map(results.cmp_cutoff_pr),
			inputs.title,
			inputs.district_short,
			year,
			inputs.dcmp_size,
			points_used
		);
		ofstream f(inputs.output_dir+"/"+district.get()+inputs.extra+".html");
		f<<g;
	}

	bool show_table=1;
	if(show_table){
		cout<<"Team #\tP(DCMP)\tPts 5%\tPts 50%\tPts 95%\tNickname\n";
		cout.precision(3);
		for(auto a:reversed(sorted(
			results.result,
			[](auto x){ return make_pair(x.dcmp_make,x); }
		))){
			cout<<a.team.str().substr(3,100)<<"\t";
			cout<<a.dcmp_make<<"\t";
			cout<<a.dcmp_interesting[0]<<"\t";
			cout<<a.dcmp_interesting[1]<<"\t";
			cout<<a.dcmp_interesting[2]<<"\t";
			auto f=filter_unique([=](auto const& f){ return f.key==a.team; },team_info);
			using namespace tba;
			cout<<f.nickname<<"\n";
		}
	}

	return to_map(::mapf(
		[](auto x){
			return make_pair(x.team,x.dcmp_make);
		},
		results.result
	));
}

#define TEAM_DATA_ITEMS(X)\
	X(vector<double>,district_points_earned)\
	X(int,unplayed_district_events_scheduled)\
	X(bool,won_district_chairmans)\
	X(optional<int>,dcmp_points)\
	X(Dcmp_home,dcmp_home)

struct Team_data{
	#define X(A,B) A B;
	TEAM_DATA_ITEMS(X)
	#undef X

	auto operator<=>(Team_data const&)const=default;
};

std::ostream& operator<<(std::ostream& o,Team_data const& a){
	o<<"Team_data( ";
	#define X(A,B) o<<""#B<<":"<<a.B<<" ";
	TEAM_DATA_ITEMS(X)
	#undef X
	return o<<")";
}

using District_data=map<tba::Team_key,Team_data>;

Dcmp_home calc_dcmp_home(TBA_fetcher &fetcher,tba::Team_key const& team_key){
	auto t=team(fetcher,team_key);
	if(t.state_prov!="California"){
		return 0;
	}
	auto c=california_region(t);
	return (Dcmp_home)c;
}

Run_input to_run_input_equal(TBA_fetcher &fetcher,tba::District_key district,District_data data){
	Run_input r;
	r.dcmp_size=dcmp_size(district);
	r.worlds_slots=worlds_slots(district);

	//this is the baseline for how many points at an upcoming district event
	const auto pr=flat_map2(historical_event_pts(fetcher));

	r.by_team=to_map(mapf(
		[&](auto data)->std::pair<tba::Team_key,Team_status>{
			auto [team,x]=data;
			Dcmp_home dcmp_home=calc_dcmp_home(fetcher,team);
			return make_pair(team,Team_status(
				x.won_district_chairmans,
				[=]()->Team_dist{
					if(x.unplayed_district_events_scheduled==2){
						return convolve(pr,pr);
					}else if(x.unplayed_district_events_scheduled==1){
						return pr+sum(x.district_points_earned);
					}else if(x.unplayed_district_events_scheduled==0){
						return Team_dist{{sum(x.district_points_earned),1}};
					}else{
						assert(0);
					}
				}(),
				dcmp_home,
				sum(x.district_points_earned)
			));
		},
		data
	));

	r.dcmp_played=[&]()->bool{
		auto e=tba::district_events_simple(fetcher,district);
		auto m=to_set(mapf([](auto e){ return e.event_type; },e));
		return m.count(tba::Event_type::DISTRICT_CMP);
	}();
	//r.dcmp_distribution1=;*/

	auto d=tba::district_rankings(fetcher,district);
	assert(d);

	/*r.d1=mapf(
		[=](auto x){
			//PRINT(x);
			x.event_points=filter([=](auto y){ return events.count(y.event_key); },x.event_points);
			x.point_total=sum(mapf([](auto x){ return x.total; },x.event_points));
			return x;
		},
		*d
	);*/
	//r.points_used=;*/

	//vector<tba::District_ranking>

	#if 0
	r.d1=mapf(
		[](auto x)->tba::District_Ranking{
			auto [team,team_data]=x;
			return tba::District_Ranking{
				team_key,
				rank,
				rookie_bonus pts,
				point total
				vector<Event_points> event points; Event_points=
			};
		},
		data
	);

	cout<<"\n\n";
	print_lines(take(5,*d));
	PRINT(data);
	//points_used:=std::map<tba::Team_key,std::tuple<std::vector<int>,int,int>>
	nyi
	#endif
	return r;
}

Run_input to_run_input_historical(
	TBA_fetcher &fetcher,
	tba::District_key district,
	District_data data
){
	//allow the other version to put in most of the data
	auto r=to_run_input_equal(fetcher,district,data);
	//now, just put in alternate shapes for the distributions.
	nyi
	return r;
}

District_data partial_data(
	TBA_fetcher &fetcher,
	tba::District_key district,
	std::set<tba::Event_key> const& events
){
	auto d=tba::district_rankings(fetcher,district);
	assert(d);
	District_data r;
	for(auto team_data:*d){
		auto team=team_data.team_key;
		auto team_events=filter(
			[&](auto x){ return events.count(x.event_key); },
			team_data.event_points
		);
		auto g=group([](auto x){ return x.district_cmp; },team_events);
		auto district_events=g[0];

		//if third events show up here, will have to put in logic to look at the order the events happen in
		assert(district_events.size()<=2);

		Team_data td;
		td.dcmp_home=calc_dcmp_home(fetcher,team);
		td.district_points_earned=mapf([](auto x){ return x.total; },district_events);

		//obviously not always true; FIXME
		td.unplayed_district_events_scheduled=2-district_events.size();

		auto m=mapf([&](auto event){ return tba::team_event_awards(fetcher,team,event); },events);
		auto c=filter(
			[](auto x)->bool{
				return x.award_type==tba::Award_type::CHAIRMANS;
			},
			flatten(m)
		);
		td.won_district_chairmans=!c.empty();

		auto cmp_events=g[1];
		//this if is here so that can differentiate between 0 points earned and did not play.
		if(!cmp_events.empty()){
			td.dcmp_points=sum(mapf([](auto x){ return x.total; },cmp_events));
		}
		r[team]=td;
	}
	return r;
}

#if 0
Run_input partial_data2(
	TBA_fetcher &fetcher,
	tba::District_key district,
	std::set<tba::Event_key> const& events
){
	(void)events;
	Run_input r;
	r.dcmp_size=dcmp_size(district);
	r.worlds_slots=worlds_slots(district);


	//by_team=
	/*r.by_team=[](){
		//team to (chairmans,distribution by dcmp)
		nyi
	}();*/
	r.dcmp_played=[&]()->bool{
		auto e=district_events_simple(fetcher,district);
		auto m=to_set(mapf([](auto e){ return e.event_type; },e));
		return m.count(tba::Event_type::DISTRICT_CMP);
	}();
	//r.dcmp_distribution1=;*/

	auto d=district_rankings(fetcher,district);
	assert(d);

	r.d1=mapf(
		[=](auto x){
			//PRINT(x);
			x.event_points=filter([=](auto y){ return events.count(y.event_key); },x.event_points);
			x.point_total=sum(mapf([](auto x){ return x.total; },x.event_points));
			return x;
		},
		*d
	);
	//r.points_used=;*/
	nyi
	return r;
}
#endif

vector<District_data> partial_data(TBA_fetcher &fetcher,tba::District_key district){
	(void)fetcher;

	auto events=district_events_simple(fetcher,district);
	auto ends=to_set(mapf([](auto x){ return x.end_date; },events));
	PRINT(ends);

	vector<District_data> r;
	for(auto date:ends){
		auto f=filter([=](auto x){ return x.end_date<=date; },events);
		//PRINT(date);
		auto k=to_set(keys(f));
		//PRINT(k);
		r|=partial_data(fetcher,district,k);
		//print_lines(r);
	}
	return r;
}

int historical_demo(TBA_fetcher &fetcher){
	tba::District_key district{"2024pnw"};
	auto p=partial_data(fetcher,district);
	//Run_input to_run_input_equal(TBA_fetcher &fetcher,tba::District_key district,District_data data){
	mapf(
		[&](auto data){
			//print_lines(count(sorted(values(data))));
			return to_run_input_equal(fetcher,district,data);
		},
		p
	);
	/*Run_input{
		dcmp_size,
		worlds_slots,
		map<Team_key,std::pair<bool,Team_dist> by team
		dcmp_played
		dcmp_distribution1
		vector<tba::District_Ranking> d1
		map<Team_key,tuple<vector<int>,int>> points_used
	};*/
	for(auto p1:p){
		Run_input input;
		input.dcmp_size=dcmp_size(district);
		input.worlds_slots=worlds_slots(district);
		input.by_team=map_values(
			[](auto x)->Team_status{
				PRINT(x);

				Team_status r;
				r.district_chairmans=x.won_district_chairmans;
				nyi//r.point_dist=;
				nyi//r.dcmp_home=;
				nyi//r.already_earned=;

				cout<<"struct Team_status{\n"
				"        bool district_chairmans;\n"
				"        Team_dist point_dist; //number of points expected pre-dcmp\n"
				"        Dcmp_home dcmp_home;\n"
				"        Point already_earned;\n"
				"};\n";

				//DESCRIBE(Team_status);
				nyi
			},
			p1
		);
		nyi//input.dcmp_played=;
		nyi//input.dcmp_distribution1=;
		Run_result result=run_calc(input);
		print_r(result);
		nyi
	}
	return 0;
}

struct Args{
	string output_dir=".";
	tba::Year year{2022};
	optional<tba::District_key> district;
	TBA_fetcher_config tba;
	bool demo=0,historical_demo=0;
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
		"--historical_demo",{},
		"Explore predictability throughout a season",
		r.historical_demo
	);
	p.add(
		"--skill",{"METHOD"},
		"How to measure skill of individual teams",
		r.skill_method
	);
	p.parse(argc,argv);
	return r;
}

using Year=tba::Year;

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

using Date=tba::Date;
using District_key=tba::District_key;

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

			r[k]=interesting_dates;
		}
	}
	return r;
}

int identify_time_demo(TBA_fetcher &f){
	auto x=identify_time(f);
	print_r(x);
	return 0;
}

int main1(int argc,char **argv){
	auto args=parse_args(argc,argv);
	std::filesystem::create_directories(args.output_dir);
	auto tba_fetcher=args.tba.get();

	//return identify_time_demo(tba_fetcher);

	if(args.demo){
		return demo(tba_fetcher);
	}

	if(args.historical_demo){
		return historical_demo(tba_fetcher);
	}

	auto d=districts(tba_fetcher,args.year);

	map<tba::District_key,map<tba::Team_key,Pr>> dcmp_pr;

	for(auto year_info:d){
		auto district=year_info.key;
		if(args.district && district!=args.district){
			continue;
		}
		PRINT(district);
		auto title=year_info.display_name+" District Championship Predictions "+::as_string(args.year);
		Run_inputs run_inputs;
		run_inputs.output_dir=args.output_dir;
		run_inputs.district=district;
		run_inputs.year=args.year;
		run_inputs.dcmp_size=dcmp_size(district);
		run_inputs.title=title;
		run_inputs.district_short=year_info.abbreviation;
		run_inputs.skill_method=args.skill_method;

		//dcmp_pr[district]=run(tba_fetcher,args.output_dir,district,args.year,dcmp_size(district),title,year_info.abbreviation);
		dcmp_pr[district]=run(tba_fetcher,run_inputs);

		if(district=="2022ne"){
			run_inputs.dcmp_size=std::vector<int>{{16}};
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
	}catch(std::invalid_argument const& e){
		cerr<<"Caught:"<<e<<"\n";
		return 1;
	}
}
