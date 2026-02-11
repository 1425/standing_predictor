#ifndef PRINT_R_H
#define PRINT_R_H

#include<set>
#include "map.h"

namespace frc_api{
	struct Match;
	struct TeamListings;
	struct Event;
}

namespace tba{
	struct Match;
	struct Team;
	struct Match_Score_Breakdown_2024_Alliance;
};

void print_r(int,frc_api::Match const&);
void print_r(int,frc_api::TeamListings const&);
void print_r(int,frc_api::Event const&);
void print_r(int,tba::Match const&);
void print_r(int,tba::Team const&);
void print_r(int,tba::Match_Score_Breakdown_2024_Alliance const&);

#define BREAKDOWN(YEAR)\
	namespace tba{ struct Match_Score_Breakdown_##YEAR##_Alliance; }\
	void print_r(int,tba::Match_Score_Breakdown_##YEAR##_Alliance const&);
BREAKDOWN(2023)
BREAKDOWN(2022)
BREAKDOWN(2020)
BREAKDOWN(2017)
BREAKDOWN(2016)
BREAKDOWN(2015)
//BREAKDOWN(2014)

template<typename T>
void print_r(int,std::vector<T> const&);

int terminal_width();
std::string abbreviate(int max_width,std::string const&);

template<typename T>
void print_r(int n,T const& t){
	indent(n);
	std::cout<<abbreviate(terminal_width()-8*n,::as_string(t))<<"\n";
}

template<typename A,typename B>
void print_r(int n,std::pair<A,B> const& a){
	indent(n++);
	std::cout<<"pair\n";
	print_r(n,a.first);
	print_r(n,a.second);
}

template<typename T>
void print_r(int n,std::vector<T> const& v){
	indent(n);
	std::cout<<"vector\n";
	for(auto x:v){
		print_r(n+1,x);
	}
}

template<typename T>
void print_r(int n,std::optional<T> const& v){
	indent(n);
	std::cout<<"optional\n";
	if(v) print_r(n+1,*v);
}

template<typename K,typename V>
void print_r(int n,std::map<K,V> const& v){
	indent(n);
	std::cout<<"map\n";
	n++;
	for(auto x:v) print_r(n,x);
}

template<typename T>
void print_r(int n,std::set<T> const& a){
	indent(n);
	std::cout<<"set\n";
	for(auto x:a){
		print_r(n+1,x);
	}
}

template<typename T>
void print_r(T const& t){
	return print_r(0,t);
}

#endif
