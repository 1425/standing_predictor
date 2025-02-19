#include "run.h"
#include<random>
#include "multiset_flat.h"

using namespace std;

flat_map<Point,Pr> convolve(std::map<Point,Pr> const& a,std::map<Point,Pr> const& b){
	flat_map<Point,Pr> r;
	for(auto [a1,ap]:a){
		for(auto [b1,bp]:b){
			auto result=a1+b1;
			auto pr=ap*bp;
			auto f=r.find(result);
			if(f==r.end()){
				r[result]=pr;
			}else{
				f->second+=pr;
			}
		}
	}
	return r;
}

flat_map<Point,Pr> convolve(flat_map<Point,Pr> const& a,std::map<Point,Pr> const& b){
	flat_map<Point,Pr> r;
	for(auto [a1,ap]:a){
		for(auto [b1,bp]:b){
			auto result=a1+b1;
			auto pr=ap*bp;
			auto f=r.find(result);
			if(f==r.end()){
				r[result]=pr;
			}else{
				f->second+=pr;
			}
		}
	}
	return r;
}

flat_map2<Point,Pr> convolve(flat_map2<Point,Pr> const& a,std::map<Point,Pr> const& b){
	flat_map2<Point,Pr> r;
	for(auto [a1,ap]:a){
		for(auto [b1,bp]:b){
			auto result=a1+b1;
			auto pr=ap*bp;
			auto f=r.find(result);
			if(f==r.end()){
				r[result]=pr;
			}else{
				f->second+=pr;
			}
		}
	}
	return r;
}

flat_map2<Point,Pr> convolve(flat_map2<Point,Pr> const& a,flat_map<Point,Pr> const& b){
	flat_map2<Point,Pr> r;
	for(auto [a1,ap]:a){
		for(auto [b1,bp]:b){
			auto result=a1+b1;
			auto pr=ap*bp;
			auto f=r.find(result);
			if(f==r.end()){
				r[result]=pr;
			}else{
				f->second+=pr;
			}
		}
	}
	return r;
}

flat_map2<Point,Pr> convolve(flat_map2<Point,Pr> const& a,flat_map2<Point,Pr> const& b){
	flat_map2<Point,Pr> r;
	for(auto [a1,ap]:a){
		for(auto [b1,bp]:b){
			auto result=a1+b1;
			auto pr=ap*bp;
			auto f=r.find(result);
			if(f==r.end()){
				r[result]=pr;
			}else{
				f->second+=pr;
			}
		}
	}
	return r;
}

map<Point,Pr> when_greater(map<Point,Pr> const& a,map<pair<Point,double>,Pr> const& b){
	map<Point,Pr> r;
	for(auto [ka,va]:a){
		for(auto [kb,vb]:b){
			auto [value,pr]=kb;
			if(ka>value){
				r[ka]+=va*vb;
			}else if(ka==value){
				r[ka]+=va*vb*pr;
			}
		}
	}
	return r;
}

template<typename T,template<typename,typename> typename MAP>
auto when_greater(T const& a,MAP<pair<Point,double>,Pr> const& b){
	T r;
	for(auto [ka,va]:a){
		for(auto [kb,vb]:b){
			auto [value,pr]=kb;
			if(ka>value){
				r[ka]+=va*vb;
			}else if(ka==value){
				r[ka]+=va*vb*pr;
			}
		}
	}
	return r;
}

map<Point,Pr> when_greater(map<Point,Pr> const& a,map<Point,Pr> const& b){
	//what the probability distribution of "a" looks like when it is higher than "b".
	//note that the output is not expected to sum to 1 unless a is always greater than b.
	//O(a.size()*b.size()) but this should still be better than sampling them since N and M are not that large.
	map<Point,Pr> r;
	for(auto [ka,va]:a){
		for(auto [kb,vb]:b){
			if(ka>kb){
				r[ka]+=va*vb;
			}
		}
	}
	return r;
}

auto find_cutoff(map<pair<bool,Point>,unsigned> these_points,unsigned eliminating){
	//if for some reason, there are equal or fewer teams than slots, then return 
	//that 0 points is the cutoff, and there is no excess.
	if(eliminating==0){
		return make_pair(0,1.0);
	}

	unsigned total=0;
	for(auto [points,teams]:these_points){
		total+=teams;
		if(total>=eliminating){
			auto excess=total-eliminating;
			assert(points.first==0);
			return make_pair(points.second,1-double(excess)/teams);
		}
	}
	assert(0);
}

