#include "run.h"
#include<random>
#include "multiset_flat.h"
#include "set.h"
#include "print_r.h"
#include "vector_void.h"
#include "array.h"
#include "vector.h"

using namespace std;

std::ostream& operator<<(std::ostream& o,Dcmp_data const& a){
	return o<<"Dcmp_data("<<a.size<<" "<<a.played<<" "<<a.dists<<")";
}

std::ostream& operator<<(std::ostream& o,Team_status const& a){
	o<<"Team_status( ";
	#define X(A,B) o<<""#B<<":"<<a.B<<" ";
	TEAM_STATUS(X)
	#undef X
	return o<<")";
}

std::ostream& operator<<(std::ostream& o,Run_input const& a){
	o<<"Run_input( ";
	#define X(A,B) o<<""#B<<":"<<a.B<<" ";
	RUN_INPUT_ITEMS(X)
	#undef X
	return o<<")";
}

template<typename T,size_t N,typename B>
std::array<T,N>& operator|=(std::array<T,N> &a,std::array<B,N> const& b){
	for(auto i:range_st<N>()){
		a[i]|=b[i];
	}
	return a;
}

bool approx_equal(double a,double b){
	return fabs(a-b)<.01;
}

flat_map<Point,Pr> convolve(flat_map2<Point,Pr> const& a,std::map<Point,flat_map2<Point,double>> const& b){
	//convolution may not be the correct name for this operation.
	//for each item in the first distribution, take a corresponding distribution out of 'b'
	//and and add the value from there

	flat_map<Point,Pr> r;
	for(auto [k,v]:a){
		auto find_dist=[&](){
			auto f=b.find(k);
			if(f!=b.end()){
				return f->second;
			}
			auto m=max(keys(b));
			auto f2=b.find(m);
			assert(f2!=b.end());
			return f2->second;
		};
		for(auto [k2,v2]:find_dist()){
			auto k3=k+k2;
			auto v3=v*v2;
			r[k3]+=v3;
		}
	}

	return r;
}

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
		return make_pair(Point(0),1.0);
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
		return make_pair(Point(0),1.0);
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

auto find_cutoff(flat_map2<pair<bool,Point>,unsigned> const& these_points,unsigned eliminating){
	//if for some reason, there are equal or fewer teams than slots, then return 
	//that 0 points is the cutoff, and there is no excess.
	if(eliminating==0){
		return make_pair(Point(0),1.0);
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

	if(these_points.empty()){
		return make_pair(Point(0),1.0);
	}
	PRINT(these_points);
	PRINT(eliminating);
	assert(0);
}

template<typename T,size_t N>
auto find_cutoff(std::array<T,N> const& a,auto const& b){
	return mapf([&](auto const& x){ return find_cutoff(x,b); },a);
}

template<typename T,size_t N,typename B>
auto find_cutoff(std::array<T,N> const& a,std::vector<B> const& b){
	return mapf(
		[&](auto i){
			if(i<b.size()){
				return find_cutoff(a[i],b[i]);
			}else{
				return find_cutoff(a[i],B{});
			}
		},
		range_st<N>()
	);
}

std::ostream& operator<<(std::ostream& o,Run_result const& a){
	o<<"Run_result( ";
	#define X(A,B) o<<""#B<<":"<<a.B<<" ";
	RUN_RESULT_ITEMS(X)
	#undef X
	return o<<")";
}

void print_r(int n,Run_result const& a){
	indent(n);
	cout<<"Run_result\n";
	n++;
	#define X(A,B) indent(n); std::cout<<""#B<<"\n"; print_r(n+1,a.B);
	RUN_RESULT_ITEMS(X)
	#undef X
}

template<typename T>
void check(std::vector<T> const&);

void check(int){}

void check(Pr p){
	assert(p>=0);
}

void check(Point a){
	if(a<0) throw "negative points";
	assert(a>=0);
}

template<typename K,typename V>
void check(flat_map2<K,V> const&);

void check(tba::Team_key const&){}

void check(Team_status const& a){

	#define X(A) try{\
		check(a.A);\
	}catch(char const *s){\
		std::vector<std::string> r;\
		r|=s;\
		r|=""#A;\
		r|="Team_status";\
		throw r;\
	}catch(std::vector<std::string> const& a){\
		std::vector<std::string> r;\
		r|=a;\
		r|=""#A;\
		r|="Team_status";\
		throw r;\
	}catch(...){\
		assert(0);\
	}
	X(point_dist)
	X(dcmp_home)
	X(already_earned)
	#undef X

	/*try{
		check(a.point_dist);
		check(a.dcmp_home);
		check(a.already_earned);
	}catch(char const *s){
		std::vector<std::string> r;
		r|=s;
		r|="Team_status";
		throw r;
	}*/
}


