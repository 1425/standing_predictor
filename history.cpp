#include "history.h"
#include<vector>
#include<boost/tokenizer.hpp>
#include "../tba/data.h"
#include "tba.h"
#include "skill.h"
#include "event_partial.h"
#include "rand.h"
#include "declines.h"

using namespace std;

using Year=tba::Year;

std::set<tba::Date> meaningful_dates(TBA_fetcher &f,tba::District_key const& district){
	auto e=events(f,district);
	auto start=min(mapf([](auto x){ return *x.start_date; },e));
	auto ends=to_set(mapf([](auto x){ return *x.end_date; },e));
	return start|ends;
}

#define HISTORY_ITEM(X)\
	X(tba::Team_key,team)\
	X(Year,year)\
	X(tba::Date,date)\
	X(size_t,appearances)\
	X(Pr,dcmp_pr)\
	X(Pr,cmp_pr)

STRUCT_DECLARE(History_item,HISTORY_ITEM)
ELEMENTWISE_RAND(History_item,HISTORY_ITEM)
PRINT_STRUCT(History_item,HISTORY_ITEM)

void diff(int n,History_item const& a,History_item const& b){
	if(a!=b){
		indent(n);
		cout<<"History_item\n";
		n++;
		#define X(A,B) diff(n,a.B,b.B);
		HISTORY_ITEM(X)
		#undef X
	}
}

void write_file(std::string const& path,std::vector<History_item> const& data){
	ofstream f(path);

	#define X(A,B) f<<""#B<<",";
	HISTORY_ITEM(X)
	#undef X
	f<<"\n";

	for(auto const& x:data){
		#define X(A,B) f<<x.B<<",";
		HISTORY_ITEM(X)
		#undef X
		f<<"\n";
	}
}

tba::Team_key decode(std::string a,tba::Team_key const* b){
	return decode2(a,b);
}

tba::Date decode(std::string const& a,tba::Date const* b){
	return tba::decode(a,b);
}

size_t decode(std::string const& a,size_t const*){
	try{
		return stoi(a);
	}catch(...){
		assert(0);
	}
}

double decode(std::string const& a,double const*){
	return stod(a);
}

std::vector<History_item> read_file(std::string const& path){
	ifstream f(path);
	std::vector<History_item> r;

	{
		std::string header;
		getline(f,header);
	}

	std::string line;
	while(getline(f,line)){
		boost::tokenizer<boost::escaped_list_separator<char>> t(line);
		auto it=t.begin();
		r|=History_item{
			#define X(A,B) [&](){\
				cout<<"item\n";\
				assert(it!=t.end());\
				auto r=decode(*it,(A*)0);\
				++it;\
				return r;\
			}(),
			HISTORY_ITEM(X)
			#undef X
		};
	}
	return r;
}

void rw_demo(){
	for(auto _:range(100)){
		auto path="tmp.csv";
		auto x=rand((std::vector<History_item>*)0);
		print_lines(x);
		write_file(path,x);
		cout<<"written\n";
		auto x2=read_file(path);
		diff(x,x2);
		assert(x==x2);
	}
	cout<<"RW works.\n";
}

int history_demo(TBA_fetcher& f){
	/*try{
		rw_demo();
	}catch(std::invalid_argument const& a){
		cerr<<a<<"\n";
		return 1;
	}
	return 0;*/

	//this is going to take a while to run; might want to save the results somewhere.

	//dcmp odds, then cmp odds
	std::vector<History_item> v;

	for(auto district:districts(f)){
		if(year(district)<2014){
			//because do not have data for how the advancement to the championship was done.
			continue;
		}
		PRINT(district);
		auto dates=meaningful_dates(f,district);
		for(auto date:dates){
			cout<<"date:"<<date<<"\n";
			auto [run_input,skill,a,extra]=read_status(f,district,Skill_method::POINTS,date);

			std::multiset<tba::Team_key> appearances;
			for(auto [event,event_data]:a.local){
				appearances|=keys(event_data.by_team);
			}

			auto result=run_calc(run_input);
			for(auto const& x:result.result){
				//also want to know the number of events that this team has played at this point.
				v|=History_item(
					x.team,
					year(district),
					date,
					appearances.count(x.team),
					x.dcmp_make,
					x.cmp_make
				);
			}
		}
	}

	write_file("history.csv",v);

	/*
	 * some things to sanity check
	 * 1) number of appearances of (team/year) combo should be <10?
	 * 2) appearances should be <=3
	 * 3) after the 2nd event, teams should move less
	 * 4) teams should move more on dates where they finish an event
	 * 5) dcmp probabilities should move to 0/1 at the end
	 * probabilities should be in the range 0-1
	 * date years should correspond to the year
	 * overall entropy for a district should go down over time (both DCMP and CMP)
	 * the number of teams in a district should stay constant (may not, bad data known)
	 *
	 * */

	/*for each district:
	 * figure out what dates are meaningful changes (aka events end)
	 * calculate the probabilities at each of the cutoffs
	 * for each (year/team) make a listing of the DCMP and CMP probability after each one
	 *
	 * then, make look at how the uncertainty level converges towards the end
	 * -also, could look at a meta-analysis of how team actually do vs prediction at different points in the season
	 * -can look at things through the les of # of days till DCMP
	 *  -or till CMP
	 *  -or by week
	 *  -or by number of events that they have been to
	 * and can look at the team in terms of -raw probability -entropy
	 *
	 * if had probability x before event, what does the distribution of probabilities look like after?
	 * -probability distribution; might want buckets
	 *
	 *  how to divide buckets:
	 *  1) deciles
	 *  2) some sort of exponentially equal sizes:
	 *     1%
	 *     2%
	 *     4%
	 *     8%
	 *     16%
	 *     32%
	 *
	 *  3) like a z-table:
	 *      1: ...
	 *      0: .5
	 *	-1: .158
	 *	-2: .022
	 *	-3: .00135
	*/
	return 0;
}
