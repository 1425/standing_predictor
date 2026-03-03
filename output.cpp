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
#include "set_flat.h"
#include "avatar.h"

using namespace std;
using Team_key=tba::Team_key;

struct Script_namer{
	size_t i=0;

	string operator()(){
		std::stringstream ss;
		ss<<"n"<<i;
		i++;
		return ss.str();
	}
};

auto td_top(auto a){
	return tag("td valign=top",a);
}

std::string as_table(tba::Event const&);

template<typename K,typename V>
std::string as_table(std::map<K,V> const& a);

template<typename K,typename V>
std::string as_table(flat_map2<K,V> const& a);

template<typename T>
std::string as_table(T const& t){
	return ::as_string(t);
}

template<typename T>
auto tr_hide(std::string name,T const& contents);

template<typename K,typename V>
std::string as_table(flat_map2<K,V> const& a){
	std::stringstream ss;
	ss<<"<table border>";
	for(auto const& [k,v]:a){
		ss<<tr_hide(as_string(k),v);
	}
	ss<<"</table>";
	return ss.str();
}

template<typename K,typename V>
std::string as_table(std::map<K,V> const& a){
	std::stringstream ss;
	ss<<"<table border>";
	for(auto const& [k,v]:a){
		ss<<tr_hide(::as_string(k),v);
	}
	ss<<"</table>";
	return ss.str();
}

template<typename T>
std::string as_table(std::vector<T> const& a){
	std::stringstream ss;
	ss<<"<table border>";
	for(auto const& x:a){
		ss<<tr(td(as_table(x)));
	}
	ss<<"</table>";
	return ss.str();
}

template<typename T>
auto tr_hide(std::string name,T const& contents){
	std::string inner=as_table(contents);
	if(inner.size()<100){
		return tr(td(name)+td(inner));
	}
	auto name1="x"+as_string(rand());
	return tr(
		td_top(tag("a href=\"\" onclick=\"toggle_viz('"+name1+"');event.preventDefault();\"",name))+
		tag("td class=hidden id=\""+name1+"\"",inner)
	);
}

#define TR_HIDE(A,B) ss<<tr_hide(""#B,a.B);
#define TR_HIDE1(A) ss<<tr_hide(""#A,a.A);

template<typename T>
std::string as_table(Rank_status<T> const& a){
	std::stringstream ss;
	ss<<"<table border>";
	RANK_STATUS(TR_HIDE)
	ss<<"</table>";
	return ss.str();
}

std::string as_table(tba::Event const& a){
	auto name="x"+as_string(rand());
	std::stringstream ss;
	ss<<tag("a href=\"\" onclick=\"toggle_viz('"+name+"');event.preventDefault();\"",a.short_name);
	ss<<"<table border class=hidden id=\""<<name<<"\">";
	#define X(A,B) ss<<tr(td_top(""#B)+td(as_table(a.B)));
	TBA_EVENT(X)
	#undef X
	ss<<"</table>";
	return ss.str();
}

template<typename T>
std::string as_table(Event_annotated<T> const& a){
	std::stringstream ss;
	ss<<"<table border>";
	TR_HIDE1(data)
	TR_HIDE1(extra)
	ss<<"</table>";
	return ss.str();
}

template<typename A,typename B>
std::string as_table(District_cmp_complex_annotated<A,B> const& a){
	std::stringstream ss;
	ss<<"<table border>";
	TR_HIDE1(finals)
	TR_HIDE1(divisions)
	TR_HIDE1(extra)
	ss<<"</table>";
	return ss.str();
}

auto click(std::string target,std::string body){
	return tag("a href=\"\" onclick=\"toggle_viz('"+target+"');event.preventDefault();\"",body);
}

std::string as_table(Team_points_used const& a){
	std::stringstream ss;
	ss<<"<table border>";
	TEAM_POINTS_USED(TR_HIDE)
	ss<<"</table>";
	return ss.str();
}

template<typename A,typename B,typename C>
std::string as_table(Event_categories_annotated<A,B,C> const& a){
	stringstream ss;
	ss<<"<table border>";
	TR_HIDE1(local)
	TR_HIDE1(dcmp)
	TR_HIDE1(extra)
	ss<<"</table>";
	return ss.str();
}

std::string as_table(Skill_estimates const& a){
	std::stringstream ss;
	ss<<"<table border>";
	SKILL_ESTIMATES(TR_HIDE)
	ss<<"</table>";
	return ss.str();
}


PRINT_STRUCT(Team_points_used,TEAM_POINTS_USED)

PRINT_STRUCT(Output_tuple,OUTPUT_TUPLE)

ELEMENTWISE_RAND(Output_tuple,OUTPUT_TUPLE)

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