template<typename K,typename V>
void check(std::map<K,V> const& a){
	for(auto [k,v]:a){
		try{
			check(k);
		}catch(std::vector<std::string> const& a){
			vector<string> r;
			r|=a;
			r|="k";
			r|="map";
			throw r;
		}catch(...){
			assert(0);
		}
		try{
			check(v);
		}catch(std::vector<std::string> const& a){
			vector<string> r;
			r|=a;
			stringstream ss;
			ss<<"v("<<k<<")";
			r|=ss.str();
			r|="map";
			throw r;
		}catch(...){
			assert(0);
		}
	}
}

template<typename T>
void check(std::vector<T> const& a){
	for(auto const& x:a){
		check(x);
	}
}

void check(Dcmp_data const& a){
	assert(a.size>=0);
	check(a.dists);
}

void check(Run_input const& a){
	/*for(auto x:a.dcmp_size){
		assert(x>=0);
	}*/
	assert(a.worlds_slots>=0);
	string s="by_team";
	try{
		check(a.by_team);
		s="dcmp_distribution1";
	}catch(std::vector<string> const& a){
		vector<string> r;
		r|=a;
		r|=s;
		r|="Run_input";
		throw r;
	}catch(...){
		assert(0);
	}
	check(a.dcmp);
}

template<typename K,typename V>
void check(flat_map2<K,V> const& a){
	for(auto [k,v]:a){
		try{
			check(k);
		}catch(std::vector<std::string> const& a){
			std::vector<std::string> r;
			r|=a;
			r|="k";
			r|="flat_map2";
			throw r;
		}catch(const char *s){
			std::vector<std::string> r;
			r|=s;
			r|="k";
			r|="flat_map2";
			throw r;
		}catch(...){
			assert(0);
		}
		try{
			check(v);
		}catch(std::vector<std::string> const& a){
			std::vector<std::string> r;
			r|=a;
			r|="v";
			r|="flat_map2";
			throw r;
		}catch(const char *s){
			std::vector<std::string> r;
			r|=s;
			r|="v";
			r|="flat_map2";
			throw r;
		}catch(...){
			assert(0);
		}
	}
}

