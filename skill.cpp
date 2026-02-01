#include "skill.h"
#include "tba.h"
#include "../tba/tba.h"
#include "util.h"
#include "set.h"
#include "multiset_flat.h"
#include "run.h"
#include "skill_opr.h"
#include "vector_void.h"

using Year=tba::Year;
using District_abbreviation=tba::District_abbreviation;
using District_key=tba::District_key;
using Team_key=tba::Team_key;
using Points=tba::Points;

using namespace std;

std::ostream& operator<<(std::ostream& o,Skill_estimates const& a){
	o<<"Skill_estimates( ";
	#define X(A,B) o<<""#B<<":"<<a.B<<" ";
	SKILL_ESTIMATES(X)
	#undef X
	return o<<")";
}

Skill_estimates skill_null(TBA_fetcher &f,District_key const& district){
	Skill_estimates r;
	auto pr=flat_map2(historical_event_pts(f));

	r.pre_dcmp=to_map(mapf(
		[=](auto team){
			return make_pair(team,convolve(pr,pr));
		},
		district_teams_keys(f,district)
	));

	r.at_dcmp=[&](){
		std::map<Point,Team_dist> r;
		for(auto i:range(500)){
			r[i]=Team_dist(dcmp_distribution(f));
		}
		return r;
	}();

	for(auto x:range(300)){
		r.second_event[x]=pr;
	}

	return r;
}

Skill_estimates skill_estimates(TBA_fetcher &f,District_key const& district,Skill_method method){
	switch(method){
		case Skill_method::POINTS:
			return calc_skill(f,district);
		case Skill_method::OPR:
			return calc_skill_opr(f,district);
		case Skill_method::NONE:
			return skill_null(f,district);
		default:
			assert(0);
	}
}

template<typename T>
auto median(std::multiset<T> const& a){
	return median(multiset_flat<T>(to_vec(a)));
}

template<typename T>
auto quartiles(std::multiset<T> const& a){
	auto b=sorted(to_vec(a));
	assert(!b.empty());
	return make_tuple(b[b.size()/4],b[b.size()/2],b[b.size()*3/4]);
}

Skill_method decode(std::span<char*> a,Skill_method const*){
	assert(a.size()==1);
	std::string s(a[0]);
	#define X(A) if(s==""#A){ return Skill_method::A; }
	SKILL_METHOD(X)
	#undef X

	cerr<<"Invalid argument for skill method.  Options are:\n";
	#define X(A) cerr<<"\t"#A<<"\n";
	SKILL_METHOD(X)
	#undef X
	exit(1);
}

std::optional<District_key> prev(TBA_fetcher& f,District_key a){
	Year year(stoi(a.get().substr(0,4)));
	auto abbrev=a.get().substr(4,100);
	Year year_p=year-1;

	auto h=tba::history(f,abbrev);
	auto found=filter([=](auto x){ return x.year==year_p; },h);
	switch(found.size()){
		case 0: return std::nullopt;
		case 1:
			return found[0].key;
		default:
			PRINT(found);
			nyi
	}

	//return District_key(as_string(year_p.get())+abbrev);
}

int pre_dcmp_pts(tba::District_Ranking const& a){
	return a.rookie_bonus+sum(mapf(
		[](auto x){ return x.total; },
		take(2,a.event_points)
	));
}

std::vector<int> team_points(TBA_fetcher& f,Team_key team,Year year){
	auto t=team_events_year(f,team,year);
	auto t2=filter(
		[](auto x){
			switch(x.event_type){
				case tba::Event_type::OFFSEASON:
				case tba::Event_type::PRESEASON:
				case tba::Event_type::CMP_DIVISION:
				case tba::Event_type::CMP_FINALS:
				case tba::Event_type::DISTRICT_CMP:
					return 0;
				case tba::Event_type::REGIONAL:
				case tba::Event_type::DISTRICT:
					return 1;
				default:
					PRINT(x);
					assert(0);
			}
		},
		t
	);

	if(t2.empty()){
		return {};
	}
	auto t3=take(2,t2);
	vector<int> r;
	for(auto x:t3){
		auto e=event_district_points(f,x.key);
		if(!e){
			PRINT(team)
			PRINT(year)
			PRINT(x);
		}
		assert(e);
		auto y=e->points[team].total;
		r|=y;
	}
	return r;
}

