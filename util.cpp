#include "util.h"
#include<fstream>
#include<filesystem>

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

