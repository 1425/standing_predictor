#include "venue.h"
#include<cstddef>
#include "address.h"
#include "print_r.h"
#include "map.h"
#include "array.h"
#include "vector_void.h"
#include "../tba/tba.h"
#include "tba.h"
#include "int_limited.h"
#include "map_auto.h"

using namespace std;
using Year=tba::Year;

template<typename T>
auto join(char a,std::vector<T> const& b){
	return join(as_string(a),b);
}

//This attempts to look at the sizes of events that have occurred at a specific venue
//to try to suggest whether there might be more slots available
//1) try to identify the venue for each event
//2) create a distribution of the number of teams that have been there
//3) for each event that's scheduled but not done yet, say how many extra slots there are expected to be left.

size_t event_size(TBA_fetcher &f,tba::Event const& e){
	//just asking for the keys to cut down on parsing time.
	return tba::event_teams_keys(f,e.key).size();
	//return tba::event_teams(f,e.key).size();
}

size_t event_size(TBA_fetcher &f,tba::Event_key const& e){
	return tba::event_teams_keys(f,e).size();
}

Address venue(tba::Event const&);

#define STRING_ITEM(NAME)\
	struct NAME{\
		std::string data;\
		auto operator<=>(NAME const&)const=default;\
	};\
	std::ostream& operator<<(std::ostream& o,NAME const& a){\
		return o<<""#NAME<<"("<<a.data<<")";\
	}

STRING_ITEM(Regional_name)
STRING_ITEM(District_championship_name)
STRING_ITEM(Fake)
STRING_ITEM(Offseason_name)
STRING_ITEM(Division_name)
//STRING_ITEM(District_event_name)

struct District_event_name{
	std::string district;
	std::string event;
	std::vector<string> rest;

	auto operator<=>(District_event_name const&)const=default;
};

std::ostream& operator<<(std::ostream& o,District_event_name const& a){
	return o<<"District_event_name("<<a.district<<":"<<a.event<<":"<<a.rest<<")";
}

using Event_name=std::variant<
	Regional_name,
	District_championship_name,
	District_event_name,
	Fake,
	Offseason_name,
	Division_name
>;

bool district_name(string s){
	if(s=="Detroit FIRST Robotics"){
		return 0;
	}
	if(s=="Groton"){
		return 0;
	}
	if(
		s=="NE" 
		|| s=="IN" || s=="FIM" || s=="MAR" || s=="PNW"
		|| s=="PCH" || s=="CHS" || s=="NC" || s=="ISR"
		|| s=="ONT" || s=="FNC" || s=="FMA" || s=="FIT"
		|| s=="FIN" || s=="FSC" || s=="CA" || s=="FCH"
		|| s=="WIN"
	){
		return 1;
	}
	if(s.size()>5){
		return 0;
	}
	PRINT(s);
	nyi
}

bool contains(string whole,string part){
	return !!strstr(whole.c_str(),part.c_str());
}