auto find_cutoff(flat_map<pair<bool,Point>,unsigned> these_points,unsigned eliminating){
	//if for some reason, there are equal or fewer teams than slots, then return 
	//that 0 points is the cutoff, and there is no excess.
	if(eliminating==0){
		return make_pair(0,1.0);
	}

	unsigned total=0;
	for(auto [points,teams]:these_points){
		total+=teams;
		if(total>=eliminating){
			auto excess=total-eliminating;
			assert(points.first==0);
			return make_pair(points.second,1-double(excess)/teams);
		}
	}
	assert(0);
}

auto find_cutoff(flat_map2<pair<bool,Point>,unsigned> these_points,unsigned eliminating){
	//if for some reason, there are equal or fewer teams than slots, then return 
	//that 0 points is the cutoff, and there is no excess.
	if(eliminating==0){
		return make_pair(0,1.0);
	}

	unsigned total=0;

	for(auto [points,teams]:these_points){
		total+=teams;
		if(total>=eliminating){
			auto excess=total-eliminating;
			assert(points.first==0);
			return make_pair(points.second,1-double(excess)/teams);
		}
	}
	assert(0);
}

Run_result run_calc(
#if 0
	int dcmp_size,
	int worlds_slots,

	/*for each team, did they win a district chairmans's award and how
	  many points are they expected to have by the time of the district
	  championship
	  */
	map<tba::Team_key,pair<bool,Team_dist>> by_team,

	bool dcmp_played,
	flat_map2<Point,Pr> dcmp_distribution1,
	std::vector<tba::District_Ranking> d1,
	map<tba::Team_key,tuple<vector<int>,int,int>> points_used //this is only passed through
#endif
	Run_input input
){
	//This function exists to run the calculations of how teams are
	//expected to do, seperatedly from doing any IO.

	auto teams_advancing=input.dcmp_size;
	auto teams_competing=input.by_team.size();
	unsigned teams_left_out=max(0,(int)teams_competing-(int)teams_advancing); //Ontario had more slots than teams in 2022.
	unsigned cmp_teams_left_out=max(0,input.dcmp_size-input.worlds_slots);

	//monte carlo method for where the cutoff is

	std::mt19937_64 rng;
	// initialize the random number generator with time-dependent seed
	uint64_t timeSeed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	std::seed_seq ss{uint32_t(timeSeed & 0xffffffff), uint32_t(timeSeed>>32)};
	rng.seed(ss);
	// initialize a uniform distribution between 0 and 1
	std::uniform_real_distribution<double> unif(0, 1);

	//auto sample=[&](map<Point,Pr> const& m)->Point{
	auto sample=[&](auto const& m)->Point{
		auto num=unif(rng);
		double total=0;
		for(auto [value,pr]:m){
			total+=pr;
			if(total>num){
				return value;
			}
		}
		assert(0);
	};

	//multiset<pair<Point,Pr>> dcmp_cutoffs,cmp_cutoff;
	multiset_flat<pair<Point,Pr>> dcmp_cutoffs,cmp_cutoff;
	static const auto iterations=2000; //usually want this to be like 2k
	for(auto iteration:range_st<iterations>()){
		(void)iteration;
		//PRINT(iteration);
		flat_map2<pair<bool,Point>,unsigned> final_points;
		for(auto const& [team,data]:input.by_team){
			auto [cm,dist]=data;
			final_points[pair<bool,Point>(cm,sample(dist))]++;
		}

		auto dcmp_cutoff=find_cutoff(final_points,teams_left_out);
		dcmp_cutoffs|=dcmp_cutoff;

		flat_map2<pair<bool,Point>,unsigned> post_dcmp_points;
		for(auto [earned,teams]:final_points){
			auto [cm,points]=earned;

			if(!cm && points<dcmp_cutoff.first) continue;
			if(points==dcmp_cutoff.first){
				teams*=(1-dcmp_cutoff.second);
			}

			for(auto _:range(teams)){
				(void)_;
				int pts;
				if(input.dcmp_played){
					pts=points;
				}else{
					pts=points+sample(input.dcmp_distribution1);
				}
				post_dcmp_points[make_pair(0,pts)]++;
			}
		}

		cmp_cutoff|=find_cutoff(post_dcmp_points,cmp_teams_left_out);
	}

	//map<pair<Point,Pr>,Pr> cutoff_pr=flat_map2(map_values(
	auto cutoff_pr=flat_map2(map_values(
		[=](auto x){ return (0.0+x)/iterations; },
		count(dcmp_cutoffs)
	));

	//map<pair<Point,Pr>,Pr> cmp_cutoff_pr=map_values(
	auto cmp_cutoff_pr=flat_map2(map_values(
		[=](auto x){ return (0.0+x)/iterations; },
		count(cmp_cutoff)
	));
	//auto cutoff_level=[=](map<pair<Point,Pr>,Pr> const& cutoff_set,Pr probability_target){
	auto cutoff_level=[=](auto const& cutoff_set,Pr probability_target){
		double t=0;
		for(auto [points,pr]:cutoff_set){
			t+=pr;
			if(t>probability_target){
				return points;
			}
		}
		assert(0);
	};

	auto interesting_cutoffs=[=](auto cutoff_set){
		map<Pr,Extended_cutoff> r;
		for(auto d:{.05,.5,.95}){
			r[d]=cutoff_level(cutoff_set,d);
		}
		return r;
	};
	auto interesting_cutoffs_dcmp=interesting_cutoffs(cutoff_pr);
	auto interesting_cutoffs_cmp=interesting_cutoffs(cmp_cutoff_pr);

	vector<Result_tuple> result;
	for(auto team:input.d1){
		//probability that get in
		//subtract the cutoff pr
		auto [cm,team_pr]=input.by_team[team.team_key];
		double pr_make=0;
		double pr_miss=0;
		if(cm){
			pr_make=1;
			pr_miss=0;
		}else{
			for(auto [cutoff,c_pr]:cutoff_pr){
				for(auto [team_value,t_pr]:team_pr){
					auto combined_pr=c_pr*t_pr;
					if(team_value>cutoff.first){
						pr_make+=combined_pr;
					}else if(team_value==cutoff.first){
						pr_make+=combined_pr*(1-cutoff.second);
						pr_miss+=combined_pr*cutoff.second;
					}else{
						pr_miss+=combined_pr;
					}
				}
			}
		}
		auto total=pr_make+pr_miss;
		assert(total>.99 && total<1.01);

		auto dcmp_entry_dist=[=,cm=cm,team_pr=team_pr]()->Team_dist{
			if(cm){
				return team_pr;
			}
			return when_greater(team_pr,cutoff_pr);
		}();
		auto post_dcmp_dist=convolve(dcmp_entry_dist,input.dcmp_distribution1);
		auto post_total=sum(values(post_dcmp_dist));
		Pr cmp_make=0;
		Pr cmp_miss=0;
		for(auto [cutoff,c_pr]:cmp_cutoff_pr){
			for(auto [team_value,t_pr]:post_dcmp_dist){
				auto combined_pr=c_pr*t_pr;
				if(team_value>cutoff.first){
					cmp_make+=combined_pr;
				}else if(team_value==cutoff.first){
					cmp_make+=combined_pr*(1-cutoff.second);
					cmp_miss+=combined_pr*cutoff.second;
				}else{
					cmp_miss+=combined_pr;
				}
			}
		}
		{
			auto residual=post_total-cmp_make-cmp_miss;
			assert(residual<.01 && residual>-.01);
		}

		auto points_so_far=team.point_total;
		vector<Point> cmp_interesting;
		for(auto [pr,pts]:interesting_cutoffs_cmp){
			cmp_interesting|=max(Point(0),Point(pts.first-points_so_far));
		}

		if(cm){
			assert(pr_make>.99);
			result|=Result_tuple(
				team.team_key,
				1.0,0,0,0,
				cmp_make,cmp_interesting[0],cmp_interesting[1],cmp_interesting[2]
			);
		}else{
			//PRINT(total);

			//PRINT(pr_make);
			//points needed to have 50% odds; 5% odds; 95% odds, or quartiles?
			vector<Point> interesting;
			for(auto [pr,pts]:interesting_cutoffs_dcmp){
				//cout<<pr<<":"<<max(0.0,pts-points_so_far)<<"\n";
				auto value=max(Point(0),Point(pts.first-points_so_far));
				interesting|=value;
			}
			assert(interesting.size()==3);

			result|=make_tuple(
				team.team_key,
				pr_make,interesting[0],interesting[1],interesting[2],
				cmp_make,cmp_interesting[0],cmp_interesting[1],cmp_interesting[2]
			);
		}
	}

	auto x=::mapf([](auto x){ return get<1>(x); },result);
	//PRINT(sum(x)); //this number should be really close to the number of slots available at the event.

	return Run_result{result,cutoff_pr,cmp_cutoff_pr,input.points_used,input.by_team};
}

