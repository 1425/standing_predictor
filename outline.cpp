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
#include<iomanip>
#include "../tba/db.h"
#include "util.h"
#include "../tba/data.h"
#include "../tba/tba.h"
#include "../tba/util.h"

//start generic stuff

#define PRINT TBA_PRINT
#define nyi TBA_NYI

//start program-specific stuff.

using namespace std;
using namespace tba;

using Pr=double; //probability
using Point=int;

multiset<Point> point_results(Cached_fetcher& server,District_key dk){
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

map<Point,Pr> convolve(map<Point,Pr> a,map<Point,Pr> b){
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

set<Team_key> chairmans_winners(Cached_fetcher& f,District_key district){
	set<Team_key> r;
	for(auto event:district_events(f,district)){
		auto k=event.key;
		auto aw=event_awards(f,k);
		if(aw.empty()) continue;
		auto f1=filter([](auto a){ return a.award_type==Award_type::CHAIRMANS; },aw);
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
		auto f=f1[0];
		assert(f.recipient_list.size()==1);
		auto team=f.recipient_list[0].team_key;
		assert(team);
		r|=*team;
	}
	return r;
}

string make_link(Team_key team){
	auto s=team.str();
	assert(s.substr(0,3)=="frc");
	auto t=s.substr(3,500);
	return link("https://www.thebluealliance.com/team/"+t,t);
}

auto digit=[](auto i)->char{
	if(i<10) return '0'+i;
	return 'a'+(i-10);
};

string color(double d){
	//input range 0-1
	//red->white->green

	auto f=[](double v){
		auto x=min(255,int(255*v));
		return string()+digit(x>>4)+digit(x&0xf);
	};

	auto rgb=[=](double r,double g,double b){
		return "#"+f(r)+f(g)+f(b);
	};

	if(d<.5){
		return rgb(1,d*2,d*2);
	}
	auto a=2*(1-d);
	return rgb(a,1,a);
}

string colorize(double d){
	return tag("td bgcolor=\""+color(d)+"\"",
		[&](){
			stringstream ss;
			ss<<setprecision(3)<<fixed;
			ss<<d;
			return ss.str();
		}()
	);
}

double entropy(Pr p){
	//units are bits.
	if(p<0) p=0;
	if(p>1) p=1;
	if(p==0 || p==1) return 0;
	assert(p>0 && p<1);
	return -(log(p)*p+log(1-p)*(1-p))/log(2);
}

using Extended_cutoff=pair<Point,Pr>;

map<Point,Pr> simplify(map<pair<Point,Pr>,Pr> m){
	map<Point,Pr> r;
	for(auto [k,v]:m){
		r[k.first]+=v;
	}
	return r;
}

string gen_html(
	vector<tuple<Team_key,Pr,Point,Point,Point>> result,
	vector<Team> team_info,
	map<Extended_cutoff,Pr> cutoff_pr,
	string title,
	string district_short,
	Year year,
	int dcmp_size
){
	auto nickname=[&](auto k){
		auto f=filter_unique([=](auto a){ return a.key==k; },team_info);
		auto v=f.nickname;
		assert(v);
		return *v;
	};

	auto cutoff_table=[=](){
		return h2("Cutoff value")+tag("table border",
			tr(th("Points")+th("Probability"))+
			join(mapf(
				[](auto a){
					return tr(join(MAP(td,a)));
				},
				simplify(cutoff_pr)
			))
		);
	}();

	auto cutoff_table_long=[=](){
		return h2("Cutoff value - extended")+
		"The cutoff values, along with how likely a team at that value is to miss advancing.  For example: a line that said (50,.25) would correspond to the probability that team above 50 get in, teams below 50 do not, and 75% of teams ending up with exactly 50 would qualify for the district championship."+
		tag("table border",
			tr(th("Points")+th("Probability"))+
			join(mapf(
				[](auto a){
					return tr(join(MAP(td,a)));
				},
				cutoff_pr
			))
		);
	}();

	double total_entropy=sum(::mapf(entropy,seconds(result)));
	PRINT(total_entropy);
	
	return tag("html",
		tag("head",
			tag("title",title)
		)+
		tag("body",
			tag("h1",title)+
			link("https://frc-events.firstinspires.org/"+as_string(year)+"/district/"+district_short,"FRC Events")+"<br>"+
			link("https://www.thebluealliance.com/events/"+district_short+"/"+as_string(year)+"#rankings","The Blue Alliance")+"<br>"+
			link("http://frclocks.com/index.php?d="+district_short,"FRC Locks")+"(slow)<br>"+
			"Slots at event:"+as_string(dcmp_size)+
			cutoff_table+
			h2("Team Probabilities")+
			tag("table border",
				tr(join(::mapf(
					th1,
					std::vector<string>{
						"Probability rank",
						"Probability of making district championship",
						"Team number",
						"Nickname",
						"Extra points needed to have 5% chance of making district championship",
						"Extra points needed to have 50% chance of making district championship",
						"Extra points needed to have 95% chance of making district championship"
					}
				)))+
				join(
					::mapf(
						[=](auto p){
							auto [i,a]=p;
							return tr(join(
								vector<string>{}+td(i)+
								colorize(get<1>(a))+
							::mapf(
								td1,
								std::vector<std::string>{
									make_link(get<0>(a)),
									nickname(get<0>(a)),
									as_string(get<2>(a)),
									as_string(get<3>(a)),
									as_string(get<4>(a))
								}
							)));
						},
						enumerate_from(1,reversed(sorted(
							result,
							[](auto x){ return make_pair(get<1>(x),x); }
						)))
					)
				)
			)+
			cutoff_table_long
		)
	);
}

void run(Cached_fetcher &f,District_key district,Year year,int dcmp_size,string title,string district_short,std::string extra="",bool ignore_chairmans=0){
	vector<District_key> old_keys{
		//excluding 2014 since point system for quals was different.
		District_key{"2015pnw"},
		District_key{"2016pnw"},
		District_key{"2017pnw"},
		District_key{"2018pnw"},
		District_key{"2019pnw"}
	};

	auto team_info=district_teams(f,district);

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

	map<Team_key,map<Point,Pr>> by_team;
	for(auto team:d1){
		if(chairmans.count(team.team_key)){
			continue;
		}
		//auto events_left=2-team.event_points.size();
		auto max_counters=2-int(team.event_points.size());
		auto events_scheduled=team_events_year_keys(f,team.team_key,year);
		auto events_left=min(max_counters,int(events_scheduled.size())-int(team.event_points.size()));
		assert(events_left>=0);
		by_team[team.team_key]=[&]()->map<Point,Pr>{
			auto first_event_points=[=]()->double{
				if(team.event_points.size()){
					return team.event_points[0].total;
				}
				return 0;
			}();
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
			nyi
		}()+team.rookie_bonus;
	}

	print_lines(by_team);
	bool by_team_csv=0;
	if(by_team_csv){
		cout<<"team,";
		for(auto i:range(140)){
			cout<<i<<",";
		}
		for(auto [team,data]:by_team){
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
	for(auto [team,data]:by_team){
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

	auto teams_advancing=dcmp_size-chairmans.size();
	auto teams_competing=sum(values(by_points));
	auto teams_left_out=teams_competing-teams_advancing;

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

	multiset<pair<Point,Pr>> cutoffs;
	const auto iterations=2000; //usually want this to be like 2k
	for(auto iteration:range(iterations)){
		(void)iteration;
		//PRINT(iteration);
		map<Point,unsigned> final_points;
		for(auto [team,dist]:by_team){
			final_points[sample(dist)]++;
		}
		auto find_cutoff=[&](){
			unsigned total=0;
			for(auto [points,teams]:final_points){
				total+=teams;
				/*if(total>=teams_left_out){
					return points;
				}*/
				if(total>=teams_left_out){
					auto excess=total-teams_left_out;
					return make_pair(points,1-excess/teams);
				}
			}
			assert(0);
		}();
		//PRINT(find_cutoff);
		cutoffs|=find_cutoff;
	}

	//print_lines(count(cutoffs));
	map<pair<Point,Pr>,Pr> cutoff_pr=map_values(
		[=](auto x){ return (0.0+x)/iterations; },
		count(cutoffs)
	);
	print_lines(cutoff_pr);

	auto cutoff_level=[=](Pr probability_target){
		double t=0;
		for(auto [points,pr]:cutoff_pr){
			t+=pr;
			if(t>probability_target){
				return points;
			}
		}
		assert(0);
	};
	map<Pr,Extended_cutoff> interesting_cutoffs;
	for(auto d:{.05,.5,.95}){
		interesting_cutoffs[d]=cutoff_level(d);
	}

	vector<tuple<Team_key,Pr,Point,Point,Point>> result;
	for(auto team:d1){
		//PRINT(team);
		//PRINT(team.team_key);
		//probability that get in
		//subtract the cutoff pr
		auto team_pr=by_team[team.team_key];
		double pr_make=0;
		double pr_miss=0;
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
		//PRINT(pr_make+pr_miss);
		auto total=pr_make+pr_miss;
		if(chairmans.count(team.team_key)){
			assert(total>=-.01 && total<=.01);
			result|=make_tuple(team.team_key,1.0,0,0,0);
		}else{
			//PRINT(total);
			assert(total>.99 && total<1.01);

			//PRINT(pr_make);
			//points needed to have 50% odds; 5% odds; 95% odds, or quartiles?
			auto points_so_far=team.point_total;
			vector<Point> interesting;
			for(auto [pr,pts]:interesting_cutoffs){
				//cout<<pr<<":"<<max(0.0,pts-points_so_far)<<"\n";
				auto value=max(0,int(pts.first-points_so_far));
				interesting|=value;
			}
			assert(interesting.size()==3);
			result|=make_tuple(team.team_key,pr_make,interesting[0],interesting[1],interesting[2]);
		}
	}

	auto x=::mapf([](auto x){ return get<1>(x); },result);
	PRINT(sum(x)); //this number should be really close to the number of slots available at the event.

	{
		auto g=gen_html(result,team_info,cutoff_pr,title,district_short,year,dcmp_size);
		ofstream f(district.get()+extra+".html");
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
			cout<<f.nickname<<"\n";
		}
	}
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

int cmp_slots(District_key district){
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

void worlds(Cached_fetcher &f){
	District_key district{"2019pnw"};
	Event_key event{"2019pncmp"};
	auto rankings=district_rankings(f,district);
	//PRINT(*rankings);
	auto teams=event_teams_keys(f,event);
	map<Team_key,int> existing_points;
	for(auto team:teams){
		auto f=filter_unique(
			[=](auto x){ return x.team_key==team; },
			*rankings
		);
		existing_points[team]=f.point_total;
	}

	//PRINT(existing_points);

	vector<int> cutoffs;
	for(auto _:range(1000)){
		(void)_;
		auto old_event=Event_key{"2018pncmp"};
		auto d=district_rankings(f,District_key{"2018pnw"});
		vector<int> dcmp_pts;
		for(auto team_result:*d){
			for(auto event:team_result.event_points){
				if(event.event_key==old_event){
					dcmp_pts|=int(event.total);
				}
			}
		}
		//PRINT(dcmp_pts);

		vector<int> team_total;
		for(auto [t,p]:existing_points){
			team_total|=(p+choose(dcmp_pts));
		}

		//print_lines(enumerate_from(1,reversed(sorted(team_total))));
		cutoffs|=reversed(sorted(team_total))[27];
	}
	//print_lines(sorted(cutoffs));
	PRINT(max(cutoffs));
	PRINT(min(cutoffs));
	PRINT(mean(cutoffs));
	PRINT(median(cutoffs));
	exit(0);
}

int main1(int argc,char **argv){
	(void)argc;
	(void)argv;

	ifstream ifs("../tba/auth_key");
	string tba_key;
	getline(ifs,tba_key);
	Cached_fetcher f{Fetcher{Nonempty_string{tba_key}},Cache{}};

	/*try{
		worlds(f);
	}catch(string s){
		cout<<s;
		return 1;
	}*/

	Year year{2022};
	auto d=districts(f,year);
	PRINT(d);
	for(auto year_info:d){
		//District_key district{"2019pnw"};
		auto district=year_info.key;
		PRINT(district);
		auto dcmp_size=[=](){
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
			nyi
		}();
		auto title=year_info.display_name+" District Championship Predictions "+as_string(year);
		run(f,district,year,dcmp_size,title,year_info.abbreviation);

		if(district=="2022ne"){
			run(f,district,year,16,"New England Championship Pre-Qualify",year_info.abbreviation,"_cmp",1);
		}
	}
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