Run_result run_calc(
	Run_input input
){
	try{
		check(input);
	}catch(...){
		print_r(input.by_team[tba::Team_key("frc1294")]);
		throw;
	}
	#if 0
	//check that this incoming distributions look ok.
	for(auto [k,v]:input.by_team){
		auto s=sum(values(v.point_dist));
		assert(approx_equal(1,s));
	}

	for(auto [k,v]:input.dcmp_distribution1){
		auto s=sum(values(v));
		PRINT(k)
		PRINT(v);
		PRINT(s);
		assert(approx_equal(1,s));
	}
	#endif

	//This function exists to run the calculations of how teams are
	//expected to do, seperatedly from doing any IO.

	//auto teams_advancing=input.dcmp_size;
	//auto teams_competing=input.by_team.size();
	//unsigned teams_left_out=max(0,(int)teams_competing-(int)teams_advancing); //Ontario had more slots than teams in 2022.
	auto teams_competing=[=](auto dcmp_index){
		auto m=mapf([](auto x){ return x.second.dcmp_home; },input.by_team);
		auto s=to_multiset(m);
		return s.count(dcmp_index);
	};

	const auto dcmp_sizes=mapf([](auto x){ return x.size; },input.dcmp);

	auto teams_left_out=mapf(
		[=](auto p){
			auto [i,teams_advancing]=p;
			auto t=teams_competing(i);
			return max(0,(int)t-(int)teams_advancing);
		},
		enumerate(dcmp_sizes)
	);
	unsigned cmp_teams_left_out=max(0,(int)sum(dcmp_sizes)-input.worlds_slots);

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
		PRINT(num);
		PRINT(total);
		assert(0);
	};

	//multiset<pair<Point,Pr>> dcmp_cutoffs,cmp_cutoff;
	using Cutoff=multiset_flat<pair<Point,Pr>>;
	std::array<Cutoff,MAX_DCMPS> dcmp_cutoffs;
	Cutoff cmp_cutoff;

	/*usually want this to be like 2k
	but theoretically the numbers should get better up to about 20k iterations

	In practice it seems like 2k iterations should get you below 1.5% 
	difference in cutoff probabilities run to run and going up to 20k will
	get you below 0.5% run to run variation.
	*/
	static const auto iterations=2000;

	using Final_points=flat_map2<pair<bool,Point>,unsigned>;
	std::array<Final_points,MAX_DCMPS> final_points;

	/*for(auto [k,v]:input.dcmp_distribution1){
		cout<<k<<" "<<sum(values(v))<<"\n";
	}
	nyi*/

	for(auto iteration:range_st<iterations>()){
		(void)iteration;
		//PRINT(iteration);
		//final_points.clear();
		for(auto &x:final_points){
			x.clear();
		}

		for(auto const& [team,data]:input.by_team){
			auto const& [cm,dist,dcmp_home,already_earned]=data;
			final_points[dcmp_home][pair<bool,Point>(cm,sample(dist))]++;
		}

		auto dcmp_cutoff=find_cutoff(final_points,teams_left_out);
		dcmp_cutoffs|=dcmp_cutoff;

		flat_map2<pair<bool,Point>,unsigned> post_dcmp_points;
		for(auto [dcmp_index,dcmp]:enumerate(input.dcmp)){
			auto & final_points_this=final_points[dcmp_index];//not sure that this should really be mutable.
			auto const& dcmp_cutoff_this=dcmp_cutoff[dcmp_index];
			for(auto [earned,teams]:final_points_this){
				auto [cm,points]=earned;
				assert(points>=0);

				if(!cm && points<dcmp_cutoff_this.first) continue;
				if(points==dcmp_cutoff_this.first){
					teams*=(1-dcmp_cutoff_this.second);
				}

				for(unsigned i=0;i<teams;i++){
					int pts;
					if(dcmp.played){
						pts=points;
					}else{
						auto const& dists=dcmp.dists;
						auto f=dists.find(points);
						auto d=[&](){
							if(f==dcmp.dists.end()){
								/*PRINT(points);
								PRINT(keys(input.dcmp_distribution1));
								assert(0);*/
								auto m=max(keys(dists));
								if(points>m){
									auto f2=dists.find(m);
									assert(f2!=dists.end());
									return f2->second;
								}
								PRINT(points);
								PRINT(keys(dists));
								nyi
							}else{
								return f->second;
							}
						}();
						//pts=points+sample(input.dcmp_distribution1[points]);
						pts=points+sample(d);
					}
					post_dcmp_points[make_pair(0,pts)]++;
				}
			}
		}

		cmp_cutoff|=find_cutoff(post_dcmp_points,cmp_teams_left_out);
	}

	//map<pair<Point,Pr>,Pr> cutoff_pr=flat_map2(map_values(
	/*auto cutoff_pr=flat_map2(map_values(
		[=](auto x){ return (0.0+x)/iterations; },
		count(dcmp_cutoffs)
	));*/
	auto cutoff_pr=mapf(
		[=](auto const& dcmp_cutoffs_this){
			return flat_map2(map_values(
				[=](auto x){ return (0.0+x)/iterations; },
				count(dcmp_cutoffs_this)
			));
		},
		dcmp_cutoffs
	);

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
	//auto interesting_cutoffs_dcmp=interesting_cutoffs(cutoff_pr);
	auto interesting_cutoffs_dcmp=mapf(interesting_cutoffs,cutoff_pr);
	auto interesting_cutoffs_cmp=interesting_cutoffs(cmp_cutoff_pr);

	vector<Output_tuple> result;
	for(auto [team,team_data]:input.by_team){
		//probability that get in
		//subtract the cutoff pr
		auto [cm,team_pr,dcmp_home,already_earned]=input.by_team[team];
		double pr_make=0;
		double pr_miss=0;
		if(cm){
			pr_make=1;
			pr_miss=0;
		}else{
			for(auto [cutoff,c_pr]:cutoff_pr[dcmp_home]){
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
			return when_greater(team_pr,cutoff_pr[dcmp_home]);
		}();

		auto post_dcmp_dist=convolve(dcmp_entry_dist,input.dcmp.at(team_data.dcmp_home).dists);
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

		auto points_so_far=already_earned;
		vector<Point> cmp_interesting;
		for(auto [pr,pts]:interesting_cutoffs_cmp){
			cmp_interesting|=max(Point(0),Point(pts.first-points_so_far));
		}

		if(cm){
			assert(pr_make>.99);
			result|=Output_tuple(
				team,
				input.by_team[team].dcmp_home,
				1.0,std::array<Point,3>{0,0,0},
				cmp_make,std::array<Point,3>{cmp_interesting[0],cmp_interesting[1],cmp_interesting[2]}
			);
		}else{
			//PRINT(total);

			//PRINT(pr_make);
			//points needed to have 50% odds; 5% odds; 95% odds, or quartiles?
			vector<Point> interesting;
			for(auto [pr,pts]:interesting_cutoffs_dcmp[dcmp_home]){
				//cout<<pr<<":"<<max(0.0,pts-points_so_far)<<"\n";
				auto value=max(Point(0),Point(pts.first-points_so_far));
				interesting|=value;
			}
			assert(interesting.size()==3);

			result|=Output_tuple(
				team,
				input.by_team[team].dcmp_home,
				pr_make,std::array<Point,3>{interesting[0],interesting[1],interesting[2]},
				cmp_make,std::array<Point,3>{cmp_interesting[0],cmp_interesting[1],cmp_interesting[2]}
			);
		}
	}

	auto x=::mapf([](auto x){ return x.dcmp_make; },result);
	//PRINT(sum(x)); //this number should be really close to the number of slots available at the event.

	return Run_result{result,cutoff_pr,cmp_cutoff_pr};
}

