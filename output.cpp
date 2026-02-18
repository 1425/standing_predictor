#include "output.h"
#include<cmath>
#include<iomanip>
#include<fstream>
#include "../tba/tba.h"
#include "map.h"
#include "util.h"
#include "tba.h"
#include "run.h"
#include "outline.h"
#include "vector_void.h"
#include "plot.h"
#include "print_r.h"
#include "ca.h"

using namespace std;
using Team_key=tba::Team_key;

std::ostream& operator<<(std::ostream& o,Team_points_used const& a){
	o<<"Team_points_used(";
	#define X(A,B) o<<""#B<<":"<<a.B<<" ";
	TEAM_POINTS_USED(X)
	#undef X
	return o<<")";
}

std::ostream& operator<<(std::ostream& o,Output_tuple const& a){
	o<<"Output_tuple( ";
	#define X(A) o<<""#A<<":"<<a.A<<" ";
	X(team)
	X(dcmp_home)
	X(dcmp_make)
	X(dcmp_interesting)
	X(cmp_make)
	X(cmp_interesting)
	#undef X
	return o<<")";
}

map<Point,Pr> simplify(map<pair<Point,Pr>,Pr> const& m){
	map<Point,Pr> r;
	for(auto [k,v]:m){
		r[k.first]+=v;
	}
	return r;
}