int old_points(TBA_fetcher& f,Team_key const& team,Year year){
	auto t=team_points(f,team,year-1);
	if(t.size()==1){
		return t[0]*2;
	}
	return sum(t);
}

Year year(District_key a){
	auto s=a.get().substr(0,4);
	return Year(stoi(s));
}

struct Skill_by_pts{
	std::map<Point,Team_dist> pre_dcmp,at_dcmp,second_event;
};

Skill_by_pts calc_skill_inner(TBA_fetcher& f){
	/*
	for each district:
		calc list of years that it has existed w/ a dcmp
		calc list of teams in it in each year
		calc pre-dcmp district points for each team/year combo
		calc total district points for each team/year combo
		for pairs of consecutive years:
			for each team that existed in both
				a=pts in year x
				b=pts in year x+1
				results[a]|=b;
			}
		}
	}
	
	form of result:
	# of points in prev year -> expected for pre-dcmp
	# of points in prev yera -> expected for whole season
	# of pre-dcmp points -> ...
	better predictor if lower stddev of result?

	TODO: Estimates for new teams
	*/

	//std::map<District_abbreviation,std::vector<Year>> district_years;

	std::set<tba::District_abbreviation> district_names;

	for(auto year:range(Year{1992},Year{2027})){
		auto d=districts(f,year);
		for(auto elem:d){
			district_names|=elem.abbreviation;
			//auto a=elem.abbreviation;
			//district_years[a]|=year;
			//auto x=dcmp_history(f,a);
			//PRINT(x);
		}
	}

	//PRINT(district_names);

	std::map<District_abbreviation,std::set<Year>> district_years;
	for(auto d:district_names){
		//PRINT(d);
		auto x=tba::dcmp_history(f,d);
		auto years=to_set(mapf([](auto x){ return x.event.year; },x));
		years-=Year(2020);//because the dcmp events did not happen in a normal way.
		years-=Year(2026);//because at the time of writing, this season is not over.
		//print_lines(x.event);
		//PRINT(years);
		district_years[d]=years;
	}

	//goes to pre-dcmp and overall total pts
	std::map<std::pair<Year,Team_key>,std::pair<Point,Point>> pts;
	std::map<Point,multiset_flat<Point>> second_event_raw;

	for(auto [k,v]:district_years){
		for(auto year:v){
			District_key key(::as_string(year)+k);
			auto d=tba::district_rankings(f,key);
			if(!d){
				//PRINT(k)
				//PRINT(year);
				continue;
			}
			assert(d);
			for(auto a:*d){
				auto district_events=take(2,a.event_points);
				int n=a.rookie_bonus+sum(mapf([](auto x){ return (int)x.total; },district_events));
				pts[make_pair(year,a.team_key)]=make_pair(n,a.point_total);

				if(district_events.size()==2){
					auto a=district_events[0].total;
					auto b=district_events[1].total;
					second_event_raw[a]|=b;
				}
			}
		}
	}

	//PRINT(count(values(pts)));

	map<Point,multiset_flat<Point>> adjacent,adjacent2,at_dcmp;
	for(auto [k,v]:pts){
		auto [year,team]=k;
		auto k2=make_pair(year+1,team);
		auto f=pts.find(k2);
		if(f==pts.end()){
			continue;
		}
		adjacent[v.first]|=f->second.first;
		adjacent2[v.second]|=f->second.first;
		
		Point dcmp_pts=v.second-v.first;
		//assuming that if there was an appearance, then there were points...
		if(dcmp_pts){
			at_dcmp[v.first]|=dcmp_pts;
		}
	}

	/*for(auto [k,v]:adjacent){
		cout<<k<<" "<<v.size()<<"\n";
		PRINT(count(v));
	}*/

	auto sample_size=sum(MAP(std::size,values(adjacent)));
	(void)sample_size;
	//PRINT(sample_size);

	//using Dist=std::multiset<int>;

	auto calc_smoothed=[=](std::map<Point,multiset_flat<Point>> &in){
		map<Point,multiset_flat<Point>> smoothed;

		auto k=keys(in);
		int min1=min(k);
		int max1=max(k)+10;
		for(auto k:range(min1,max1)){
			auto get_samples=[&](){
				for(int width=0;width<200;width++){
					auto to_use=range(k-width,k+width+1);
					multiset_flat<Point> found;
					for(auto n:to_use){
						found|=in[n];
					}
					if(found.size()>=100){
						return found;
					}
				}
				//not enough data; should not be reachable.
				PRINT(k);
				assert(0);
			}();

			smoothed[k]=get_samples;
		}
		return smoothed;
	};

	auto s1=calc_smoothed(adjacent);
	//auto s2=calc_smoothed(adjacent2);
	auto s3=calc_smoothed(at_dcmp);
	/*for(auto [k,v]:s3){
		cout<<k<<" "<<v.size()<<"\t";
		cout<<std_dev(v);
		cout<<"\t"<<quartiles(v)<<"\n";
	}*/

	/*auto sd1=mean(MAP(std_dev,values(s1)));
	auto sd2=mean(MAP(std_dev,values(s2)));
	PRINT(sd1);
	PRINT(sd2);*/

	auto to_pr=[](auto const& m){
		return map_values(
			[](auto const& x)->Team_dist{
				Team_dist r;
				for(auto k:to_set(x)){
					r[k]=(double)x.count(k)/x.size();
				}
				return r;
			},
			m
		);
	};

	auto pre_dcmp=to_pr(s1);
	auto at_dcmp_out=[=](){
		//For the values lower than the worst that has been seen, make them equal to that.
		auto x=to_pr(s3);
		auto min_seen=min(keys(x));
		for(auto i:range(min_seen)){
			x[i]=x[min_seen];
		}
		return x;
	}();

	auto second_event=to_pr(calc_smoothed(second_event_raw));

	return Skill_by_pts(pre_dcmp,at_dcmp_out,second_event);
}

