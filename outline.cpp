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
//#include "../frc_api/db.h"
#include "util.h"
#include "output.h"

//start generic stuff


//start program-specific stuff.

using namespace std;

int dcmp_size(tba::District_key const& district){
	if(district=="2019chs") return 58;
	if(district=="2019isr") return 45;
	if(district=="2019fma") return 60;
	if(district=="2019fnc") return 32;
	if(district=="2019ont") return 80;
	if(district=="2019tx") return 64;
	if(district=="2019in") return 32;
	if(district=="2019fim") return 160;
	if(district=="2019ne") return 64;
	if(district=="2019pnw") return 64;
	if(district=="2019pch") return 45;

	//via the 2022 game manual v5
	if(district=="2022chs") return 60;
	if(district=="2022isr") return 36;
	if(district=="2022fma") return 60;
	if(district=="2022fnc") return 32;
	if(district=="2022ont") return 80;
	if(district=="2022fit") return 80; //Texas; previously "tx"
	if(district=="2022fin") return 32; //Previously "in"
	if(district=="2022fim") return 160;
	if(district=="2022ne") return 80;
	if(district=="2022pnw") return 50;
	if(district=="2022pch") return 32;

	cerr<<"Unknown event size for "<<district<<"\n";
	exit(1);
}

int worlds_slots(tba::District_key key){
	//Via https://www.firstinspires.org/resource-library/frc/championship-eligibility-criteria
	//Subtract out chairmans, EI, and Rookie All-Star winners
	//Note that Rookie All-Star is not always awarded. (which is the last number)
	if(key=="2022chs") return 16-2-2-1;
	if(key=="2022fim") return 64-4-1-1;
	if(key=="2022fma") return 18-2-2-1;
	if(key=="2022fin") return 8-1-2-1;
	if(key=="2022ne") return 25-3-2-1;
	if(key=="2022ont") return 11-1-1-1;
	if(key=="2022fit") return 23-3-2-2;
	if(key=="2022isr") return 9-1-1-1;
	if(key=="2022fnc") return 10-1-2-1;
	if(key=="2022pnw") return 18-2-1-1;
	if(key=="2022pch") return 10-1-2-1;
	cerr<<"Error: Unknown number of worlds slots for district:"<<key<<"\n";
	exit(1);
}

int cmp_slots(tba::District_key district){
	map<string,int> slots{
		{"2019chs",21},
		{"2019fim",87},
		{"2019isr",11},
		{"2019fma",21},
		{"2019in",10},
		{"2019ne",33},
		{"2019ont",29},
		{"2019tx",38},
		{"2019fnc",15},
		{"2019pnw",31},
		{"2019pch",17},
	};
	auto f=slots.find(district.get());
	assert(f!=slots.end());
	return f->second;
}

multiset<Point> point_results(tba::Cached_fetcher& server,tba::District_key dk){
	auto d=district_rankings(server,dk);
	assert(d);
	multiset<Point> r;
	for(auto team_result:*d){
		for(auto event:team_result.event_points){
			r|=Point(event.total);
		}
	}
	return r;
}

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

set<tba::Team_key> chairmans_winners(tba::Cached_fetcher& f,tba::District_key district){
	set<tba::Team_key> r;
	for(auto event:district_events(f,district)){
		auto k=event.key;
		auto aw=event_awards(f,k);
		if(aw.empty()) continue;
		auto f1=filter([](auto a){ return a.award_type==tba::Award_type::CHAIRMANS; },aw);
		if(f1.empty()){
			continue;
		}
		if(f1.size()!=1){
			PRINT(district);
			PRINT(k);
			PRINT(aw);
			PRINT(f1);
			nyi
			
		}

		//There is more than one recipient at the dcmp events.
		for(auto x:f1[0].recipient_list){
			auto team=x.team_key;
			assert(team);
			r|=*team;
		}
	}
	return r;
}

using namespace tba;

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

map<Point,Pr> dcmp_distribution(tba::Cached_fetcher &f){
	vector<tba::District_key> old_districts{
		tba::District_key{"2019pnw"},
		tba::District_key{"2018pnw"},
		tba::District_key{"2017pnw"},
		tba::District_key{"2016pnw"},
		tba::District_key{"2015pnw"},
		//tba::District_key{"2014pnw"},
	};
	multiset<Point> v;
	for(auto district:old_districts){
		auto a=district_rankings(f,district);
		if(!a) continue;
		for(auto team_data:*a){
			for(auto event_points:team_data.event_points){
				if(event_points.district_cmp){
					v|=int(event_points.total);
				}
			}
		}
	}
	map<Point,Pr> r;
	for(auto x:v){
		r[x]=v.count(x)/double(v.size());
	}
	return r;
}

