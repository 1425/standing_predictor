#include "history.h"
#include<vector>
#include<filesystem>
#include<boost/tokenizer.hpp>
#include "tba.h"
#include "skill.h"
#include "event_partial.h"
#include "rand.h"
#include "declines.h"
#include "skill_opr.h"
#include "../tba/tba.h"
#include "query.h"
#include "decode.h"

using namespace std;

using Year=tba::Year;

std::set<tba::Date> meaningful_dates(TBA_fetcher &f,tba::District_key const& district){
	auto e=events(f,district);
	auto start=min(mapf([](auto x){ return *x.start_date; },e));
	auto ends=to_set(mapf([](auto x){ return *x.end_date; },e));
	return start|ends;
}

ELEMENTWISE_RAND(History_item,HISTORY_ITEM)
PRINT_STRUCT(History_item,HISTORY_ITEM)

auto dcmp(History_item const& a){
	return a.dcmp_pr;
}

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
	for(auto _:range_st<100>()){
		(void)_;

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

static const std::string HISTORY_TMP_PATH="history.csv";

void write_history(TBA_fetcher &f){
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
				//appearances|=keys(event_data.by_team);

				auto fixed=mapf([](auto x){ return x.width().second==0; },values(event_data.by_team));
				if(all(fixed)){
					appearances|=keys(event_data.by_team);
				}else if(none(fixed)){
					//future event
					break;
				}else{
					//what is happening here?
					//event in progress?
					assert(0);
				}
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

	write_file(HISTORY_TMP_PATH,v);

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
}

using Team_key=tba::Team_key;

std::optional<Team_key> mean(std::vector<Team_key>){
	return std::nullopt;
}

int operator/(tba::Year,size_t);

auto mean(std::vector<tba::Year> a){
	auto m=mapf([](auto x){ return x.get(); },a);
	return mean(m);
}

auto mean(std::vector<tba::Date> a){
	tba::Date base(
		std::chrono::year(2000),
		std::chrono::month(1),
		std::chrono::day(1)
	);
	auto m=mapf([=](auto x){ return x-base; },a);
	return base+mean(m);
}

template<typename T>
void analyze(std::vector<T> const& x){
	if(x.empty()){
		cout<<"Empty\n";
		return;
	}
	PRINT(min(x));
	PRINT(max(x));
	PRINT(mean(x));
	PRINT(quartiles(x));
	PRINT(deciles(x));

	auto s=to_set(x);
	cout<<"unique options:"<<s.size()<<"\n";
	if(s.size()<10){
		auto c=count(x);
		print_lines(c);
	}

	cout<<"\n";
}

template<typename T>
bool ascending_or_eq(std::vector<T> const& a){
	for(auto i:range(a.size()-1)){
		if(a[i]>a[i+1]){
			return 0;
		}
	}
	return 1;
}

using Date=tba::Date;

using s8=Int_limited<-128,127>;

#define NORMALIZED_ITEM(X)\
	X(std::chrono::days,cmp_days)\
	X(std::chrono::days,dcmp_days)\
	X(size_t,appearances)\
	X(Pr,dcmp_pr)\
	X(Pr,cmp_pr)\

STRUCT_DECLARE(Normalized_item,NORMALIZED_ITEM)
PRINT_STRUCT(Normalized_item,NORMALIZED_ITEM)

auto week_normalize(std::vector<Normalized_item> const& a){
	//auto week=[](auto x){ return (x.cmp_days.count()-1)/7; };
	//actually just look at a day.
	auto week=[](auto x){ return x.cmp_days.count(); };

	auto g=group(week,a);
	auto g2=map_values([](auto x){ return last(x); },g);
	auto x=reversed(sorted(values(g2)));
	//print_r(x);
	return mapf(
		[=](auto x){
			auto [a,b]=x;
			return make_tuple(week(b),a.appearances!=b.appearances,a.cmp_pr,b.cmp_pr);
		},
		adjacent_pairs(x)
	);
}

int normalize_data(TBA_fetcher &f,std::vector<History_item> a){
	a=filter(
		[&](auto x){
			return x.year<2026 &&
				x.year!=2020 && 
				x.year!=2021;
		},
		a
	);

	auto g=group([](auto x){ return make_pair(x.team,x.year); },a);
	auto g2=nonempty(mapf(
		[&](auto x)->optional<vector<Normalized_item>>{
			auto [k,v]=x;
			auto [team,year]=k;
			auto district_key=district(f,team,year);
			if(!district_key){
				return std::nullopt;
			}
			auto dcmp_date=dcmp_start(f,*district_key);
			auto cmp_date=cmp_start(f,year);

			auto found=mapf(
				[=](auto y){
					return Normalized_item(
						cmp_date-y.date,
						dcmp_date-y.date,
						y.appearances,
						y.dcmp_pr,
						y.cmp_pr
					);
				},
				v
			);
			//print_lines(found);
			//why are some of the days ending w/ end of dcmp?
			return found;
		},
		g
	));

	//print_r(g2);

	auto w=flatten(mapf(week_normalize,g2));

	//print_r(w);

	//week,playing,before,after
	map<pair<int,bool>,vector<pair<double,double>>> org;
	for(auto x:w){
		org[make_pair(get<0>(x),get<1>(x))]|=make_pair(get<2>(x),get<3>(x));
	}
	//print_r(org);

	using Box=int;
	using Dist=map<Box,Pr>;
	map<pair<int,bool>,map<Box,Dist>> r;

	auto mv=map_values(
		[](std::vector<std::pair<double,double>> x){
			map<Box,multiset<Box>> c;
			auto quantize=[](auto d){ return int(d*10); };
			for(auto [a,b]:x){
				c[quantize(a)]|=quantize(b);
			}
		//	PRINT(take(5,x));
		//	PRINT(x.size());
		//	PRINT(c);
			return map_values(
				[](auto y){
					Dist r;
					for(auto k:to_set(y)){
						r[k]=(0.0+y.count(k))/y.size();
					}
					return r;
				},
				c
			);
		},
		org
	);

	//print_lines(mv);

	for(auto [k,v]:mv){
		PRINT(k);
		//print_r(v);
		for(auto [k2,v2]:v){
			cout<<"\t"<<k2<<"\n";
			print_r(2,v2);
		}
	}

	//first, find the slope when not competing?
	//or markov chain?
	//could turn everything into approximate weeks
	
	//map<std::chrono::days,

	//weeks out, playing this week, current status box, next status, pr
	
	{
		ofstream f("transition_p.csv");
		{
			for(auto [k,v]:mv){
				for(auto [k2,v2]:v){
					for(auto [k3,v3]:v2){
						f<<k.first<<","<<k.second<<","<<k2<<","<<k3<<","<<v3<<"\n";
					}
				}
			}
		}
	}

	return 0;
}

map<pair<int,bool>,map<int,map<int,Pr>>> read_dist(){
	ifstream f("transition_p.csv");
	//no header
	std::map<pair<int,bool>,map<int,map<int,Pr>>> r;
	std::string line;
	while(getline(f,line)){
		boost::tokenizer<boost::escaped_list_separator<char>> t(line);
		/*int weeks_out=t[0];
		bool playing_this_week=t[1];
		int current_status_box=t[2];
		int next_status_box=t[3];
		Pr p=t[4];*/
		auto it=t.begin();
		#define X(TYPE,NAME) assert(it!=t.end()); TYPE NAME=decode(*it,(TYPE*)0); ++it;
		X(int,weeks_out)
		X(bool,playing_this_week)
		X(int,current_status_box)
		X(int,next_status_box)
		X(Pr,p)
		#undef X
		r[make_pair(weeks_out,playing_this_week)][current_status_box][next_status_box]=p;
		//PRINT(r.size());
	}
	return r;
}

static const std::string TRANSITION_TABLE_PATH="transition_p.csv";

std::map<std::pair<int,bool>,std::map<int,std::map<int,Pr>>> transition_table(TBA_fetcher &f){
	if(!std::filesystem::exists(TRANSITION_TABLE_PATH)){
		auto data=read_history(f);
		int r=normalize_data(f,data);
		assert(r==0);
	}
	return read_dist();
}

int read_dist_demo(){
	auto r=read_dist();
	print_r(r);
	return 0;
}


std::vector<History_item> read_history(TBA_fetcher &f){
	if(!std::filesystem::exists(HISTORY_TMP_PATH)){
		write_history(f);
	}
	return read_file(HISTORY_TMP_PATH);
}

int history_demo(TBA_fetcher& f){
	//return read_dist_demo();

	//write_history(f);
	(void)f;

	auto data=read_file(HISTORY_TMP_PATH);

	return normalize_data(f,data);

	/* Variables to look at:
	 * 1) what is current probability
	 * 2) how far out are you from dcmp
	 *
	 * then go to distribution of probability afterwards
	 *
	 * how to find points to look at: maholnobis distance?
	 * stddev of probabilities
	 * stddev of distance to dcmp?
	 *
	 * */

	/*#define HISTORY_ITEM(X)\
	X(tba::Team_key,team)\
	X(Year,year)\
	X(tba::Date,date)\
	X(size_t,appearances)\
	X(Pr,dcmp_pr)\
	X(Pr,cmp_pr)*/

	if(0){	
		#define X(A,B) cout<<""#B<<"\n"; analyze(mapf([](auto x){ return x.B; },data));
		HISTORY_ITEM(X)
		#undef X
	}

	auto g=group([](auto x){ return make_pair(x.year,x.team); },data);

	if(1){
		auto lengths=mapf([](auto x){ return x.size(); },values(g));
		cout<<"lengths:\n";
		analyze(lengths);
	}

	cout<<"Unique team years:"<<g.size()<<"\n";

	for(auto p:g){
		auto [k,v]=p;
		//within a series, # of appearances should only go up (or stay the same)
		auto m=mapf([](auto x){ return x.appearances; },v);
		assert(ascending_or_eq(m));
		//should always start at 0 appearances?
		//print_r(p);
	}

	//TODO: Figure out the start/end dates for each of the district championships
	//so that know when to cut things off 
	
	//first, how much does a probability tend to change after each date?
	
	auto found=flatten(mapf(
		[](auto x){
			return mapf([](auto x){ return x.second-x.first; },adjacent_pairs(MAP(dcmp,x)));
		},
		values(g)
	));

	cout<<"Differences by date:\n";
	analyze(found);

	auto found2=flatten(mapf(
		[](auto x){
			std::vector<History_item> v;
			for(auto elem:x){
				if(v.empty()){
					v|=elem;
				}else{
					if(last(v).appearances!=elem.appearances){
						v|=elem;
					}
				}
			}
			//print_lines(v);
			return mapf([](auto x){ return x.second-x.first; },adjacent_pairs(MAP(dcmp,v)));
		},
		values(g)
	));

	cout<<"Differences on play:\n";
	analyze(found2);

	auto f2mag=MAP(fabs,found2);
	cout<<"Magnitude:\n";
	analyze(f2mag);

	//from each of the buckets, which bucket likely to end up in
	//afterwards
	
	vector<tuple<int,int,double,double>> vfound;

	for(auto [k,v]:g){
		for(auto [a,b]:adjacent_pairs(v)){
			vfound|=make_tuple(a.appearances,b.appearances,a.dcmp_pr,b.dcmp_pr);
		}
	}

	//first, just generically, when there is a change in appearances
	vector<tuple<double,double>> f2;
	for(auto [a1,a2,p1,p2]:vfound){
		if(a1!=a2){
			f2|=make_tuple(p1,p2);
		}
	}

	auto box=[](Pr p){
		return int(p*10);
	};

	using Box=int;
	map<Box,std::multiset<Box>> count;

	for(auto [a,b]:f2){
		count[box(a)]|=box(b);
	}

	for(auto [k,v]:count){
		cout<<k<<"\t"<<v.size()<<"\t"<<mean(v)<<"\t"<<quartiles(v)<<"\n";
	}

	auto pd=map_values(
		[](auto x){
			//return to_dist(x);
			map<Box,Pr> r;
			for(auto k:to_set(x)){
				r[k]=(0.0+x.count(k))/x.size();
			}
			return r;
		},
		count
	);

	//print_r(pd);
	//print_lines(pd);

	for(auto [k,v]:pd){
		cout<<k<<",";
		/*for(auto [k2,v2]:v){
			cout<<v2<<",";
		}
		cout<<"\n";*/
		for(auto i:range_st<11>()){
			cout<<v[i]<<",";
		}
		cout<<"\n";
	}

	return 0;
}
