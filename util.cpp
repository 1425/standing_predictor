#include "util.h"
#include<fstream>
#include<filesystem>
#include "vector.h"

#ifdef __unix__
#include<cxxabi.h>
#endif

#include "set.h"
#include "io.h"
#include "vector_void.h"

std::string slurp(std::string const& filename){
	std::ifstream f(filename.c_str());
	if(!f.good()){
		throw "File_not_found(filename)";
	}
	std::stringstream ss;
	while(f>>ss.rdbuf());
	return ss.str();
}

std::string tolower(std::string const& s){
	std::stringstream ss;
	for(auto c:s){
		ss<<char(tolower(c));
	}
	return ss.str();
}

bool prefix(std::string const& whole,std::string const& p){
	return whole.substr(0,p.size())==p;
}

std::string as_pct(double d){
	std::stringstream ss;
	ss<<int(d*100)<<'%';
	return ss.str();
}

std::chrono::year_month_day& operator++(std::chrono::year_month_day& a){
	a=std::chrono::sys_days{a}+std::chrono::days{1};
	return a;
}

std::chrono::days operator-(std::chrono::year_month_day a,std::chrono::year_month_day b){
	return std::chrono::sys_days(a)-std::chrono::sys_days(b);
}

std::string demangle(const char *s){
	assert(s);
	#ifdef __unix__
	int status;
	char *ret=abi::__cxa_demangle(s,0,0,&status);
	assert(ret);
	return std::string(ret);
	#else
	return s;
	#endif
}

bool all_equal(std::pair<long int,bool> const& a){
	return a.first==(long int)a.second;
}

bool contains(std::vector<std::string> const& a,const char *s){
	if(!s){
		return 0;
	}
	return contains(a,std::string(s));
}

std::string strip(std::string const& a){
	auto s=a;
	while(!s.empty() && s[0]==' '){
		s=s.substr(1,s.size());
	}
	while(!s.empty() && s[s.size()-1]==' '){
		s=s.substr(0,s.size()-1);
	}
	return s;
}

bool prefix(std::optional<std::string> const& whole,std::string const& part){
	if(!whole) return 0;
	return prefix(*whole,part);
}

bool suffix(std::string const& whole,std::string const& b){
	if(b.size()>whole.size()){
		return 0;
	}
	auto sub=whole.substr(whole.size()-b.size(),whole.size());
	return sub==b;
}

int max(short a,int b){
	return std::max(int(a),b);
}

int min(int a,short b){
	return std::min(a,int(b));
}

short coerce(int a,short const*){
	assert(a>=std::numeric_limits<short>::min());
	assert(a<=std::numeric_limits<short>::max());
	return a;
}

