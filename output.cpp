#include "output.h"
#include<cmath>
#include<iomanip>
#include<fstream>
#include "../tba/tba.h"
#include "map.h"
#include "util.h"
#include "tba.h"

using namespace std;

map<Point,Pr> simplify(map<pair<Point,Pr>,Pr> const& m){
	map<Point,Pr> r;
	for(auto [k,v]:m){
		r[k.first]+=v;
	}
	return r;
}

double entropy(Pr p){
	//units are bits.
	if(p<0) p=0;
	if(p>1) p=1;
	if(p==0 || p==1) return 0;
	assert(p>0 && p<1);
	return -(log(p)*p+log(1-p)*(1-p))/log(2);
}

string make_link(tba::Team_key team){
	auto s=team.str();
	assert(s.substr(0,3)=="frc");
	auto t=s.substr(3,500);
	return link("https://www.thebluealliance.com/team/"+t,t);
}

char digit(auto i){
	if(i<10) return '0'+i;
	return 'a'+(i-10);
}

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

int as_num(tba::Team_key const& a){
	return atoi(a.str().c_str()+3);
}

template<typename T>
T find_cutoff(map<T,Pr> const& cutoff_pr,double threshold){
	auto t=sum(seconds(cutoff_pr));
	assert(t>.99 && t<1.01);
	double total=0;
	for(auto [a,p]:cutoff_pr){
		total+=p;
		if(total>=threshold){
			return a;
		}
	}
	assert(0);
}

template<typename T>
tuple<T,T,T> summary(map<T,Pr> const& a){
	return make_tuple(
		find_cutoff(a,.05),
		find_cutoff(a,.5),
		find_cutoff(a,.95)
	);
}

template<typename T>
string join(string const& s,vector<T> const& v){
	if(v.empty()) return "";

	std::stringstream ss;
	auto at=v.begin();
	ss<<*at;
	at++;
	while(at!=v.end()){
		ss<<s<<*at;
		at++;
	}
	return ss.str();
}

