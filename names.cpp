#include "names.h"
#include<set>
#include<iostream>
#include<string>
#include<cassert>
#include "../tba/data.h"
#include "../tba/tba.h"
#include "io.h"
#include "util.h"
#include "set.h"
#include "tba.h"
#include "skill_opr.h"
#include "dates.h"
#include "declines.h"

using namespace std;
using Team_key=tba::Team_key;
using Year=tba::Year;
using District_key=tba::District_key;

std::ostream& operator<<(std::ostream& o,Team_name_contents const& a){
	o<<"Team_name_contents(";
	o<<a.sponsors;
	o<<a.orgs;
	return o<<")";
}

Team_name_contents parse_name(std::string const& name){
	//Note that this is very approximate.
	//because the delimiters are chars that can appear in the names of
	//financial sponsors and organizations.

	auto sp=split(name,'/');
	assert(sp.size()>=1);

	auto sponsors=take(sp.size()-1,sp);

	auto sp2=split(last(sp),'&');
	assert(sp2.size()>=1);

	vector<string> orgs;
	if(sp2.size()==1){
		orgs=sp2;
	}else{
		sponsors|=sp2[0];
		orgs=skip(1,sp2);
	}

	return Team_name_contents(to_set(sponsors),to_set(orgs));
}

auto parse_name(optional<string> const& a){
	assert(a);
	return parse_name(*a);
}

Team_name_contents parse_name(tba::Team const& t){
	return parse_name(t.name);
}

void check_sponsors(TBA_fetcher& f){
	//This is useless because it just gives all the teams all the time
	//with the same info.
	map<Team_key,std::map<Year,Team_name_contents>> r;
	for(auto year:years()){
		auto teams=teams_year_all(f,Year(2025));
		for(auto const& team:teams){
			r[team.key][year]=parse_name(team.name);
		}
	}
	for(auto [team,info]:r){
		PRINT(team);
		PRINT(info.size());
		for(auto [a,b]:adjacent_pairs(info)){
			if(a.second!=b.second){
				diff(a.second,b.second);
			}
		}

		//print_lines(info);
	}
}

std::vector<tba::Team> all_teams(TBA_fetcher &f){
	//just asking for one year because it actually doesn't give different results for different years.
	return teams_year_all(f,Year(2026));
}


void common_sponsors(TBA_fetcher &f){
	//Figure out what the most common sponsors are for teams.
	std::multiset<string> sponsors,orgs;
	std::multiset<int> num_sponsors,num_orgs;

	for(auto team:all_teams(f)){
		auto p=parse_name(team);
		sponsors|=p.sponsors;
		orgs|=p.orgs;

		num_sponsors|=p.sponsors.size();
		num_orgs|=p.orgs.size();
	}

	PRINT(quartiles(num_sponsors));
	PRINT(quartiles(num_orgs));

	auto show=[](string label,auto x){
		auto c=reversed(sorted(swap_pairs(count(x))));
		cout<<label<<"\n";
		for(auto x:take(20,c)){
			cout<<"\t"<<x<<"\n";
		}
	};
	show("sponsors",sponsors);
	show("orgs",orgs);
}

std::optional<std::tuple<string,string,string>> parse_district_event_name(std::string const& s){
	auto sp=split(s);
	auto district_abbrev=sp[0];
	if(sp[1]!="District"){
		return std::nullopt;
	}
	vector<string> name;
	size_t i=2;
	while(i<sp.size() && sp[i]!="Event"){
		name|=sp[i];
		i++;
	}
	assert(i<sp.size());
	i++;
	vector<string> sponsor;
	if(i<sp.size()){
		assert(sp[i]=="presented");
		i++;
		assert(sp[i]=="by");
		i++;
		while(i<sp.size()){
			sponsor|=sp[i];
			i++;
		}
	}
	return std::make_tuple(district_abbrev,join(" ",name),join(" ",sponsor));
}

auto abbrev(District_key a){
	return a.get().substr(4,100);
}

auto display_name(TBA_fetcher& f,District_key district){
	auto a=abbrev(district);
	auto x=tba::history(f,a);
	auto found=filter([=](auto x){ return x.year==year(district); },x);
	assert(found.size()==1);
	auto f1=found[0];
	//PRINT(f1);
	return f1.display_name;
}

auto district_display_names(TBA_fetcher &f){
	//could have this go through all the years
	auto d=tba::districts(f,Year(2026));
	return mapf([](auto x){ return x.display_name; },d);
}

std::optional<tuple<string,string,string>> parse_dcmp_name(TBA_fetcher &f,std::string const& s){
	PRINT(s);
	auto n=district_display_names(f);
	auto found=filter([=](auto x){ return prefix(s,x); },n);
	switch(found.size()){
		case 0:
			return std::nullopt;
		case 1:{
			auto d=found[0];
			PRINT(d);
			auto sp=split(s.substr(d.size(),s.size()));
			PRINT(sp);
			if(sp[0]=="FIRST"){
				sp=skip(1,sp);
			}
			vector<string> name;
			size_t i=0;
			while(i<sp.size() && sp[i]!="presented"){
				name|=sp[i];
				i++;
			}
			vector<string> sponsor;
			if(i<sp.size()){
				i++;
				assert(sp[i]=="by");
				i++;
				while(i<sp.size()){
					sponsor|=sp[i++];
				}
			}
			//can also parse an optional "presented by" at the end
			//return s.substr(d.size(),s.size());
			return make_tuple(d,join(" ",name),join(" ",sponsor));
		}
		default:
			print_lines(found);
			assert(0);
	}
}

std::string parse_event_name(TBA_fetcher &f,std::string const& s){
	{
		auto x=parse_district_event_name(s);
		if(x){
			return get<1>(*x);
		}
	}

	{
		auto x=parse_dcmp_name(f,s);
		if(x){
			return get<1>(*x);
		}
	}

	if(s=="FIRST Championship - FIRST Robotics Competition"){
		return "Worlds";
	}
	return s;

	//return s;

	//"<> District ... Event [presented by ...]";
	auto sp=split(s);
	auto district_abbrev=sp[0];
	//sp[1]=="District"
}


