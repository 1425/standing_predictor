#ifndef PRINT_R_H
#define PRINT_R_H

#include<map>
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

template<typename K,typename V>
void print_r(int n,std::map<K,V> const& v){
	indent(n);
	std::cout<<"map\n";
	n++;
	for(auto x:v) print_r(n,x);
}

template<typename T>
void print_r(T const& t){
	return print_r(0,t);
}

#endif