Event_name parse_event_name(string a){
	/* Warning: This is not 100% accurate, nor is it intended to be.  
	 * For example, anything that it doesn't understand will basically get classified as an offseason.
	 *
	 * */
	a=strip(a);
	auto sp=split(a);
	assert(!sp.empty());

	//strip out the markings of events being cancelled due to covid.
	//could add a flag to note when this happened, but not immediately relavent
	if(sp[0]=="*"){
		sp=skip(1,sp);
	}

	assert(!sp.empty());

	if(sp[sp.size()-1]=="(Cancelled)"){
		sp=take(sp.size()-1,sp);
	}

	{
		//at some point would be interesting to look at who the sponsors are
		//but not immediately relavent so ignore for now.
		vector<string> rest;
		for(auto x:sp){
			if(
				x!="presented" && x!="Presented" && 
				x!="sponsored" && x!="Sponsored" && 
				x!="co-sponsored"
			){
				rest|=x;
			}else{
				break;
			}
		}
		sp=rest;
	}


	if(
		contains(a,"Season Participation for") ||
		contains(a,"District Awards Event")
	){
		return Fake(a);
	}

	if(sp[sp.size()-1]=="Regional"){
		return Regional_name(
			join(' ',take(sp.size()-1,sp))
		);
	}
	if(contains(sp,"Championship")){
		return District_championship_name(join(' ',sp));
	}

	{
		string su=" FIRST Robotics District Competition";
	//if(contains(a,"District Competition")){
		if(suffix(a,su)){
			//PRINT(a);
			assert(suffix(a,su));
			//TODO: These are all old MI events
			/*PRINT(a);
			PRINT(sp);
			nyi*/
			return District_event_name("",a.substr(0,a.size()-su.size()),{});
		}
	}

	if(suffix(a,"District Event")){
		//TODO: old NE events here
		return District_event_name("",a,{});
	}
	if(contains(sp,"District")){
		string prefix;
		if(sp.size()>=3 && sp[0]=="MAR" && sp[1]=="FIRST" && sp[2]=="Robotics"){
			prefix="MAR";
			sp=skip(3,sp);
		}else{
			std::vector<string> pre;
			while(sp[0]!="District"){
				pre|=sp[0];
				sp=skip(1,sp);
			}
			prefix=join(' ',pre);
		}
		//PRINT(a);
		if(!district_name(prefix)){
			PRINT(a);
			PRINT(prefix);
			nyi//return a;
		}
		sp=skip(1,sp);

		if(!sp.empty() && sp[0]=="-"){
			sp=skip(1,sp);
		}

		vector<string> event_name;
		while(!sp.empty() && sp[0]!="Event"){
			event_name|=sp[0];
			sp=skip(1,sp);
		}
		//assert(!event_name.empty());

		if(!sp.empty() && sp[0]=="Event"){
			sp=skip(1,sp);
		}

		if(event_name.empty()){
			event_name=sp;
			sp.clear();
		}

		return District_event_name(prefix,join(" ",event_name),sp);
	}
	
	if(contains(sp,"Division")){
		//PRINT(a);
		//assert(sp.size()==2);
		sp=take(sp.size()-1,sp);
		return Division_name(join(" ",sp));
	}

	return Offseason_name(join(" ",sp));
}

bool match(Address const& a,Address const& b){
	if(a.country!=b.country) return 0;
	//return 1;
	if(a.state && b.state){
		if(a.state!=b.state){
			return 0;
		}
	}
	if(a.city && b.city){
		return a.city==b.city;
	}
	return 1;
}

bool match(std::optional<Address> const& a,std::optional<Address> const& b){
	assert(a);
	assert(b);
	return match(*a,*b);
}

bool match(tba::Event_key const& a,tba::Event_key const& b){
	//it is known that these strings must be length >4.
	const char *s1=a.get().c_str()+4;
	const char *s2=b.get().c_str()+4;
	return strcmp(s1,s2)==0;
	/*auto p1=a.get().substr(4,100);
	auto p2=b.get().substr(4,100);
	return p1==p2;*/
}

auto const& base(Regional_name const& a){
	return a.data;
}

auto const& base(District_championship_name const& a){
	return a.data;
}

auto const& base(District_event_name const& a){
	return a.event;
}

auto const& base(Fake const& a){
	return a.data;
}

auto const& base(Offseason_name const& a){
	return a.data;
}

auto const& base(Division_name const& a){
	return a.data;
}

template<typename ...T>
auto const& base(std::variant<T...> const& a){
	return std::visit([](auto const& x)->string const&{ return base(x); },a);
}

bool match(Event_name const& a,Event_name const& b){
	if(a==b) return 1;
	return base(a)==base(b);
}

template<typename T>
bool match(std::pair<T,T> const& a){
	return match(a.first,a.second);
}

template<typename Func,typename A,typename B,typename C>
auto zipWith(Func f,std::tuple<A,B,C> const& a,std::tuple<A,B,C> const& b){
	return std::make_tuple(
		#define X(N) f(get<N>(a),get<N>(b))
		X(0),X(1),X(2)
		#undef X
	);
}

#define ZIPWITH(F,A,B) zipWith([&](auto const& xa,auto const& xb){ return (F)(xa,xb); },(A),(B))

template<typename Func,typename T>
auto argmax_result(Func f,std::vector<T> const& v){
	assert(!v.empty());
	auto it=v.begin();

	using E=decltype(f(v[0]));
	std::pair<T,E> r{
		*it,
		f(*it)
	};

	for(;it!=v.end();++it){
		auto result=f(*it);
		if(result>r.second){
			r.second=result;
			r.first=*it;
		}
	}

	return r;
}

template<typename Apply,typename Reduce,typename T>
auto map_reduce(Apply apply,Reduce reduce,std::vector<T> const& a){
	//this is very similar to std::transform_reduce
	//but the standard library lacks a version in which an initial value is not required.
	assert(!a.empty());
	auto it=a.begin();
	auto r=apply(*it);
	++it;
	for(;it!=a.end();++it){
		r=reduce(r,apply(*it));
	}
	return r;
}