string gen_html(
	vector<tuple<
		tba::Team_key,
		Pr,Point,Point,Point,
		Pr,Point,Point,Point
	>> const& result,
	vector<tba::Team> const& team_info,
	map<Extended_cutoff,Pr> const& dcmp_cutoff_pr,
	map<Extended_cutoff,Pr> const& cmp_cutoff_pr,
	string const& title,
	string const& district_short,
	tba::Year year,
	int dcmp_size,
	map<tba::Team_key,tuple<vector<int>,int,int>> points_used
){
	auto nickname=[&](auto k){
		auto f=filter_unique([=](auto a){ return a.key==k; },team_info);
		auto v=f.nickname;
		assert(v);
		return *v;
	};

	auto data_used_table=[&](){
		return h2("Team data used")+
			tag("table border",
				tr(th("Team")+th("Rookie Points")+th("Played")+th("Remaining events"))
				+join(::mapf(
					[](auto x){
						auto [team,data]=x;
						auto [played,rookie,events_left]=data;
						return tr(td(as_num(team))+td(rookie)+td(played)+td(events_left));
					},
					sorted(to_vec(points_used),[](auto x){ return as_num(x.first); })
				))
			);
	}();

	auto cutoff_table=[=](string s,auto cutoff_pr){
		auto simple=simplify(cutoff_pr);
		return h2(s+" cutoff value")+
		table(tr(
			td(tag("table border",
				tr(th("Points")+th("Probability"))+
				join(mapf(
					[](auto a){
						return tr(join(MAP(td,a)));
					},
					simple
				))
			))+
			td(
				h3("Summary")+
				tag("table border",
					tr(th("Probability")+th("Point total"))+
					[=](){
						auto q=summary(simple);
						return
							tr(th("5%")+td(std::get<0>(q)))+
							tr(th("Median")+td(std::get<1>(q)))+
							tr(th("95%")+td(std::get<2>(q)))
						;
					}()
				)
			)
		));
	};

	auto cutoff_table1=cutoff_table("District Championship",dcmp_cutoff_pr);
	auto cutoff_table_cmp=cutoff_table("FRC Championship",cmp_cutoff_pr);

	auto cutoff_table_long=[=](){
		return h2("Cutoff value - extended")+
		"The cutoff values, along with how likely a team at that value is to miss advancing.  For example: a line that said (50,.25) would correspond to the probability that team above 50 get in, teams below 50 do not, and 75% of teams ending up with exactly 50 would qualify for the district championship."+
		tag("table border",
			tr(th("Points")+th("Probability"))+
			join(mapf(
				[](auto a){
					return tr(join(MAP(td,a)));
				},
				dcmp_cutoff_pr
			))
		);
	}();

	double total_entropy=sum(::mapf(entropy,seconds(result)));
	PRINT(total_entropy);

	static const std::vector<std::pair<std::string,std::string>> columns{
		{"Rank","Ranking of probability of advancement"},
		{"P<sub>DCMP</sub>","Probability of making district championship"},
		{"Team","Team number"},
		{"Nickname","Team nickname"},
		{"DCMP 5% pts","Extra points needed to have 5% chance of making district championship"},
		{"DCMP 50% pts","Extra points needed to have 50% chance of making district championship"},
		{"DCMP 95% pts","Extra points needed to have 95% chance of making district championship"},
		{"P<sub>CMP</sub>","Probability of making championship"},
		{"CMP 5% pts","Extra points needed to have a 5% chance of making championship"},
		{"CMP 50% pts","Extra points needed to have a 50% chance of making championship"},
		{"CMP 95% pts","Extra points needed to have a 95% chance of making championship"},
		{"Rookie Points","Rookie bonus points awarded"},
		{"Played","Results from events played so far"},
		{"Remaining events","Number of counting events for which the team is scheduled"}
	};

	auto explain=tag("table border",
		tr(th("Column")+th("Description"))+
		join(mapf(
			[](auto a){
				return tr(td(a.first)+td(a.second));
			},
			columns
		))
	);

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
			cutoff_table1+
			h2("Team Probabilities")+
			explain+
			tag("table border",
				tr(join(::mapf(
					[](auto x){ return th1(x.first); },
					columns
				)))+
				join(
					::mapf(
						[=](auto p){
							auto [i,a]=p;
							auto used=points_used.find(get<0>(a))->second;
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
								)
								)+
								colorize(get<5>(a))+
								td(get<6>(a))+
								td(get<7>(a))+
								td(get<8>(a))+
								td(get<1>(used))+ //rookie points
								td(join("&nbsp;",get<0>(used)))+ //played
								td(get<2>(used)) //remaining events
							);
						},
						enumerate_from(1,reversed(sorted(
							result,
							[](auto x){ return make_tuple(get<1>(x),get<5>(x),x); }
						)))
					)
				)
			)+
			cutoff_table_long+cutoff_table_cmp /*+data_used_table*/
		)
	);
}

pair<tba::Event_key,std::string> championship_event(auto &f,tba::District_key const& d){
	auto f1=filter([](auto x){
		return x.event_type==tba::Event_type::DISTRICT_CMP; },
		district_events_simple(f,d)
	);
	assert(f1.size()==1);
	auto e=f1[0];
	return make_pair(e.key,e.name);
}

int team_number(tba::Team_key const& a){
	return atoi(a.str().c_str()+3);
}

int make_spreadsheet(
	TBA_fetcher &f,
	map<tba::District_key,map<tba::Team_key,Pr>> const& m,
	string const& output_dir
){
	//write a csv with all the data, then use LibreOffice to convert it to an Excel spreadsheet
	//This output exists specifically for the use in this thread:
	//https://www.chiefdelphi.com/t/2022-frc-robot-data-is-beautiful-see-if-you-can-find-your-team/405227

	string filename="results.csv";
	{
		ofstream o(output_dir+"/"+filename);
		o<<"Team #,CMP,Event,P(DCMP)\n";
		for(auto [district,teams]:m){
			auto [event_key,event_name]=championship_event(f,district);
			for(auto [team,p]:teams){
				o<<team_number(team)<<","<<event_key<<","<<event_name<<","<<p<<"\n";
			}
		}
	}

	//This is a call to LibreOffice or OpenOffice
	//If neither of those is installed or in the path, this may error out
	//Assuming that this is found though, it will produce some onscreen output that is not especially informative.
	//It wouldn't be the worst thing to send that to /dev/null.
	return system( ("cd "+output_dir+"; soffice --convert-to xlsx "+filename).c_str() );
}
