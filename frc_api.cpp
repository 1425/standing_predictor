#include "frc_api.h"
#include "../frc_api/curl.h"
#include "../frc_api/data.h"
#include "../frc_api/rapidjson.h"
#include "rank_pts.h"
#include "set.h"
#include "util.h"
#include "print_r.h"

//start generic code

template<typename T>
bool all_equal(std::vector<T> const& a){
	for(auto elem:a){
		if(elem!=a[0]){
			return 0;
		}
	}
	return 1;
}

template<typename Func,typename T>
std::vector<T> sort_by(Func f,std::vector<T> a){
	sort(
		a.begin(),
		a.end(),
		[=](auto a,auto b){
			return f(a)<f(b);
		}
	);
	return a;
}

template<typename K,typename V>
std::ostream& operator<<(std::ostream& o,std::map<K,V> const& a){
	return o<<to_vec(a);
}

template<typename K,typename V>
std::map<K,V>& operator+=(std::map<K,V>& a,std::map<K,V> const& b){
	for(auto [k,v]:b){
		a[k]+=v;
	}
	return a;
}

//start program-specific code

using namespace std;

int points(auto &f,frc_api::Season season,int awardId){
	//obviously very slow to run this every time.  Could easily make this get cached.
	auto aw=run(f,frc_api::Award_listings{season});
	assert(aw);

	auto f0=filter([=](auto x){ return x.awardId==awardId; },aw->awards);
	auto names=mapf([](auto x){ return x.description; },f0);
	if(!all_equal(names)){
		PRINT(names);
		nyi
	}
	assert(names.size());
	map<string,int> m{
		{"District Chairman's Award",10},
		{"District Engineering Inspiration Award",8},
		{"District Event Winner",0},
		{"District Event Finalist",0},
		{"Industrial Safety Award sponsored by Underwriters Laboratories",5},
		{"Industrial Design Award sponsored by General Motors",5},
		{"Highest Rookie Seed",0},
		{"Judges' Award",5},
		{"Rookie All Star Award",5},
		{"Rookie Inspiration Award",5},
		{"Entrepreneurship Award sponsored by Kleiner Perkins Caufield and Byers",5},
		{"Team Spirit Award sponsored by Chrysler",5},
		{"Excellence in Engineering Award sponsored by Delphi",5},
		{"Gracious Professionalism Award sponsored by Johnson & Johnson",5},
		{"Creativity Award sponsored by Xerox",5},
		{"Quality Award sponsored by Motorola",5},
		{"Innovation in Control Award sponsored by Rockwell Automation",5},
		{"Imagery Award in honor of Jack Kamen",5},

		//also appears at DCMP
		{"Regional Chairman's Award",0}, //auto advance, so pts don't matter
		{"Regional Engineering Inspiration Award",0},

		//not a team award
		{"FIRST Dean's List Finalist Award",0},

		{"District Championship Winner",0},//autoadv
		{"District Championship Finalist",0},
		{"Woodie Flowers Finalist Award",0},

		//obviously will end up getting multiplied by 3.
		{"District Championship Rookie All Star Award",5},

		{"Volunteer of the Year",0},
		{"Rookie Inspiration Award sponsored by National Instruments",5},
		{"Team Spirit Award sponsored by FCA Foundation",5},
		{"Quality Award sponsored by Motorola Solutions Foundation",5},

		//not really an award
		{"District Championship Points Qualifying Team",0},
		{"FIRST Championship District Points Qualifying Team",0},

		{"Safety Award sponsored by Underwriters Laboratories",5},
		{"Autonomous Award sponsored by Ford",5},
		{"Entrepreneurship Award",5},
		{"Team Spirit Award",5},
		{"Excellence in Engineering Award",5},
		{"Gracious Professionalism Award",5},
		{"Creativity Award sponsored by Rockwell Automation",5},
		{"Quality Award",5},
		{"Innovation in Control Award",5},
	};
	auto f1=m.find(names[0]);
	if(f1==m.end()){
		PRINT(names[0]);
		nyi
	}
	return f1->second;
}

using Color=frc_api::Alliance_color;

