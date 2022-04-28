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
*/

#include<fstream>
#include<sstream>
#include<set>
#include<random>
#include<span>
#include<filesystem>
#include "../tba/db.h"
#include "../tba/data.h"
#include "../tba/tba.h"
#include "output.h"
#include "event.h"
#include "arguments.h"
#include "tba.h"
#include "set.h"
#include "util.h"

//start generic stuff

//start program-specific stuff.

using namespace std;

map<Point,Pr> convolve(map<Point,Pr> const& a,map<Point,Pr> const& b){
	map<Point,Pr> r;
	for(auto [a1,ap]:a){
		for(auto [b1,bp]:b){
			auto result=a1+b1;
			auto pr=ap*bp;
			auto f=r.find(result);
			if(f==r.end()){
				r[result]=pr;
			}else{
				f->second+=pr;
			}
		}
	}
	return r;
}

map<Point,Pr> operator+(map<Point,Pr> a,int i){
	map<Point,Pr> r;
	for(auto [k,v]:a){
		r[k+i]=v;
	}
	return r;
}

map<Point,Pr> when_greater(map<Point,Pr> const& a,map<pair<Point,double>,Pr> const& b){
	map<Point,Pr> r;
	for(auto [ka,va]:a){
		for(auto [kb,vb]:b){
			auto [value,pr]=kb;
			if(ka>value){
				r[ka]+=va*vb;
			}else if(ka==value){
				r[ka]+=va*vb*pr;
			}
		}
	}
	return r;
}

map<Point,Pr> when_greater(map<Point,Pr> const& a,map<Point,Pr> const& b){
	//what the probability distribution of "a" looks like when it is higher than "b".
	//not that the output is not expected to sum to 1 unless a is always greater than b.
	//O(a.size()*b.size()) but this should still be better than sampling them since N and M are not that large.
	map<Point,Pr> r;
	for(auto [ka,va]:a){
		for(auto [kb,vb]:b){
			if(ka>kb){
				r[ka]+=va*vb;
			}
		}
	}
	return r;
}

auto find_cutoff(map<pair<bool,Point>,unsigned> these_points,unsigned eliminating){
	unsigned total=0;
	for(auto [points,teams]:these_points){
		total+=teams;
		if(total>=eliminating){
			auto excess=total-eliminating;
			assert(points.first==0);
			return make_pair(points.second,1-double(excess)/teams);
		}
	}
	assert(0);
}

