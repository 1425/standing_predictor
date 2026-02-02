#include "io.h"
#include "util.h"
#include "../tba/data.h"

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


std::ostream& operator<<(std::ostream& o,std::chrono::time_zone const& a){
	o<<"timezone(";
	o<<a.name();//<<" "<<a.get_info();
	return o<<")";
}

std::ostream& operator<<(std::ostream& o,std::chrono::time_zone const * const x){
	if(!x){
		return o<<"NULL";
	}
	return o<<*x;
}

std::string link(tba::Event_key const& event,std::string const& body){
	return link("https://www.thebluealliance.com/event/"+event.get(),body);
}

std::string link(tba::Event const& event,std::string const& body){
	return link(event.key,body);
}