Color color(frc_api::Station a){
	switch(a){
		case frc_api::Station::Red1:
		case frc_api::Station::Red2:
		case frc_api::Station::Red3:
			return Color::Red;
		case frc_api::Station::Blue1:
		case frc_api::Station::Blue2:
		case frc_api::Station::Blue3:
			return Color::Blue;
		default:
			assert(0);
	}
}

using Team=frc_api::Team_number;

enum class Level{OF,QF,SF,F};

std::ostream& operator<<(std::ostream& o,Level a){
	switch(a){
		case Level::OF: return o<<"OF";
		case Level::QF: return o<<"QF";
		case Level::SF: return o<<"SF";
		case Level::F: return o<<"F";
		default: assert(0);
	}
}

struct Match_data{
	Level level;
	map<Color,set<Team>> teams;
	Color winner;
};

std::ostream& operator<<(std::ostream& o,Match_data const& a){
	o<<"Match_data(";
	o<<a.level<<" ";
	o<<a.teams<<" ";
	o<<a.winner;
	return o<<")";
}

optional<map<frc_api::Team_number,int>> playoff_pts(
	auto& f,
	frc_api::Season const& year,
	frc_api::Event_code const& event_code
){
	auto x1=run(
		f,
		frc_api::Match_results{
			year,
			event_code,
			frc_api::M2{
				frc_api::Tournament_level::Playoff,
				{},{},{}
			}
		}
	);
	assert(x1);
	if(x1->Matches.size()==0){
		return std::nullopt;
	}
	//PRINT(x1->Matches.size());
	//print_r(x1->Matches);
	//print_lines(enumerate(x1->Matches));

	auto as=run(f,frc_api::Alliance_selection{year,event_code});
	assert(as);

	auto alliances=mapf(
		[](auto x){
			set<Team> teams;
			teams|=x.captain;
			teams|=x.round1;
			teams|=x.round2;
			teams|=x.round3;
			teams|=x.backup;
			return make_pair(x.number,teams);
		},
		as->Alliances
	);

	auto alliance=[&](set<frc_api::Team_number> v)->int{
		//print_r(as->Alliances);
		auto f=filter([=](auto x){ return (v&x.second).size(); },alliances);
		assert(f.size()==1);
		return f[0].first;
	};

	vector<Match_data> v;

	//print_r(x1->Matches);

	Level level_last=Level::OF;
	for(auto match:x1->Matches){
		auto sp=split(match.description);
		assert(sp.size());
		Level level;
		if(sp[0]=="Quarterfinal"){
			level=Level::QF;
		}else if(sp[0]=="Semifinal"){
			level=Level::SF;
		}else if(sp[0]=="Finals" || sp[0]=="Final"){
			level=Level::F;
		}else if(sp[0]=="Tiebreaker" || sp[0]=="Overtime"){
			level=level_last;
		}else if(sp[0]=="Octofinal"){
			level=Level::OF;
		}else{
			PRINT(sp);
			nyi
		}
		level_last=level;

		map<Color,set<Team>> teams;
		for(auto team:match.teams){
			/*if(team.dq){
				print_r(match);
			}
			assert(team.dq==0);*/
			teams[color(team.station)]|=team.teamNumber;
		}
		auto r=match.scoreRedFinal;
		auto b=match.scoreBlueFinal;

		//ignore ties		
		if(r==b) continue;

		v|=Match_data{level,teams,(r>b)?Color::Red:Color::Blue};
	}
	//print_lines(v);

	//calculate the result for each alliance number
	//then calculate how many matches each of their members contributed to that
	//this is harder than it really seems like it should be due to 2015.
	//auto g=group([&](auto x){ return map_values(alliance,x.teams); },v);
	//print_r(g);

	auto finals_matches=filter([](auto x){ return x.level==Level::F; },v);

	auto winning_alliance_num=[=](auto x){
		return alliance(x.teams[x.winner]);
	};

	auto ff=mapf(winning_alliance_num,finals_matches);
	//PRINT(ff);
	//assert(ff.size()==2 || ff.size()==3);

	auto won=winning_alliance_num(last(finals_matches));
	//PRINT(won);
	using Alliance_num=int;
	enum Alliance_finish{OF,QF,SF,F,W};
	map<Alliance_num,Alliance_finish> finish;
	finish[won]=W;
	//for all of the rest of the alliances, their result is defined by the highest level in which they played a match.

	auto to_finish=[](Level l)->Alliance_finish{
		switch(l){
			case Level::OF: return OF;
			case Level::QF: return QF;
			case Level::SF: return SF;
			case Level::F: return F;
			default:
				assert(0);
		}
	};

	auto mark=[&](Level level,set<Team> const& teams){
		auto a=alliance(teams);
		auto f=finish.find(a);
		Alliance_finish fin;
		if(f==finish.end()){
			fin=QF;
		}else{
			fin=f->second;
		}
		finish[a]=max(fin,to_finish(level));
	};

	for(auto x:v){
		mark(x.level,x.teams[Color::Red]);
		mark(x.level,x.teams[Color::Blue]);
	}

	//PRINT(finish);

	//for each match, look at the winning alliance
	//if they advanced, then credit the teams playing for them
	map<Team,int> r;
	for(auto match:v){
		//PRINT(match);
		auto winners=match.teams[match.winner];
		auto an=alliance(winners);
		if(finish[an]>to_finish(match.level)){
			for(auto team:winners){
				r[team]+=5;
			}
		}
	}
	return r;
}