auto colorize(double d){
	return tag("td align=center bgcolor=\""+color(d)+"\"",
		tag("font color=black",[&](){
			stringstream ss;
			ss<<setprecision(3)<<fixed;
			ss<<d;
			return ss.str();
		}())
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

std::map<tba::Team_key,std::string> find_charts(std::map<Team_key,Team_points_used> const& a){
	std::vector<Plot_setup> setups;
	for(auto [k,v]:a){
		Plot_setup p;
		p.title=::as_string(k);
		p.data=mapf([](auto x){ return Plot_point2(x.first,x.second); },v.pre_dcmp_dist);
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

template<typename T>
auto td_right(T const& t){
	return tag("td align=right",t);
}

std::string show_skill(Skill_estimates const& in){
	std::stringstream ss;
	ss<<h2("Skill estimates");
	ss<<as_table(in);

	auto get_plot_setup=[&](std::string name,auto data_in){
		std::vector<Plot_point3> data;
		for(auto [pts_in,v]:data_in){
			for(auto [pts_out,pr]:v){
				data|=Plot_point3(pts_in,pts_out,pr);
			}
		}
		return Plot_setup{data,name};
	};

	auto ps1=get_plot_setup("Second event",in.second_event);
	auto ps2=get_plot_setup("At DCMP",in.at_dcmp);

	auto out=plot(std::vector{ps1,ps2});
	/*vector out{
		plot(std::vector{ps1}),
		plot(std::vector{ps2})
	};*/

	assert(out.size()==2);
	ss<<h3("Probable second event points by first event points")<<out[0]<<"\n";
	ss<<h3("Probable District Championship points given pre-dcmp points")<<out[1]<<"\n";

	return ss.str();
}

void gen_html(
	std::ostream& o,
	Gen_html_input const& in,
	Event_categories_annotated<
		Rank_status<Tournament_status>,
		Tournament_status,
		Rank_status<District_status>
	> const& limits
){
	const auto by_team=[&](){
		std::map<tba::Team_key,tba::Team> r;
		for(auto x:in.team_info){
			r.insert(make_pair(x.key,x));
		}
		return r;
	}();

	int script_names_used=0;
	auto get_script_name=[&](){
		std::stringstream ss;
		ss<<"u"<<script_names_used;
		script_names_used++;
		return ss.str();
	};

	auto dcmp_string=[&](tba::Team_key t){
		auto f=by_team.find(t);
		if(f==by_team.end()){
			PRINT(in.district_short)
			PRINT(in.year)
			PRINT(t)
		}
		assert(f!=by_team.end());
		auto f1=f->second;
		if(f1.state_prov!="California"){
			return string();
		}
		return as_string(california_region(f1));
	};
	auto by_dcmp=group([&](auto const& x){ return dcmp_string(x.team); },in.result);
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
		auto v=by_team.at(k).nickname;
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
						return tr(td(as_num(team))+td(data.rookie_bonus)+td(data.event_points_earned)+td(data.events_left));
					},
					sorted(to_vec(in.points_used),[](auto x){ return as_num(x.first); })
				))
			);
	}();

	auto dcmp_name=[=](int i)->string{
		if(in.district_short=="ca"){
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

	auto dcmp_names=(in.district_short=="ca")?2:1;

	auto cutoff_table_long1=[=](auto data){
		return "The cutoff values, along with how likely a team at that value is to miss advancing.  For example: a line that said (50,.25) would correspond to the probability that team above 50 get in, teams below 50 do not, and 75% of teams ending up with exactly 50 would qualify for the district championship."+
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
		auto name=get_script_name();
		auto name2=get_script_name();
		return h2(s+" cutoff value")+
		table(tr(
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
				)+
				p(tag("a href='' onclick=\"toggle_viz('"+name+"');event.preventDefault();\"","Details"))
			)+
			td(chart)
		))+
			tag("table class='hidden' id=\""+name+"\"",
				tr(td(
					tag("table border",
						tr(th("Points")+th("Probability"))+
						join(mapf(
							[](auto a){
								return tr(join(MAP(td,a)));
							},
							simple
						))
					)+
					tag("a href='' onclick=\"toggle_viz('"+name2+"');event.preventDefault();\"","More details")+
					tag("table class=hidden id=\""+name2+"\"",
						tr(td(cutoff_table_long1(cutoff_pr)))
					)
				))
			);
	};

	//auto cutoff_table1=cutoff_table("District Championship",dcmp_cutoff_pr);
	auto cutoff_table1=join(mapf(
		[=](auto i){
			return cutoff_table("District Championship "+dcmp_name(i),in.dcmp_cutoff_pr[i]);
		},
		range(dcmp_names)
	));
	auto cutoff_table_cmp=cutoff_table("FRC Championship",in.cmp_cutoff_pr);

	//double total_entropy=sum(::mapf(entropy,seconds(result)));
	double total_entropy=sum(::mapf([](auto x){ return entropy(x); },mapf([](auto x){ return x.dcmp_make; },in.result)));
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

	auto explain=[&](){
		auto name=get_script_name();
		//		p(tag("a href='' onclick=\"toggle_viz('"+name+"');event.preventDefault();\"","Details"))
		return tag("a href=\"#\" onclick=\"toggle_viz('"+name+"');event.preventDefault();\"","Column descriptions")+
		tag("table border class=\"hidden\" id=\""+name+"\"",
			tr(th("Column")+th("Description"))+
			join(mapf(
				[](auto a){
					return tr(td(a.first)+td(a.second));
				},
				columns
			))
		);
	}();

	auto charts=[&]()->std::map<Team_key,std::string>{
		if(in.plot){
			return find_charts(in.points_used);
		}
		return {};
	}();

	auto fancy=[&](auto a){
		//nickname(a.team),
		//auto x=points_used[a.team];
		//print_r(x);
		stringstream ss;
		//ss<<"<div class=\"tooltip\">";

		ss<<table(tr(td(avatar(a.team))+td(nickname(a.team))))<<"\n";

		//ss<<"<span class=\"tooltiptext\">"<<h3("Expected "+::as_string(a.team)+ " pre-dcmp points")+charts[a.team]<<"</span>";
		//ss<<"</div>\n";
		return ss.str();
		//return nickname(a.team)+as_string(quartiles(x.pre_dcmp_dist))+charts[a.team];
	};

	auto team_details=[&](auto a)->std::string{
		auto x=in.points_used.at(a.team);
		std::stringstream ss;
		ss<<h3("Expected pre-dcmp points")+charts[a.team];

		const auto team_num=a.team.str().substr(3,20);

		{
			std::stringstream u;
			u<<"https://frc-events.firstinspires.org/"<<in.year<<"/team/"<<team_num;
			ss<<link(u.str(),"FRC Events");
		}
		ss<<"<br>";
		ss<<link("https://thebluealliance.com/team/"+team_num+"/"+::as_string(in.year),"The Blue Alliance");
		ss<<"<br>";
		{
			std::stringstream u;
			u<<"https://www.statbotics.io/team/"<<team_num<<"/"<<in.year;
			ss<<link(u.str(),"Statbotics");
		}

		ss<<h3("Points used");
		ss<<as_table(x);

		ss<<"<p>"<<"Lock status:"<<in.lock.at(a.team)<<"\n";
		return ss.str();
	};

	string style="\n\
		:root{\n\
			color-scheme: light dark;\n\
		}\n\
		table{\n\
			border-spacing: 0px;\n\
		}\n\
		tr.rank:hover{\n\
			background-color: #888888;\n\
		}\n\
		.hidden{\n\
			display: none;\n\
		}\n\
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

	auto script="\n\
		function toggle_viz(name){\n\
			var content=document.getElementById(name);\n\
			content.classList.toggle(\"hidden\");\n\
		}\n";

	o<<tag("html",
		tag("head",
			tag("title",in.title)+
			tag("style",style)+
			tag("script",script)
		)+
		tag("body",
			tag("h1",in.title)+
			link("https://frc-events.firstinspires.org/"+::as_string(in.year)+"/district/"+in.district_short,"FRC Events")+"<br>"+
			link("https://www.thebluealliance.com/events/"+in.district_short+"/"+::as_string(in.year)+"#rankings","The Blue Alliance")+"<br>"+
			//link("http://frclocks.com/index.php?d="+district_short,"FRC Locks")+"(slow)<br>"+
			link("http://frclocks.com/districts/"+in.district_short+".html","FRC Locks")+"<br>"+
			"Slots at district championship:"+[=](){
				if(in.dcmp_size.size()==1){
					return as_string(in.dcmp_size[0]);
				}
				return as_string(in.dcmp_size);
			}()+
			cutoff_table1+
			cutoff_table_cmp+
			h2("Team Probabilities")+
			p(explain)+
			tag("table border",
				tr(join(::mapf(
					[](auto x){ return th1(x.first); },
					columns
				)))+
				join(
					::mapf(
						[=](auto p){
							auto [i,a]=p;
							auto name=get_script_name();
							auto used=in.points_used.find(a.team)->second;
							return tag("tr class=\"rank\" onclick=\"toggle_viz('"+name+"');event.preventDefault();\"",
								td(as_string(i)+" "+get_team_str(a.team))+
								colorize(a.dcmp_make)+
								td_right(make_link(a.team))+
								td(fancy(a))+
								td_right(a.dcmp_interesting[0])+
								td_right(a.dcmp_interesting[1])+
								td_right(a.dcmp_interesting[2])+
								colorize(a.cmp_make)+
								td_right(a.cmp_interesting[0])+
								td_right(a.cmp_interesting[1])+
								td_right(a.cmp_interesting[2])+
								td_right(used.rookie_bonus)+
								td(join("&nbsp;",used.event_points_earned))+
								td_right(used.events_left)
							)+tag("tr class=\"hidden\" id=\""+name+"\"",
								tag("td colspan=\"100%\"",team_details(a))
							);
						},
						enumerate_from(1,reversed(sorted(
							in.result,
							[](auto x){ return make_tuple(x.dcmp_make,x.cmp_make,x); }
						)))
					)
				)
			)+
			h2("Extra data")+as_table(limits)+
			show_skill(in.skill)
			/*+data_used_table*/
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