map<Point,Pr> simplify(flat_map2<pair<Point,Pr>,Pr> const& m){
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

auto find_charts(std::map<Team_key,Team_points_used> const& a){
	std::vector<Plot_setup> setups;
	for(auto [k,v]:a){
		Plot_setup p;
		p.title=::as_string(k);
		p.data=mapf([](auto x){ return Plot_point(x.first,x.second); },v.pre_dcmp_dist);
		setups|=p;
	}
	auto plots=plot(setups);
	return to_map(mapf(
		[](auto const& x){
			auto [in,out]=x;
			return make_pair(in.first,out);
		},
		zip(a,plots)
	));
}

string gen_html(
	vector<Output_tuple> const& result,
	vector<tba::Team> const& team_info,
	std::array<Cutoff2,2> const& dcmp_cutoff_pr,
	Cutoff const& cmp_cutoff_pr,
	string const& title,
	string const& district_short,
	tba::Year year,
	std::vector<int> dcmp_size,
	map<tba::Team_key,Team_points_used> points_used
){
	std::map<tba::Team_key,tba::Team> by_team;
	for(auto x:team_info){
		by_team.insert(make_pair(x.key,x));
		//by_team[x.key]=x;
	}

	auto dcmp_string=[&](tba::Team_key t){
		auto f=by_team.find(t);
		assert(f!=by_team.end());
		auto f1=f->second;
		if(f1.state_prov!="California"){
			return string();
		}
		return as_string(california_region(f1));
	};
	auto by_dcmp=group([&](auto x){ return dcmp_string(x.team); },result);
	//PRINT(by_dcmp);
	map<tba::Team_key,std::string> team_str;
	for(auto &p:by_dcmp){
		auto &l=p.second;
		std::sort(
			l.begin(),
			l.end(),
			[](auto a,auto b){
				auto t=[](auto x){ return make_tuple(x.dcmp_make,x.cmp_make,x); };
				return t(a)<t(b);
			}
		);

		std::reverse(l.begin(),l.end());
		auto m=mapf([](auto x){ return x.team; },l);
		for(auto [i,team]:enumerate_from(1,m)){
			std::stringstream ss;
			auto d=dcmp_string(team);
			ss<<d;
			if(!d.empty()){
				ss<<"("<<i<<" of "<<m.size()<<")";
			}
			team_str.insert(make_pair(team,ss.str()));
		}
	}

	auto get_team_str=[=](tba::Team_key t){
		auto f=team_str.find(t);
		assert(f!=team_str.end());
		return f->second;
	};

	auto nickname=[&](auto k){
		auto f=filter_unique([=](auto a){ return a.key==k; },team_info);
		auto v=f.nickname;
		assert(v);
		/*if(f.state_prov=="California"){
			return "("+as_string(california_region(f))+") "+*v;
		}*/
		return *v;
	};

	auto data_used_table=[&](){
		return h2("Team data used")+
			tag("table border",
				tr(th("Team")+th("Rookie Points")+th("Played")+th("Remaining events"))
				+join(::mapf(
					[](auto x){
						auto [team,data]=x;
						return tr(td(as_num(team))+td(data.rookie_bonus)+td(data.event_points_earned)+td(data.events_left));
					},
					sorted(to_vec(points_used),[](auto x){ return as_num(x.first); })
				))
			);
	}();

	auto cutoff_table=[=](string s,auto cutoff_pr){
		auto simple=simplify(cutoff_pr);
		auto chart=plot([&](){
			std::vector<std::pair<int,double>> r;
			auto ks=keys(simple);
			for(auto k:range_inclusive(min(ks),max(ks))){
				r|=make_pair(k,simple[k]);
			}
			return r;
		}());
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
			)+td(chart)
		));
	};

	auto dcmp_name=[=](int i)->string{
		if(district_short=="ca"){
			switch(i){
				case 0:
					return "NORTH";
				case 1:
					return "SOUTH";
				default:
					assert(0);
			}
		}
		return "";
	};

	auto dcmp_names=(district_short=="ca")?2:1;

	//auto cutoff_table1=cutoff_table("District Championship",dcmp_cutoff_pr);
	auto cutoff_table1=join(mapf(
		[=](auto i){
			return cutoff_table("District Championship "+dcmp_name(i),dcmp_cutoff_pr[i]);
		},
		range(dcmp_names)
	));
	auto cutoff_table_cmp=cutoff_table("FRC Championship",cmp_cutoff_pr);

	auto cutoff_table_long1=[=](auto data){
		return h2("Cutoff value - extended")+
		"The cutoff values, along with how likely a team at that value is to miss advancing.  For example: a line that said (50,.25) would correspond to the probability that team above 50 get in, teams below 50 do not, and 75% of teams ending up with exactly 50 would qualify for the district championship."+
		tag("table border",
			tr(th("Points")+th("Probability"))+
			join(mapf(
				[](auto a){
					return tr(join(MAP(td,a)));
				},
				data
			))
		);
	};

	auto cutoff_table_long=join(mapf(
		[=](auto i){ return cutoff_table_long1(dcmp_cutoff_pr[i]); },
		range(dcmp_names)
	));

	//double total_entropy=sum(::mapf(entropy,seconds(result)));
	double total_entropy=sum(::mapf([](auto x){ return entropy(x); },mapf([](auto x){ return x.dcmp_make; },result)));
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

	//std::map<Team_key,std::string> charts; //TODO: Put a bunch of stuff here.
	auto charts=find_charts(points_used);

	auto fancy=[&](auto a){
		//nickname(a.team),
		auto x=points_used[a.team];
		//print_r(x);
		stringstream ss;
		ss<<"<span class=\"tooltip\">"<<nickname(a.team);
		ss<<"<span class=\"tooltiptext\">"<<h3("Expected "+::as_string(a.team)+ " pre-dcmp points")+charts[a.team]<<"</span>";
		ss<<"</span>\n";
		return ss.str();
		//return nickname(a.team)+as_string(quartiles(x.pre_dcmp_dist))+charts[a.team];
	};

	string style="\n\
		.tooltip{\n\
		        position:relative;\n\
		        display: inline-block;\n\
		        border-bottom: 1px dotted black;\n\
		        cursor: pointer;\n\
		}\n\
		.tooltip .tooltiptext{\n\
		        visibility: hidden;\n\
		        background-color: black;\n\
		        color: #ffffff;\n\
		        text-align: center;\n\
		        padding: 5px 0;\n\
		        border-radius: 6px;\n\
		        position: absolute;\n\
		        z-index: 1;\n\
		        bottom: 100%;\n\
		        left: 50%;\n\
		        margin-left: -65px;\n\
		}\n\
		.tooltip:hover .tooltiptext{\n\
		        visibility: visible;\n\
		}\n";


	return tag("html",
		tag("head",
			tag("title",title)+tag("style",style)
		)+
		tag("body",
			tag("h1",title)+
			link("https://frc-events.firstinspires.org/"+::as_string(year)+"/district/"+district_short,"FRC Events")+"<br>"+
			link("https://www.thebluealliance.com/events/"+district_short+"/"+::as_string(year)+"#rankings","The Blue Alliance")+"<br>"+
			//link("http://frclocks.com/index.php?d="+district_short,"FRC Locks")+"(slow)<br>"+
			link("http://frclocks.com/districts/"+district_short+".html","FRC Locks")+"<br>"+
			"Slots at district championship:"+[=](){
				if(dcmp_size.size()==1){
					return as_string(dcmp_size[0]);
				}
				return as_string(dcmp_size);
			}()+
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
							auto used=points_used.find(a.team)->second;
							return tr(join(
								vector<string>{}+td(as_string(i)+" "+get_team_str(a.team))+
								colorize(a.dcmp_make)+
								::mapf(
									td1,
									std::vector<std::string>{
										make_link(a.team),
										//nickname(a.team),
										fancy(a),
										as_string(a.dcmp_interesting[0]),
										as_string(a.dcmp_interesting[1]),
										as_string(a.dcmp_interesting[2])
									}
								)
								)+
								colorize(a.cmp_make)+
								td(a.cmp_interesting[0])+
								td(a.cmp_interesting[1])+
								td(a.cmp_interesting[2])+
								td(used.rookie_bonus)+
								td(join("&nbsp;",used.event_points_earned))+
								td(used.events_left)
							);
						},
						enumerate_from(1,reversed(sorted(
							result,
							[](auto x){ return make_tuple(x.dcmp_make,x.cmp_make,x); }
						)))
					)
				)
			)+
			cutoff_table_long+cutoff_table_cmp /*+data_used_table*/
		)
	);
}

std::vector<pair<tba::Event_key,std::string>> championship_event(auto &f,tba::District_key const& d){
	auto f1=filter(
		[](auto x)->bool{
			//This is here because there is an event whose code is "mbfrc25"
			//And is named "DO NOT APPLY - FRC Test Event for BV"
			//But it says that it is a second 2025 New England district championship.
			if(prefix(x.name,"DO NOT APPLY - FRC Test Event")){
				return 0;
			}

			return x.event_type==tba::Event_type::DISTRICT_CMP;
		},
		district_events_simple(f,d)
	);
	assert(f1.size()>=1);
	assert(f1.size()<=MAX_DCMPS);
	return mapf(
		[](auto x){ return make_pair(x.key,x.name); },
		f1
	);
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
			//auto [event_key,event_name]=championship_event(f,district);
			auto e=championship_event(f,district);
			auto [event_key,event_name]=e[0];
			for(auto [team,p]:teams){
				auto [event_key,event_name]=[&](){
					if(e.size()==1){
						return e[0];
					}
					auto x=calc_dcmp_home(f,team);
					assert(x<e.size());
					return e[x];
				}();
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
