#include<iostream>
#include<vector>
#include<algorithm>
#include "z3++.h"
#include "util.h"

using namespace std;
using namespace z3;

template<typename T>
T sum(vector<T> a){
	T r{0};
	for(auto elem:a){
		r+=elem;
	}
	return r;
}

template<typename T>
vector<T> tail(vector<T> a){
	vector<T> r;
	for(auto i:range(size_t{1},a.size())){
		r|=a[i];
	}
	return r;
}

//start program-specific code

using Team=int;

unsigned qual_pts(int event_size,int rank){
	//TODO: FIXME
	assert(rank>0);
	assert(rank<=event_size);
	assert(event_size>0);
	return event_size-rank;
}

void add_event(string event_code,vector<Team> teams,context &c,solver &s){
	auto prefix=event_code+"_";

	//qual
	auto team_ranks=mapf(
		[&](auto team){
			auto name=prefix+"rank_"+as_string(team);
			auto t=c.int_const(name.c_str());
			s.add(t>=1 && t<=int(teams.size()));
			return make_pair(team,t);
		},
		teams
	);
	auto declare_not_equal=[&](auto a){
		/*for(auto i:range(a.size())){
			for(auto j:range(i)){
				s.add(a[i]!=a[j]);
			}
		}*/
		//distinct(a);
		expr_vector t(c);
		for(auto &elem:a){
			t.push_back(elem);
		}
		s.add(distinct(t));
	};
	declare_not_equal(seconds(team_ranks));

	auto ranks=range(size_t(1),1+teams.size());
	mapf(
		[&](auto const& rank_var){
			//PRINT(rank_var);
			auto x=c.int_const( (prefix+"qual_pts_"+as_string(rank_var.first)).c_str() );
			for(auto rank:ranks){
				s.add(x==int(qual_pts(teams.size(),rank)) || rank_var.second!=int(rank));
			}
			return 0;
		},
		team_ranks
	);

	auto decl_team_number=[&](string name){
		auto x=c.int_const(name.c_str());
		auto v=(x==teams[0]);
		for(auto a:tail(teams)){
			//v|=(x==a);
			v= v || (x==a);
		}
		s.add(v);
		return x;
	};

	auto decl_maybe_team_number=[&](string name){
		auto x=c.int_const(name.c_str());
		auto v=(x==teams[0]);
		for(auto a:tail(teams)){
			//v|=(x==a);
			v= v || (x==a);
		}
		//v= v||(x==0); //if the number is 0 then there was not a team there.
		v= v||0; //if the number is 0 then there was not a team there.
		s.add(v);
		return x;
	};

	//alliance selection
	vector<z3::expr> places;
	for(auto i:range(1,9)){
		places|=decl_team_number(prefix+"capt"+as_string(i));
		places|=decl_team_number(prefix+"a"+as_string(i)+"_p1");
		places|=decl_team_number(prefix+"a"+as_string(i)+"_p2");
	}
	declare_not_equal(places);

	vector<z3::expr> backup_slots;
	for(auto i:range(1,9)){	
		backup_slots|=decl_maybe_team_number(prefix+"a"+as_string(i)+"_backup");
	}
	for(auto i:range(backup_slots.size())){
		for(auto j:range(i+1,backup_slots.size())){
			s.add(backup_slots[i]==0 || backup_slots[i]!=backup_slots[j]);
		}
	}
	for(auto& elem:places){
		for(auto &x:backup_slots){
			s.add(elem!=x);
		}
	}

	//next, put in all the logic to make who can be captain correct
	//0r could just skip this and say that lots of captains could decide to take themselves out of the 
	//tournament.
	//captains need to be ranked lower than the captains above them.
	//and all captains need to be ranked higher than all non-included teams

	//all the places exist
	//teams are all different

	//elims
	/*
	tournament structure:
	1v8
	2v7
	3v6
	4v5

	1/8 v 4/5
	2/7 v 3/6
	
	1/4/5/8 v 2/3/6/7
	*/
	//q1 in r2/r3/b2/b3 (how many matches to win)

	//awards
	//total points
}

void demo(){
	/*for each of the events:
	 * add event
	 *for each of the events that has finished:
	 *   add details about how it went
	 *   including who's no longer eligible for chairmans
	 *after have all of the constraints, need to run it with the push/pop
	 logic that will ask each of the interesting questions
	 * */
	context c;
	solver s(c);
	add_event("2018wase",range(100,124),c,s);
	switch(s.check()){
		case unsat:
			cout<<"unsat\n";
			break;
		case sat:{
			cout<<"sat\n";
			model m=s.get_model();
			vector<pair<string,string>> data;
			for(unsigned i=0;i<m.size();i++){
				func_decl v=m[i];
				assert(v.arity()==0);//don't know why
				//cout<<v.name()<<" = "<<m.get_const_interp(v)<<"\n";
				data|=make_pair(as_string(v.name()),as_string(m.get_const_interp(v)));
			}
			print_lines(sorted(data));
			break;
		}
		case unknown:
			cout<<"Unknown\n";
			break;
		default:
			assert(0);
	}
}

int main(){
	demo();
}