template<typename T>
T ident(T a){
	return a;
}

class Event_grouping{
	using T=tuple<tba::Event_key,std::optional<Address>,Event_name>;
	using Collection=vector<T>;//whether this is set or vector makes no meaningful difference.
	vector<Collection> groups;

	static T to_info(tba::Event const& event){
		return T(event.key,address(event),parse_event_name(event.name));
	}

	void add(std::vector<tba::Event> const& events_in){
		auto match_degree=[](T const& a,T const& b){
			return sum(ZIPWITH(match,a,b));
		};

		for(auto const& event:events_in){
			auto info=to_info(event);

			if(groups.empty()){
				groups|=Collection{info};
				continue;
			}

			auto [arg,value]=argmax_result(
				[&](auto i){
					return map_reduce(
						[&](auto const& x){ return match_degree(x,info); },
						[](auto a,auto b){ return std::max(a,b); },
						groups[i]
					);
				},
				range(groups.size())
			);

			if(value<2){
				groups|=Collection{info};
				continue;
			}
			groups[arg]|=info;
			continue;
		}
	}

	public:
	explicit Event_grouping(std::vector<tba::Event> const& a){
		add(a);
	}

	std::vector<tba::Event_key> find(tba::Event_key const& a)const{
		for(auto const& c:groups){
			for(auto const& e:c){
				if(get<0>(e)==a){
					return mapf([](auto x){ return get<0>(x); },c);
				}
			}
		}
		throw std::invalid_argument("unknown event");
	}

	std::vector<tba::Event_key> find(tba::Event const& a)const{
		return find(a.key);
	}
};

using Event_key=tba::Event_key;
using Team=tba::Team_key;
using Event=tba::Event_key;

template<typename K,typename V>
std::map<K,V> operator-(std::map<K,V> a,std::vector<K> const& b){
	for(auto const& k:b){
		a.erase(k);
	}
	return a;
}

template<typename T>
auto self_cross_no_dup(std::vector<T> a){
	std::vector<std::pair<T,T>> r;
	for(auto it=a.begin();it!=a.end();++it){
		for(auto i2=it+1;i2!=a.end();++i2){
			r|=make_pair(*it,*i2);
		}
	}
	return r;
}

void typical_teams(TBA_fetcher &f,Event_grouping const& grouping,tba::Event const& event){
	auto found=grouping.find(event);
	//calculate likelyhood for each team based on history
	auto ms=to_multiset(flatten(mapf([&](auto x){ return event_teams_keys(f,x); },found)));
	auto odds=dict(mapf(
		[&](auto x){
			assert(found.size());
			return make_pair(x,float(ms.count(x))/found.size());
		},
		to_set(ms)
	));

	//give the most likely teams
	{
		auto top=reversed(sorted(reverse_pairs(odds)));
		cout<<"\tMost likely: "<<take(5,top)<<"\n";
	}

	//calculate the 5 top teams that are expected but not present
	auto here=event_teams_keys(f,event.key);
	{
		auto not_here=odds-here;
		auto top=reversed(sorted(reverse_pairs(not_here)));
		cout<<"\tMissing: "<<take(5,top)<<"\n";
	}

	//calculate the 5 top teams that are present but not expected
	auto odds_here=sorted(mapf(
		[&](auto x){
			return make_pair(odds[x],x);
		},
		here
	));
	cout<<"\tUnexpected:"<<take(5,odds_here)<<"\n";

}

