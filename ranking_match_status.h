#ifndef RANKING_MATCH_STATUS_H
#define RANKING_MATCH_STATUS_H

#include "set_limited.h"
#include "rp.h"
#include "map_auto.h"
#include "tba.h"
#include "../tba/tba.h"
#include "pick_points.h"

template<size_t N>
set_limited<tba::Team_key,N> teams(set_limited<tba::Team_key,N> const& a){
	return a;
}

template<typename Team>
using Alliance=set_limited<Team,3>;

template<typename Team>
class Match{
	using Data=std::array<Alliance<Team>,2>;
	Data data;

	public:

	Match(Data a):data(std::move(a)){
		assert( (data[0]&data[1]).empty());
	}

	auto const& get()const{
		return data;
	}

	using const_iterator=Data::const_iterator;

	const_iterator begin()const{
		return data.begin();
	}

	const_iterator end()const{
		return data.end();
	}

	Alliance<Team> const& operator[](size_t i){
		assert(i<2);
		return data[i];
	}

	auto operator<=>(Match const&)const=default;
};

template<typename Team>
std::ostream& operator<<(std::ostream& o,Match<Team> const& a){
	return o<<a.get();
}

template<typename Team>
Match<Team> rand(Match<Team> const*);

template<typename Team,typename T,size_t N>
auto zip(Match<Team> const& a,std::array<T,N> const& b){
	return zip(a.get(),b);
}

template<typename Team>
auto enumerate_from(size_t i,Match<Team> const& a){
	return enumerate_from(i,a.get());
}

template<typename Func,typename Team>
auto mapf(Func f,Match<Team> const& a){
	return mapf(f,a.get());
}

template<typename Team>
//std::set<Team> teams(Match<Team> const& a){
set_limited<Team,6> teams(Match<Team> const& a){
	return to_set(teams(a.get()));
}

template<typename Team>
using Schedule=std::vector<Match<Team>>;

template<typename Team>
using Standings=map_auto<Team,RP>;

template<typename Team>
auto teams(Standings<Team> const& a){
	return keys(a);
}

//year is relavent because it changes the ranking rules.
#define RANKING_MATCH_STATUS(X)\
	X(Standings<Team>,standings)\
	X(Schedule<Team>,schedule)\
	X(tba::Year,year)\
	X(int,matches_completed)\

#define REMOVE_FIRST(...) REMOVE_FIRST_SUB(__VA_ARGS__)
#define REMOVE_FIRST_SUB(X, ...) __VA_ARGS__

template<typename Team>
struct Ranking_match_status{
	RANKING_MATCH_STATUS(INST)

	explicit Ranking_match_status(tba::Year a):year(a){}

	Ranking_match_status(Ranking_match_status const&)=default;

	Ranking_match_status(
		#define X(A,B) ,A B##1
		REMOVE_FIRST(RANKING_MATCH_STATUS(X))
		#undef X
	):
		#define X(A,B) ,B(std::move(B##1))
		REMOVE_FIRST(RANKING_MATCH_STATUS(X))
		#undef X
	{}

	void fill_standings(TBA_fetcher &f,tba::Event_key event){
		for(auto const& team:
			teams(schedule)|teams_keys(f,event)|teams(event_alliances(f,event))
		){
			auto f=standings.find(team);
			if(f==standings.end()){
				standings[team]=0;
			}
		}
	}

	auto operator<=>(Ranking_match_status const&)const=default;
};

template<typename Team>
std::ostream& operator<<(std::ostream& o,Ranking_match_status<Team> const& a){
	o<<"Ranking_match_status( ";
	#define X(A,B) o<<""#B<<":"<<a.B<<" ";
	RANKING_MATCH_STATUS(X)
	#undef X
	return o<<")";
}

template<typename Team>
void print_r(int n,Ranking_match_status<Team> const& a){
	indent(n);
	std::cout<<"Ranking_match_status\n";
	n++;
	#define X(A,B) indent(n); std::cout<<""#B<<"\n"; print_r(n+1,a.B);
	RANKING_MATCH_STATUS(X)
	#undef X
}

template<typename Team>
std::set<Team> teams(Ranking_match_status<Team> const& a){
	auto t1=teams(a.standings);
	auto t2=teams(a.schedule);
	assert( (t2-t1).empty());
	return t1;
}

Ranking_match_status<tba::Team_key> ranking_match_status(TBA_fetcher&,tba::Event_key const&);

#endif
