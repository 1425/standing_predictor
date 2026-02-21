#include "vector.h"
#include<filesystem>
#include "set.h"
#include "io.h"

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

std::vector<char> to_vec(std::string const& s){
	std::vector<char> r;
	for(auto c:s) r|=c;
	return r;
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


