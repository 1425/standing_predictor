#ifndef PRINT_R_H
#define PRINT_R_H

#include "set.h"
#include "map.h"
#include "util.h"

namespace frc_api{
	struct Match;
	struct TeamListings;
	struct Event;
}

void print_r(int,frc_api::Match const&);
void print_r(int,frc_api::TeamListings const&);
void print_r(int,frc_api::Event const&);

template<typename T>
void print_r(int,std::vector<T> const&);

template<typename T>
void print_r(int n,T const& t){
	indent(n);
	std::cout<<t<<"\n";
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