void typical_teams(TBA_fetcher &f,Year year){
	Event_grouping grouping(events(f));
	for(auto event:events(f,year)){
		cout<<event.key<<"\n";
		typical_teams(f,grouping,event);
	}

	//Event_grouping grouping(events(f));
	//for each team, list the events this year that they would be most expected to be at
	map<Event_key,map<Team,Pr>> by_event;;

	map<Team,map<Event,Pr>> by_team;
	for(auto [event,v]:by_event){
		for(auto [team,pr]:v){
			by_team[team][event]=pr;
		}
	}

	for(auto [team,v]:by_team){
		cout<<team<<"\t";
		cout<<reversed(sorted(reverse_pairs(v)))<<"\n";
		//and how that compares to which events they are attending
		nyi/*auto events=to_set(tba::team_events_year_keys(f,team));
		auto m=mapf(
			[&](auto x){
				auto [event,p]=x;
				return make_pair(event,events.count(event)-p);
			},
			v
		);
		cout<<"\t"<<m<<"\n";*/
	}

	//would also be intersting to calculate which events overlap the most with each other.
	//to start with, could just look at the events that are occurring this year
	//could also use a set of projected teams for district events, etc. that don't have a team list yet.
	
	//This is going to be crazy-slow to start with because it's going to be doing something like N^2*log(N) string comparisons
	auto events_to_compare=events_keys(f,year);
	map<Event,std::set<Team>> teams_by_event;
	for(auto event:events_to_compare){
		teams_by_event[event]=to_set(event_teams_keys(f,event));
	}

	//similarity metric: # of common teams/total # of teams between the events	
	auto similarity=reversed(sorted(mapf(
		[&](auto x){
			auto a=teams_by_event[x.first];
			auto b=teams_by_event[x.second];
			auto both=(a&b).size();
			auto either=(a|b).size();
			double value;
			if(either==0){
				value=0;
			}else{
				value=float(both)/either;
			}
			return make_tuple(value,x);
		},
		self_cross_no_dup(events_to_compare)
	)));

	cout<<"Events with most similar composition:\n";
	print_lines(take(500,similarity));
}

template<typename T>
double entropy(std::multiset<T> const& a){
	auto c=values(count(a));
	double r=0;
	for(auto v:c){
		Pr p=double(v)/a.size();
		r+=entropy(p);
	}
	return r;
}

template<typename T>
double entropy(std::vector<T> const& a){
	return entropy(to_multiset(a));
}

void most_international(TBA_fetcher &f){
	Year year(2026);
	auto m=reversed(sorted(mapf(
		[&](auto x){
			auto c=mapf(
				[](auto x)->optional<Country>{
					auto a=address(x);
					if(a){
						return a->country;
					}
					return std::nullopt;
				},
				tba::event_teams(f,x)
			);
			return make_pair(entropy(c),x);
		},
		//events_keys(f,year)
		keys(events(f))
	)));
	print_lines(take(50,m));
}

int venue_demo(TBA_fetcher &f){
	if(1){
		most_international(f);
		return 0;
	}
	typical_teams(f,Year(2026));
	return 0;

	/*for(auto event:events(f)){
		parse_event_name(event.name);
		//address(event);
	}*/

	auto g=GROUP(address,events(f));
	using Venue=Address;
	map<Venue,vector<size_t>> by_venue;

	/*TODO: Make it so that skip events with 0 teams listed
	 * list the place by name of event
	 * see if events with the same name happen in different places
	 * */

	for(auto [k,v]:g){
		assert(k);
		//PRINT(k);
		//PRINT(v.size());
		//print_r(v);
		auto m=mapf([&](auto x){ return event_size(f,x); },v);
		auto m2=FILTER(ident,m);
		if(m2.empty()){
			//cout<<"No data:"<<k<<"\n";
			continue;
		}
		assert(!m2.empty());
		//cout<<"\t"<<m.size()<<"\t";
		//PRINT(quartiles(m));
		//PRINT(sorted(m));
		by_venue[*k]=m2;
		/*for(auto x:v){
			cout<<"\t\t"<<x.key<<"\t"<<parse_event_name(x.name)<<"\t"<<event_size(f,x)<<"\n";
		}*/
	}

	Event_grouping grouping(events(f));

	//Try to calculate which upcoming events might have openings.
	for(auto event:events(f,Year(2026))){
		/*auto v=address(event);
		assert(v);
		auto f=by_venue.find(*v);
		if(f==by_venue.end()){
			cout<<event.key<<"\tnew\n";
		}else{
			cout<<event.key<<" "<<quartiles(f->second)<<"\n";
		}*/
		auto g=grouping.find(event.key);
		//PRINT(event.key);
		auto sizes=mapf([&](auto x){ return event_size(f,x); },g);
		sizes=FILTER(ident,sizes);
		if(sizes.empty()){
			cout<<event.key<<"\tno data\n";
		}else{
			//PRINT(count(sizes));
			//PRINT(quartiles(sizes));
			int med=median(sizes);
			int current=event_size(f,event);
			int mx=max(sizes);
			cout<<event.key<<"\t"<<current<<"\t"<<med<<"\t"<<current-med;
			cout<<"\t"<<mx<<"\t"<<current-mx<<"\n";
		}
	}
	return 0;
}

std::string nice_name(TBA_fetcher &f,tba::Event_key const& a){
	auto event=tba::event(f,a);
	auto p=parse_event_name(event.name);
	return base(p);
}

std::string nice_name(tba::Event const& event){
	return base(parse_event_name(event.name));
}