map<tba::Team_key,Pr> run(
	tba::Cached_fetcher &f,
	std::string const& output_dir,
	tba::District_key district,
	tba::Year year,
	int dcmp_size,
	string const& title,
	string const& district_short,
	std::string extra="",
	bool ignore_chairmans=0
){
	bool dcmp_played=0;

	const auto pr=historical_event_pts(f);
	const auto chairmans=[&](){
		if(ignore_chairmans){
			return set<tba::Team_key>{};//chairmans.clear();
		}
		return chairmans_winners(f,district);
	}();

	const auto d1=[&](){
		auto d=district_rankings(f,district);
		assert(d);
		return *d;
	}();

	map<tba::Team_key,pair<bool,map<Point,Pr>>> by_team;
	map<tba::Team_key,tuple<vector<int>,int,int>> points_used;
	for(auto team:d1){
		auto max_counters=2-int(team.event_points.size());
		auto events_scheduled=team_events_year_keys(f,team.team_key,year);
		auto events_left=min(max_counters,int(events_scheduled.size())-int(team.event_points.size()));
		auto dist=[&]()->map<Point,Pr>{
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
				return map<Point,Pr>{{
					sum(::mapf([](auto x){ return x.total; },team.event_points)),
					1
				}};
			}
			if(events_left==0){
				return map<Point,Pr>{{
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
				return to_map(mapf(
					[&](auto p){
						return make_pair(int(p.first+first_event_points),p.second);
					},
					pr
				));
			}
			if(events_left==2){
				return convolve(pr,pr);
			}
			PRINT(team);
			PRINT(events_left);
			nyi
		}()+team.rookie_bonus;
		points_used[team.team_key]=make_tuple(
			::mapf([](auto x){ return int(x.total); },team.event_points),
			team.rookie_bonus,
			events_left
		);
		by_team[team.team_key]=make_pair(chairmans.count(team.team_key),dist);
	}

	//print_lines(by_team);
	bool by_team_csv=0;
	if(by_team_csv){
		cout<<"team,";
		for(auto i:range(140)){
			cout<<i<<",";
		}
		for(auto [team,data1]:by_team){
			auto [cmd,data]=data1;
			cout<<team<<",";
			for(auto i:range(140)){
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

	map<Point,Pr> by_points; //# of teams expected to end at each # of points
	for(auto [team,data1]:by_team){
		auto [cm,data]=data1;
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

	auto teams_advancing=dcmp_size;
	auto teams_competing=sum(values(by_points));
	unsigned teams_left_out=max(0.0,teams_competing-teams_advancing); //Ontario has more slots than team in 2022.
	unsigned cmp_teams_left_out=max(0,dcmp_size-worlds_slots(district));

	//monte carlo method for where the cutoff is

	std::mt19937_64 rng;
	// initialize the random number generator with time-dependent seed
	uint64_t timeSeed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	std::seed_seq ss{uint32_t(timeSeed & 0xffffffff), uint32_t(timeSeed>>32)};
	rng.seed(ss);
	// initialize a uniform distribution between 0 and 1
	std::uniform_real_distribution<double> unif(0, 1);

	auto sample=[&](map<Point,Pr> const& m)->Point{
		auto num=unif(rng);
		double total=0;
		for(auto [value,pr]:m){
			total+=pr;
			if(total>num){
				return value;
			}
		}
		assert(0);
	};

	auto dcmp_distribution1=dcmp_played?map<Point,Pr>{{0,1}}:dcmp_distribution(f);
	multiset<pair<Point,Pr>> dcmp_cutoffs,cmp_cutoff;
	const auto iterations=2000; //usually want this to be like 2k
	for(auto iteration:range(iterations)){
		(void)iteration;
		//PRINT(iteration);
		map<pair<bool,Point>,unsigned> final_points;
		for(auto [team,data]:by_team){
			auto [cm,dist]=data;
			final_points[pair<bool,Point>(cm,sample(dist))]++;
		}

		auto dcmp_cutoff=find_cutoff(final_points,teams_left_out);
		dcmp_cutoffs|=dcmp_cutoff;

		map<pair<bool,Point>,unsigned> post_dcmp_points;
		for(auto [earned,teams]:final_points){
			auto [cm,points]=earned;

			if(!cm && points<dcmp_cutoff.first) continue;
			if(points==dcmp_cutoff.first){
				teams*=(1-dcmp_cutoff.second);
			}

			for(auto _:range(teams)){
				(void)_;
				int pts;
				if(dcmp_played){
					pts=points;
				}else{
					pts=points+sample(dcmp_distribution1);
				}
				post_dcmp_points[make_pair(0,pts)]++;
			}
		}

		cmp_cutoff|=find_cutoff(post_dcmp_points,cmp_teams_left_out);
	}

	map<pair<Point,Pr>,Pr> cutoff_pr=map_values(
		[=](auto x){ return (0.0+x)/iterations; },
		count(dcmp_cutoffs)
	);

	map<pair<Point,Pr>,Pr> cmp_cutoff_pr=map_values(
		[=](auto x){ return (0.0+x)/iterations; },
		count(cmp_cutoff)
	);
	cout<<"Championship cutoff\n";
	for(auto x:cmp_cutoff_pr){
		cout<<"\t"<<x<<"\n";
	}

	auto cutoff_level=[=](map<pair<Point,Pr>,Pr> const& cutoff_set,Pr probability_target){
		double t=0;
		for(auto [points,pr]:cutoff_set){
			t+=pr;
			if(t>probability_target){
				return points;
			}
		}
		assert(0);
	};

	auto interesting_cutoffs=[=](auto cutoff_set){
		map<Pr,Extended_cutoff> r;
		for(auto d:{.05,.5,.95}){
			r[d]=cutoff_level(cutoff_set,d);
		}
		return r;
	};
	auto interesting_cutoffs_dcmp=interesting_cutoffs(cutoff_pr);
	auto interesting_cutoffs_cmp=interesting_cutoffs(cmp_cutoff_pr);

	vector<tuple<tba::Team_key,Pr,Point,Point,Point,Pr,Point,Point,Point>> result;
	for(auto team:d1){
		//probability that get in
		//subtract the cutoff pr
		auto [cm,team_pr]=by_team[team.team_key];
		double pr_make=0;
		double pr_miss=0;
		if(cm){
			pr_make=1;
			pr_miss=0;
		}else{
			for(auto [cutoff,c_pr]:cutoff_pr){
				for(auto [team_value,t_pr]:team_pr){
					auto combined_pr=c_pr*t_pr;
					if(team_value>cutoff.first){
						pr_make+=combined_pr;
					}else if(team_value==cutoff.first){
						pr_make+=combined_pr*(1-cutoff.second);
						pr_miss+=combined_pr*cutoff.second;
					}else{
						pr_miss+=combined_pr;
					}
				}
			}
		}
		auto total=pr_make+pr_miss;
		assert(total>.99 && total<1.01);

		auto dcmp_entry_dist=[=](){
			if(cm){
				return team_pr;
			}
			return when_greater(team_pr,cutoff_pr);
		}();
		auto post_dcmp_dist=convolve(dcmp_entry_dist,dcmp_distribution1);
		auto post_total=sum(values(post_dcmp_dist));
		Pr cmp_make=0;
		Pr cmp_miss=0;
		for(auto [cutoff,c_pr]:cmp_cutoff_pr){
			for(auto [team_value,t_pr]:post_dcmp_dist){
				auto combined_pr=c_pr*t_pr;
				if(team_value>cutoff.first){
					cmp_make+=combined_pr;
				}else if(team_value==cutoff.first){
					cmp_make+=combined_pr*(1-cutoff.second);
					cmp_miss+=combined_pr*cutoff.second;
				}else{
					cmp_miss+=combined_pr;
				}
			}
		}
		{
			auto residual=post_total-cmp_make-cmp_miss;
			assert(residual<.01 && residual>-.01);
		}

		auto points_so_far=team.point_total;
		vector<Point> cmp_interesting;
		for(auto [pr,pts]:interesting_cutoffs_cmp){
			cmp_interesting|=max(0,int(pts.first-points_so_far));
		}

		if(cm){
			assert(pr_make>.99);
			result|=make_tuple(
				team.team_key,
				1.0,0,0,0,
				cmp_make,cmp_interesting[0],cmp_interesting[1],cmp_interesting[2]
			);
		}else{
			//PRINT(total);

			//PRINT(pr_make);
			//points needed to have 50% odds; 5% odds; 95% odds, or quartiles?
			vector<Point> interesting;
			for(auto [pr,pts]:interesting_cutoffs_dcmp){
				//cout<<pr<<":"<<max(0.0,pts-points_so_far)<<"\n";
				auto value=max(0,int(pts.first-points_so_far));
				interesting|=value;
			}
			assert(interesting.size()==3);

			result|=make_tuple(
				team.team_key,
				pr_make,interesting[0],interesting[1],interesting[2],
				cmp_make,cmp_interesting[0],cmp_interesting[1],cmp_interesting[2]
			);
		}
	}

	auto x=::mapf([](auto x){ return get<1>(x); },result);
	//PRINT(sum(x)); //this number should be really close to the number of slots available at the event.

	auto team_info=district_teams(f,district);
	{
		auto g=gen_html(result,team_info,cutoff_pr,cmp_cutoff_pr,title,district_short,year,dcmp_size,points_used);
		ofstream f(output_dir+"/"+district.get()+extra+".html");
		f<<g;
	}

	bool show_table=1;
	if(show_table){
		cout<<"Team #\tP(DCMP)\tPts 5%\tPts 50%\tPts 95%\tNickname\n";
		cout.precision(3);
		for(auto a:reversed(sorted(
			result,
			[](auto x){ return make_pair(get<1>(x),x); }
		))){
			cout<<get<0>(a).str().substr(3,100)<<"\t";
			cout<<get<1>(a)<<"\t";
			cout<<get<2>(a)<<"\t";
			cout<<get<3>(a)<<"\t";
			cout<<get<4>(a)<<"\t";
			auto f=filter_unique([=](auto f){ return f.key==get<0>(a); },team_info);
			using namespace tba;
			cout<<f.nickname<<"\n";
		}
	}

	return to_map(::mapf(
		[](auto x){
			return make_pair(get<0>(x),get<1>(x));
		},
		result
	));
}

struct Args{
	string output_dir=".";
	string tba_auth_key="../tba/auth_key";
	string tba_cache="cache.db";
	tba::Year year{2022};
	optional<tba::District_key> district;
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
		"--auth_key",{"PATH"},
		"Path to The Blue Alliance auth key",
		r.tba_auth_key
	);
	p.add(
		"--cache",{"PATH"},
		"Path to use for cached data from The Blue Alliance",
		r.tba_cache
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
	
	p.parse(argc,argv);
	return r;
}

int main1(int argc,char **argv){
	//auto frc_fetcher=get_frc_fetcher();

	auto args=parse_args(argc,argv);
	std::filesystem::create_directories(args.output_dir);
	auto tba_fetcher=get_tba_fetcher(args.tba_auth_key,args.tba_cache);

	auto d=districts(tba_fetcher,args.year);
	map<tba::District_key,map<tba::Team_key,Pr>> dcmp_pr;

	for(auto year_info:d){
		auto district=year_info.key;
		if(args.district && district!=args.district){
			continue;
		}
		PRINT(district);
		auto title=year_info.display_name+" District Championship Predictions "+as_string(args.year);
		dcmp_pr[district]=run(tba_fetcher,args.output_dir,district,args.year,dcmp_size(district),title,year_info.abbreviation);

		if(district=="2022ne"){
			run(tba_fetcher,args.output_dir,district,args.year,16,"New England Championship Pre-Qualify",year_info.abbreviation,"_cmp",1);
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
