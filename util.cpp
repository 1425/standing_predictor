#include "util.h"
#include<fstream>
#include<filesystem>

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

double sum(std::vector<double> const& v){
	double r=0;
	for(auto elem:v){
		r+=elem;
	}
	return r;
}

double sum(std::vector<int> const& v){
	int x=0;
	for(auto elem:v){
		x+=elem;
	}
	return x;
}

size_t sum(std::vector<bool> const& a){
	size_t i=0;
	for(auto elem:a){
		if(elem){
			i++;
		}
	}
	return i;
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

std::vector<char> to_vec(std::string const& s){
	std::vector<char> r;
	for(auto c:s) r|=c;
	return r;
}

std::vector<std::string> find(std::string const& base,std::string const& name){
	//should do something similar to "find $BASE -name $NAME*"
	std::vector<std::string> r;
	for(auto x:std::filesystem::recursive_directory_iterator(base)){
		if(x.is_regular_file()){
			std::string s=as_string(x).c_str()+1;
			s=s.substr(0,s.size()-1);
			auto sp=split(s,'/');
			//PRINT(sp);
			//cout<<"\""<<sp[sp.size()-1]<<"\"\n";
			if(prefix(last(sp),name)){
				r|=s;
			}
		}
	}
	return r;
}

static auto consolidate_inner(std::vector<int> const& in){
	auto s=to_set(in);
	std::vector<std::pair<int,int>> v;
	if(s.empty()){
		return v;
	}
	int start=*begin(s);
	for(int i=start;i<=max(s);++i){
		if(!s.count(i)){
			v|=std::make_pair(start,i-1);
			do{
				i++;
			}while(i<=max(s) && s.count(i)==0);
			start=i;
		}
	}
	v|=std::make_pair(start,max(s));
	return v;
}

std::string consolidate(std::vector<int> const& in){
	auto v=consolidate_inner(in);
	std::stringstream ss;
	for(auto x:v){
		if(x.first==x.second){
			ss<<x.first;
		}else{
			ss<<x;
		}
		ss<<' ';
	}
	return ss.str();
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

