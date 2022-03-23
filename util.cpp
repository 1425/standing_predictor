#include "util.h"
#include<fstream>

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

std::string link(std::string const& url,std::string const& body){
	return tag("a href=\""+url+"\"",body);
}

std::string td1(std::string const& s){ return td(s); }
std::string th1(std::string const& s){ return th(s); }