optional<map<frc_api::Team_number,int>> rank_pts(
	auto &f,
	frc_api::Season const& year,
	frc_api::Event_code const& event_code
){
	auto a=run(
		f,
		frc_api::Event_rankings{year,event_code,{}}
	);
	assert(a);
	auto m=mapf([](auto x){ return x.rank; },a->Rankings);
	if(m.empty()) return std::nullopt;
	assert(m.empty() || to_set(m)==range(min(m),max(m)+1));
	auto teams=max(m);
	//print_lines(enumerate(a->Rankings));
	return to_map(mapf(
		[=](auto x){
			return make_pair(x.teamNumber,rank_pts(teams,x.rank));
		},
		a->Rankings
	));
}

std::optional<map<frc_api::Team_number,int>> alliance_pts(
	auto &f,
	frc_api::Season const& year,
	frc_api::Event_code const& event_code
){
	try{
		auto as=run(f,frc_api::Alliance_selection{year,event_code});
		assert(as);
		assert(as->count>=0);
		assert(as->Alliances.size()==unsigned(as->count));
		auto i=as->Alliances.size();
		if(i!=8 && i!=16 && i!=4 && i!=2){
			//assert(as->Alliances.size()==8);
			PRINT(as->Alliances.size());
			nyi
		}
		//print_lines(as->Alliances);
		map<frc_api::Team_number,int> r;
		//TODO: Check what the values are for different-size events.
		auto mark=[&](optional<frc_api::Team_number> t,int value){
			if(t){
				r[*t]=value;
			}
		};
		for(auto alliance:as->Alliances){
			mark(alliance.captain,17-alliance.number);
			mark(alliance.round1,17-alliance.number);
			mark(alliance.round2,alliance.number);
		}
		return r;
	}catch(frc_api::HTTP_error const& e){
		//should get a code 500.
		//The cancelled 2020 events do this.
		//cout<<"No alliance selection for:"<<year<<event_code<<"\n";
		//nyi
		return std::nullopt;
	}
}

optional<map<frc_api::Team_number,int>> award_points(
	auto &f,
	frc_api::Season const& year,
	frc_api::Event_code const& event_code
){
	auto aw=run(f,frc_api::Event_awards{year,event_code});

	if(aw.Awards.empty()){
		//obviously, this means that we don't know who won the awards rather 
		//than that there were no awards given.
		return std::nullopt;
	}

	//TODO: Will want to make this note when winning an auto-advance award
	map<frc_api::Team_number,int> r;
	for(auto x:aw.Awards){
		if(x.teamNumber){
			auto n=*x.teamNumber;
			auto p=points(f,year,x.awardId);
			r[n]+=p;
		}
	}
	return r;
}