Skill_by_pts const& calc_skill(TBA_fetcher& f){
	static auto a=calc_skill_inner(f);
	return a;
}

Skill_estimates calc_skill(TBA_fetcher &f,District_key const& district){
	auto d=district_rankings(f,district);
	assert(d);
	//should fall back to something else if this fails
	auto &current=*d;

	auto prev_district=prev(f,district);
	//PRINT(prev_district);
	auto prev=[&](){
		if(prev_district){
			auto d=district_rankings(f,*prev_district);
			assert(d);
			return *d;
		}
		return std::vector<tba::District_Ranking>();
	}();

	/*
	 * 1) figure out which teams are in the district in the given year
	 * 2) Figure out which events that team has played this year
	 * 2) for each of those teams, figure out how they did last year
	 * 3) call calc_skill() to try to project if no events played this year
	 * 4) figure out how many pts to expect if has played one event w/ results...
	 * for now, could just do the existing thing when one event has been played
	 * */
	std::map<Team_key,Team_dist> r;

	auto [c,at_dcmp,second_event]=calc_skill(f);

	for(auto x:current){
		auto team=x.team_key;
		auto found=filter([=](auto y){ return y.team_key==team; },prev);
		int to_index;

		if(found.empty()){
			//for now, putting in generic data about how good teams are
			//would be better if looked at how good this team was outside
			//of the district if it existed and how good rookies are if 
			//not
			to_index=old_points(f,team,year(district));

			//auto h=flat_map2(historical_event_pts(f));
			//r[team]=convolve(h,h);
			//continue;
		}else{

			if(found.size()!=1){
				PRINT(district);
				PRINT(team);
				PRINT(found);
			}
			assert(found.size()==1);
			auto old=found[0];
			auto pre_dcmp=pre_dcmp_pts(old);
			to_index=pre_dcmp;
		}
		r[team]=c[to_index];
	}

	return Skill_estimates(r,at_dcmp,second_event);
}

void demo(){
	TBA_fetcher_config tba;
	auto f=tba.get();
	for(auto year:range(Year(1992),Year(2027))){
		auto d=districts(f,year);
		if(d.empty()) continue;
		for(auto x:d){
			//PRINT(x.key);
			try{
				calc_skill(f,x.key);
			}catch(tba::Decode_error const& a){
				cout<<"fail: "<<a<<"\n";
			}
		}
	}

	auto c=calc_skill(f,District_key("2025pnw"));
	//print_lines(c);
}
