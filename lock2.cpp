#include "event_limits.h"
#include "event.h"

using namespace std;
using Team=tba::Team_key;

template<typename Func,typename K,typename V>
auto group(Func f,std::map<K,V> const& a){
	return group(f,to_vec(a));
}

template<typename A,typename B,typename C,typename D>
auto operator-(std::pair<A,B> const& a,std::pair<C,D> const& b){
	return std::make_pair(
		a.first-b.first,
		a.second-b.second
	);
}

template<typename A,typename B,typename C,typename D>
auto& operator-=(std::pair<A,B>& a,std::pair<C,D> const& b){
	a.first-=b.first;
	a.second-=b.second;
	return a;
}



auto lock2(Rank_status const& a,int dcmp_size){
	std::map<Team,string> r;
	//a.by_team
	//a.unclaimed

	auto g=reversed(sorted(group(
		[](auto x){
			return x.second.min;
		},
		a.by_team
	)));

	Rank_value min_threshold=[=](){
		//might be able to find a tighter threshold by assigning values to teams.
		int found=0;
		for(auto [value,teams]:g){
			found+=teams.size();
			if(found>=dcmp_size){
				return value;
			}
		}
		//everyone gets in
		return Rank_value();
	}();

	int already_ranked=0;
	for(auto i:range(g.size())){
		auto [min_value,teams_here]=g[i];

		auto slots_left=dcmp_size-already_ranked;
		if(slots_left<=0){
			//then these teams are not in range
			for(auto [team,info]:teams_here){
				if(info.max>=min_threshold){
					r[team]="out of range";
				}else{
					r[team]="out";
				}
			}
			continue;
		}

		if(slots_left<(int)teams_here.size()){
			for(auto [team,info]:teams_here){
				r[team]="in range, but already tied";
			}
			continue;
		}

		Rank_value budget=a.unclaimed;
		size_t found=0;
		for(size_t j=i+1;j<g.size();j++){
			auto [min2,teams2]=g[j];
			for(auto [team,info]:teams2){
				if(min_value>info.max){
					//this team can't get high enough; skip them
					//would be intersting to know how often this happens because this is
					//one of the key differences compared to the other method.
					continue;
				}
				auto cost=min_value-info.min;
				if(cost.second<0){
					cost.second=0;
				}
				if(cost<=budget){
					budget-=cost;
					found++;
				}else{
					//actually could break out of two layers of loops here
					goto done;
				}
			}
		}
		done:

		const auto leeway=slots_left-teams_here.size();
		if(found<=leeway){
			for(auto [team,info]:teams_here){
				r[team]="in";
			}
		}else{
			for(auto [team,info]:teams_here){
				std::stringstream ss;
				ss<<budget<<" "<<a.unclaimed;
				if(a.unclaimed.second){
					ss<<"\t"<<(budget.second/a.unclaimed.second);
				}
				r[team]="in range";
			}
		}
	}
	return r;
}

void lock2_demo(TBA_fetcher &f,tba::District_key district){
	auto in=district_limits(f,district);
	
	PRINT(in);
	PRINT(entropy(in));

	auto d=[=](){
		auto x=dcmp_size(district);
		assert(x.size()==1);
		return x[0];
	}();

	auto out=lock2(in,d);
	//print_r(out);
	PRINT(in.unclaimed);
	PRINT(count(values(out)));
}

int lock2_demo(TBA_fetcher &f){
	//lock2_demo(f,tba::District_key("2022chs"));
	lock2_demo(f,tba::District_key("2022pnw"));

	return 0;
}