map<Team_key,Pr> run(
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
	vector<tba::District_key> old_keys{
		//excluding 2014 since point system for quals was different.
		tba::District_key{"2015pnw"},
		tba::District_key{"2016pnw"},
		tba::District_key{"2017pnw"},
		tba::District_key{"2018pnw"},
		tba::District_key{"2019pnw"}
	};

	auto team_info=district_teams(f,district);
	bool dcmp_played=0;

	//print_lines(district_rankings(f,district));
	auto d=district_rankings(f,district);
	assert(d);
	auto d1=*d;
	//print_lines(d1);
	multiset<Point> old_results;
	for(auto key:old_keys){
		old_results|=point_results(f,district);
	}
	map<Point,unsigned> occurrances;
	for(auto value:old_results){
		occurrances[value]=old_results.count(value); //slow
	}
	map<Point,Pr> pr;
	for(auto [pts,count]:occurrances){
		pr[pts]=double(count)/old_results.size();
	}
	//print_lines(pr);
	//PRINT(sum(seconds(pr)));
	//PRINT(old_results.size())

	auto chairmans=chairmans_winners(f,district);
	if(ignore_chairmans){
		chairmans.clear();
	}

	map<tba::Team_key,pair<bool,map<Point,Pr>>> by_team;
	map<tba::Team_key,tuple<vector<int>,int,int>> points_used;
	for(auto team:d1){
		//auto events_left=2-team.event_points.size();
		auto max_counters=2-int(team.event_points.size());
		auto events_scheduled=team_events_year_keys(f,team.team_key,year);
		auto events_left=min(max_counters,int(events_scheduled.size())-int(team.event_points.size()));
		//assert(events_left>=0);
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

	map<Point,Pr> cdf;
	{
		double d=0;
		for(auto [pts,pr]:by_points){
			d+=pr;
			cdf[pts]=d;
		}
	}

	bool cdf_display=0;
	if(cdf_display){
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
	multiset<pair<Point,Pr>> cutoffs,cmp_cutoff;
	const auto iterations=2000; //usually want this to be like 2k
	for(auto iteration:range(iterations)){
		(void)iteration;
		//PRINT(iteration);
		map<pair<bool,Point>,unsigned> final_points;
		for(auto [team,data]:by_team){
			auto [cm,dist]=data;
			final_points[pair<bool,Point>(cm,sample(dist))]++;
		}

		//PRINT(find_cutoff);

		auto dcmp_cutoff=find_cutoff(final_points,teams_left_out);
		cutoffs|=dcmp_cutoff;

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

	//print_lines(count(cutoffs));
	map<pair<Point,Pr>,Pr> cutoff_pr=map_values(
		[=](auto x){ return (0.0+x)/iterations; },
		count(cutoffs)
	);
	//print_lines(cutoff_pr);

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
		//PRINT(team);
		//PRINT(team.team_key);
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
		//PRINT(pr_make+pr_miss);
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

#if 0
void dcmp_awards(District_key district){
	map<District_key,int> chairmans{
		{"2019pnw",3/*chairmans*/+2/*ei*/+1/*ras*/},
	};
	//able to win DCMP EI without competing there with robot?
	//looks like 568 did last year in PNW
}
#endif

auto get_tba_fetcher(std::string const& auth_key_path,std::string const& cache_path){
	ifstream ifs(auth_key_path);
	string tba_key;
	getline(ifs,tba_key);
	return tba::Cached_fetcher{tba::Fetcher{tba::Nonempty_string{tba_key}},tba::Cache{cache_path.c_str()}};

}

/*auto get_frc_fetcher(){
	ifstream f("../frc_api/api_key");
	string s;
	getline(f,s);
	return frc_api::Cached_fetcher{frc_api::Fetcher{frc_api::Nonempty_string{s}},frc_api::Cache{}};
}*/

struct Args{
	string output_dir=".";
	string tba_auth_key="../tba/auth_key";
	string tba_cache="cache.db";
	tba::Year year{2022};
	optional<tba::District_key> district;
};

Args parse_args(int argc,char **argv){
	struct Flag{
		string name;
		vector<string> args;
		string help;
		std::function<void(std::span<char*>)> func;
	};
	Args r;
	vector<Flag> flags{
		Flag{
			"--out",{"PATH"},
			"Directory in which to store result files",
			[&](span<char*> v){
				r.output_dir=v[0];
			}
		},
		Flag{
			"--auth_key",
			{"PATH"},
			"Path to The Blue Alliance auth key",
			[&](span<char *> v){
				r.tba_auth_key=v[0];
			}
		},
		Flag{
			"--cache",
			{"PATH"},
			"Path to use for cached data from The Blue Alliance",
			[&](span<char *> v){
				r.tba_cache=v[0];
			}
		},
		Flag{
			"--year",
			{"YEAR"},
			"For which year the predictions should be made",
			[&](span<char*> v){
				r.year=tba::Year{stoi(v[0])};
			}
		},
		Flag{
			"--district",
			{"KEY"},
			"Examine only a specific district",
			[&](span<char*> v){
				r.district=tba::District_key{v[0]};
			}
		}
	};

	auto help=[&](){
		cout<<argv[0];
		for(auto flag:flags){
			cout<<" ["<<flag.name;
			for(auto x:flag.args){
				cout<<" "<<x;
			}
			cout<<"]";
		}
		cout<<"\n";
		cout<<"Calculates odds of FRC teams advancing to their district championships and the championship event.\n";
		for(auto flag:flags){
			cout<<flag.name;
			for(auto a:flag.args){
				cout<<" "<<a;
			}
			cout<<"\n";
			cout<<"\t"<<flag.help<<"\n";
		}
	};

	flags|=Flag{
		"--help",
		{},
		"Show this message",
		[=](auto){
			help();
			exit(0);
		}
	};

	for(int i=1;i<argc;){
		auto f=filter([=](auto x){ return x.name==argv[i]; },flags);
		if(f.empty()){
			cerr<<"Error: Unrecognized argument: "<<argv[i]<<"\n";
			exit(1);
		}
		auto flag=f[0];
		i++;
		auto left=argc-i;
		if(left<int(flag.args.size())){
			cerr<<"Missing arg to "<<flag.name<<"\n";
			help();
			exit(1);
		}
		flag.func(span{argv+i,flag.args.size()});
		i+=flag.args.size();
	}

	return r;
}

int main1(int argc,char **argv){
	//auto frc_fetcher=get_frc_fetcher();

	auto args=parse_args(argc,argv);
	std::filesystem::create_directories(args.output_dir);
	auto tba_fetcher=get_tba_fetcher(args.tba_auth_key,args.tba_cache);

	auto d=districts(tba_fetcher,args.year);
	map<District_key,map<Team_key,Pr>> dcmp_pr;

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
	}
}