auto analyze_event(auto &f,frc_api::Season const& year,frc_api::Event_code const& event_code){
	map<frc_api::Team_number,int> r;
	vector<string> missing;

	auto total=[&](string name,auto x){
		if(x){
			r+=*x;
		}else{
			missing|=name;
		}
	};

	total("rank",rank_pts(f,year,event_code));
	total("alliance",alliance_pts(f,year,event_code));
	total("award",award_points(f,year,event_code));
	total("playoff",playoff_pts(f,year,event_code));
	return make_pair(missing,r);
}

map<frc_api::District_code,vector<Team>> teams(FRC_api_fetcher& f,frc_api::Season year){
	auto get_page=[&](int page){
		auto r=run(
			f,
			frc_api::Team_listings{
				year,
				//frc_api::Which4{}
				std::tuple<
					std::optional<frc_api::Event_code>,
					std::optional<frc_api::District_code>,
					std::optional<frc_api::State>,
					int
				>{std::nullopt,std::nullopt,std::nullopt,page}
			}
		);
		assert(r.pageCurrent==page);
		return r;
	};

	map<frc_api::District_code,vector<Team>> r;
	auto p=get_page(1);
	for(auto i:range(1,1+p.pageTotal)){
		auto x=get_page(i);
		for(auto team:x.teams){
			if(team.districtCode){
				r[*team.districtCode]|=team.teamNumber;
			}
		}
	}
	return r;
}

optional<map<Team,pair<vector<int>,optional<int>>>> analyze_district(
	FRC_api_fetcher &f,
	frc_api::Season year,
	frc_api::District_code district
){
	PRINT(district);

	auto team_list=teams(f,year)[district];

	auto r2=run(
		f,
		frc_api::Event_listings{
			year,frc_api::Event_criteria{std::nullopt,district}
		}
	);

	auto x=sort_by([](auto x){ return x.dateStart; },r2.Events);

	map<Team,vector<int>> pts_earned;
	map<Team,int> dcmp_pts;
	for(auto event:x){
		auto [error,result]=analyze_event(f,year,event.code);

		//Right now, no data for 2022oncmp1
		//seems just like a dummy and they ran the whole thing as a normal event.
		if(result.empty()) continue;

		switch(event.type){
			case frc_api::Event_Event_type::DistrictEvent:{
				if(error.size()){
					return std::nullopt;
				}
				if(error.size()){
					print_r(event);
					PRINT(error)
				}
				assert(error.empty());
				for(auto [team,pts]:result){
					auto &v=pts_earned[team];
					if(v.size()<2){
						v|=pts;
					}
				}
				break;
			}
			case frc_api::Event_Event_type::DistrictParent:
				//no idea what these are used for.
				break;
			case frc_api::Event_Event_type::DistrictChampionshipDivision:
			case frc_api::Event_Event_type::DistrictChampionship:{
				if(error.size()){
					PRINT(error);
					print_r(event);
					print_r(result);
				}
				if(error.size()) return std::nullopt;
				for(auto [team,pts]:result){
					dcmp_pts[team]+=pts*3;
				}
				break;
			}
			case frc_api::Event_Event_type::DistrictChampionshipWithLevels:
				//This is the equivalent of the Einstein field.
				//So we expect there to be no ranks.
				if(error!=vector<string>{"rank"}){
					return std::nullopt;
				}
				assert(error==vector<string>{"rank"});
				for(auto [team,pts]:result){
					dcmp_pts[team]+=pts*3;
				}
				break;
			default:{
				print_r(event);
				auto y=analyze_event(f,year,event.code);
				print_r(y);
				nyi
			}
		}
	}


	//then total up the points that are earned at those events
	using V=std::pair<vector<int>,optional<int>>;
	map<Team,V> r;
	for(auto team:keys(pts_earned)|keys(dcmp_pts)){
		r[team]=make_pair(
			[&](){
				auto f=pts_earned.find(team);
				if(pts_earned.end()==f){
					PRINT(team);
					PRINT(pts_earned.size());
				}
				if(f==pts_earned.end()){
					return vector<int>{};
				}else{
					return f->second;
				}
			}(),
			[&]()->optional<int>{
				auto f=dcmp_pts.find(team);
				if(f==dcmp_pts.end()){
					return std::nullopt;
				}
				return f->second;
			}()
		);
	}
	return r;
}
