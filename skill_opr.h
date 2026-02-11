#ifndef SKILL_OPR_H
#define SKILL_OPR_H

#include "set.h"
#include "skill.h"

Skill_estimates calc_skill_opr(TBA_fetcher&,tba::District_key const&);

std::vector<tba::Year> years();
std::map<tba::District_abbreviation,std::set<tba::Year>> normal_district_years(TBA_fetcher&);

void check_dist(Team_dist const&);

template<typename T>
auto to_dist(std::multiset<T> const& a){
	assert(!a.empty());
	flat_map2<T,Pr> r;
	for(auto k:to_set(a)){
		r[k]=(0.0+a.count(k))/a.size();
	}
	check_dist(r);
	return r;
}

template<typename K,typename V>
V get(flat_map2<K,V> const& a,auto const& k,auto const& otherwise){
	auto f=a.find(k);
	if(f==a.end()){
		return otherwise;
	}
	return (*f).second;
}

template<typename K,typename V>
V get(flat_map2<K,V> const& a,auto const& k){
	auto f=a.find(k);
	assert(f!=a.end());
	return (*f).second;
}

template<typename K,typename V>
V get(std::map<K,V> const& a,auto const& k){
	auto f=a.find(k);
	assert(f!=a.end());
	return f->second;
}

template<typename K,typename V>
V get(std::map<K,V> const& a,auto const& k,auto const& otherwise){
	auto f=a.find(k);
	if(f==a.end()){
		return otherwise;
	}
	return f->second;
}

#endif
