#include "io.h"
#include<iomanip>
#include "vector.h"
#include "../tba/data.h"
#include "query.h"

std::vector<std::string> split(std::string const& s){
	std::vector<std::string> r;
	std::stringstream ss;
	for(auto c:s){
		if(isblank(c)){
			if(ss.str().size()){
				r|=ss.str();
				ss.str("");
			}
		}else{
			ss<<c;
		}
	}
	if(ss.str().size()){
		r|=ss.str();
	}
	return r;
}

std::vector<std::string> split(std::string const& s,char delim){
	std::vector<std::string> r;
	std::stringstream ss;
	for(auto c:s){
		if(c==delim){
			if(ss.str().size()){
				r|=ss.str();
				ss.str("");
			}
		}else{
			ss<<c;
		}
	}
	if(ss.str().size()){
		r|=ss.str();
	}
	return r;
}

std::string link(std::string const& url,std::string const& body){
	return tag("a href=\""+url+"\"",body);
}

std::string html_link(const char *a,const char *b){
	return link(std::string(a),std::string(b));
}

std::string td1(std::string const& s){ return td(s); }
std::string th1(std::string const& s){ return th(s); }

void indent(int x){
	for(auto _:range(x)){
		(void)_;
		std::cout<<"\t";
	}
}

std::ostream& operator<<(std::ostream& o,std::invalid_argument const& a){
	return o<<"invalid_argument("<<a.what()<<")";
}

std::chrono::hours offset(std::chrono::time_zone const& a){
	//This returns floating-point hours.

	//This works by assuming that system_clock is synchronoized to UTC.
	//That is not technically required by the C++ standard.
	auto now=std::chrono::system_clock::now();
	auto info=a.get_info(now);
	auto x=std::chrono::duration<double,std::ratio<3600>>(info.offset);
	return std::chrono::hours((int)x.count());
}

std::ostream& operator<<(std::ostream& o,std::chrono::hours a){
	return o<<a.count()<<"h";
}

std::ostream& operator<<(std::ostream& o,std::chrono::time_zone const& a){
	o<<"timezone(";
	o<<a.name()<<" "<<offset(a);
	return o<<")";
}

std::ostream& operator<<(std::ostream& o,std::chrono::time_zone const * const x){
	if(!x){
		return o<<"NULL";
	}
	return o<<*x;
}

std::ostream& operator<<(std::ostream& o,std::stringstream const& ss){
	return o<<ss.str();
}

std::ostream& operator<<(std::ostream& o,std::nullopt_t){
	return o<<"nullopt";
}

std::string round3(double d){
	std::stringstream ss;
	ss<<std::setprecision(3)<<std::fixed;
	ss<<d;
	return ss.str();
}

std::string toupper(tba::District_abbreviation const& a){
	return toupper(a.get());
}

std::string operator+(const char* a,tba::District_abbreviation const& b){
	return a+b.get();
}

std::string splat(tba::District_abbreviation const& district,tba::Team_key const& team){
	std::stringstream ss;
	ss<<"https://splatfrc.com/team.html?district="<<district<<"&team="<<team.raw();
	return link(ss.str(),"FRC Splat");
}

std::string splat(tba::District_abbreviation const& district){
	std::stringstream ss;
	ss<<"https://splatfrc.com/districts.html?district="<<district;
	return link(ss.str(),"FRC Splat");
}

std::string splat(tba::Event_key const& event){
	std::stringstream ss;
	ss<<"https://splatfrc.com/events.html?event="<<event;
	return link(ss.str(),"FRC Splat");
}

std::string splat(tba::Event const& event){
	return splat(event.key);
}

std::string statbotics(tba::Event const& e){
	return link(
		std::string("https://www.statbotics.io/event/"+::as_string(e.key))+"#figures",
		"Statbotics"
	);
}

std::string statbotics(tba::Team_key const& a,tba::Year const& year){
	std::stringstream u;
	u<<"https://www.statbotics.io/team/"<<a.raw()<<"/"<<year;
	return link(u.str(),"Statbotics");
}

std::string frc_events(tba::Event const& e){
	(void)e;
	std::stringstream ss;
	ss<<"https://frc-events.firstinspires.org/"<<year(e)<<"/"<<e.key.code();
	return link(ss.str(),"FRC events");
}

std::string frc_events(tba::Year const& year,tba::Team_key const& team){
	std::stringstream u;
	u<<"https://frc-events.firstinspires.org/"<<year<<"/team/"<<team.raw();
	return link(u.str(),"FRC Events");
}

std::string frc_events(tba::Year const& year,tba::District_abbreviation const& district_short){
	return link(
		"https://frc-events.firstinspires.org/"+::as_string(year)+"/district/"+toupper(district_short),
		"FRC Events"
	);
}

std::string frc_locks(tba::Team_key const& e){
	std::stringstream ss;
	ss<<"https://frclocks.com/teams/"<<e<<".html";
	return link(ss.str(),"FRC Locks");
}

std::string frc_locks(tba::District_abbreviation const& a){
	auto main=[=](){
		std::stringstream ss;
		ss<<"https://frclocks.com/districts/"<<a<<".html";
		return link(ss.str(),"FRC Locks");
	}();

	if(a=="ca"){
		std::stringstream ss;
		ss<<main<<"(";
		ss<<html_link("https://frclocks.com/districts/ca_north.html","North");
		ss<<" ";
		ss<<html_link("https://frclocks.com/districts/ca_south.html","Sorth");
		ss<<")";
		return ss.str();
	}else{
		return main;
	}
}

std::string make_link(tba::Team_key const& team){
	auto s=team.str();
	assert(s.substr(0,3)=="frc");
	auto t=s.substr(3,500);
	return link("https://www.thebluealliance.com/team/"+t,t);
}

std::string make_link(tba::Team_key const& team,tba::Year const& year){
	auto s=team.str();
	assert(s.substr(0,3)=="frc");
	auto t=s.substr(3,500);
	return link("https://www.thebluealliance.com/team/"+t+"/"+year,t);
}

std::string the_blue_alliance(tba::Team_key const& team,tba::Year const& year){
	return link("https://thebluealliance.com/team/"+team.raw()+"/"+::as_string(year),"The Blue Alliance");
}

std::string the_blue_alliance(tba::Year const& year,tba::District_abbreviation const& district_short){
	return link(
		"https://www.thebluealliance.com/events/"+district_short+"/"+::as_string(year)+"#rankings",
		"The Blue Alliance"
	);
}
