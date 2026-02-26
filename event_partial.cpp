#include "event_partial.h"
#include "skill_opr.h"
#include "print_r.h"
#include "tba.h"
#include "award_limits.h"
#include "../tba/tba.h"
#include "map.h"
#include "rand.h"
#include "event_limits.h"
#include "run.h"
#include "event.h"

using namespace std;

template<typename Func,typename T,size_t N>
auto filter(Func f,tba::vector_fixed<T,N> const& a){
	tba::vector_fixed<T,N> r;
	for(auto const& x:a){
		if(f(x)){
			r|=x;
		}
	}
	return r;
}

template<typename Func,typename T,size_t N>
auto mapf(Func f,tba::vector_fixed<T,N> const& a){
	using E=decltype(f(a[0]));
	tba::vector_fixed<E,N> r;
	for(auto const& x:a){
		r|=f(x);
	}
	return r;
}

template<typename A,typename B,typename C,size_t N>
auto group(tba::vector_fixed<std::variant<A,B,C>,N> const& a){
	std::tuple<
		tba::vector_fixed<A,N>,
		tba::vector_fixed<B,N>,
		tba::vector_fixed<C,N>
	> r;
	for(auto const& elem:a){
		if(std::holds_alternative<A>(elem)){
			get<0>(r)|=get<A>(elem);
		}else if(std::holds_alternative<B>(elem)){
			get<1>(r)|=get<B>(elem);
		}else if(std::holds_alternative<C>(elem)){
			get<2>(r)|=get<C>(elem);
		}else{
			assert(0);
		}
	}
	return r;
}

template<typename T,size_t N>
auto take(size_t n,tba::vector_fixed<T,N> const& a){
	tba::vector_fixed<T,N> r;
	for(auto i:range(min(n,a.size()))){
		r|=a[i];
	}
	return r;
}

template<typename K,typename V>
K min_key(std::map<K,V>);

template<typename K,typename V>
K min_key(flat_map2<K,V> const& a){
	assert(!a.empty());
	return (*a.begin()).first;
}

#define X(A) std::ostream& operator<<(std::ostream& o,Team_event_status_##A const& a){\
	return o<<""#A<<"("<<a.data<<")";\
}
X(rank)
X(post_rank)
X(post_pick)
X(post_elims)
#undef X

Team_event_status_rank rand(Team_event_status_rank const*){
	return Team_event_status_rank(rand((Interval<Point>*)0));
}

Team_event_status_post_rank rand(Team_event_status_post_rank const*){
	return Team_event_status_post_rank(rand((Point*)0));
}

Team_event_status_post_pick rand(Team_event_status_post_pick const*){
	return Team_event_status_post_pick(rand((Point*)0));
}

Team_event_status_post_elims rand(Team_event_status_post_elims const*){
	return Team_event_status_post_elims(rand((Point*)0));
}


PRINT_STRUCT(Event_partial,EVENT_PARTIAL)
PRINT_R_ITEM(Event_partial,EVENT_PARTIAL)

bool district_local(TBA_fetcher &f,tba::Event_key a){
	return event(f,a).event_type==tba::Event_type::DISTRICT;
}

void ranks_partial(TBA_fetcher &f){
	(void)f;
	//assume uniform among the possible limits and then look at the distribution for the specific
	//number

	//at some point, you go from the previous numbers being the most useful to what's going on at this event
	//is more information-dense how much of the event has to happen before that is true?
	//could find this empirically

	nyi
}

Team_dist sum(std::vector<Team_dist> const& a){
	Team_dist r;
	for(auto const& x:a){
		for(auto [k,v]:x){
			r[k]+=v;
		}
	}
	return r;
}

Team_dist mean(std::vector<Team_dist> const& a){
	assert(!a.empty());
	auto s=sum(a);
	return map_values([=](auto x){ return x/a.size(); },s);
}

template<typename K,typename V>
auto get_closest(map_auto<K,V> const& a,K k){
	assert(a.size());

	if(k>max(keys(a))){
		auto k2=max(keys(a));
		return a[k2];
	}
	if(k<min(keys(a))){
		auto k2=min(keys(a));
		return a[k2];
	}

	for(auto i:range(300)){
		auto m=maybe_get(a,k+i);
		if(m) return *m;

		auto m2=maybe_get(a,k-i);
		if(m2){
			return *m2;
		}
	}
	assert(0);
}

Team_dist Event_partial::operator[](Team_event_status_rank const& a)const{
	//return mean(mapf([&](auto x){ return post_rank[x]; },range(a.data)));
	auto x=nonempty(mapf([&](auto x){ return maybe_get(post_rank,x); },range(a.data)));
	if(x.size()){
		return mean(x);
	}
	return get_closest(post_rank,a.data.min);
}

Team_dist Event_partial::operator[](Team_event_status_post_rank const& a)const{
	return get_closest(post_rank,a.data);
}

Team_dist Event_partial::operator[](Team_event_status_post_pick const& a)const{
	return get_closest(post_pick,a.data);
}

Team_dist Event_partial::operator[](Team_event_status_post_elims const& a)const{
	return get_closest(post_elims,a.data);
}

Team_dist Event_partial::operator[](Team_event_status const& a)const{
	return std::visit([&](auto x){ return (*this)[x]; },a);
}

Event_partial event_partial(TBA_fetcher &f){
	/* for each event:
	 * after each of the stages, given how each team is doing, see how they do at the event overall
	 *
	 * The stages:
	 * 0: nothing has been played yet -> ignore this because handled in other ways
	 * 1/2: might be able to do something after X number of matches played or X% of matches played
	 * 1: post-qual
	 * 2: post-alliance selection
	 * 3: post elim matches
	 * 4: post awards -> ignore this because it's handled by just looking at the total
	 *
	 * 1: measure is rank? (or rank pts?)
	 * 2: measure is total pts so far
	 * 3: measure is total pts so far
	 *
	 * for all of them the output should be point dist & chairmans odds
	*/

	using Target=Point; //TODO: Put in chairmans odds here, so you'd have Rank_value

	multiset<pair<Point,Target>> found1,found2,found3;

	for(auto district:districts(f)){
		auto d=tba::district_rankings(f,district);
		//Note: Need to filter out in-progress events here
		//Also, need to filter out district championships, or at least categorize them seperately!
		//Also, should do some flattening to avoid very low probabilities
		if(!d) continue;
		for(auto ranking:*d){
			for(auto event:ranking.event_points){
				if(!district_local(f,event.event_key)){
					continue;
				}

				//TODO: Calc whether won chairmans here.
				auto target=event.total;
				found1|=make_pair<Point,Target>(event.qual_points,target);
				
				found2|=make_pair(event.qual_points+event.alliance_points,target);

				found3|=make_pair(
					event.qual_points+event.alliance_points+event.elim_points,
					target
				);
			}
		}
	}

	auto make_dists=[](auto x){
		auto g=GROUP(first,x);
		static constexpr auto MIN_SAMPLE_SIZE=100;
		assert(x.size()>MIN_SAMPLE_SIZE);
		auto v=MAP_VALUES(seconds,g);

		map<Point,flat_map2<Point,Pr>> r;
		for(auto k:keys(v)){
			multiset<Point> found;
			//Note that this will sample the exact value twice.
			for(int i=0;i<100 && found.size()<MIN_SAMPLE_SIZE;i++){
				found|=v[k-i];
				found|=v[k+i];
			}
			r[k]=to_dist(found);
		}
		return r;
		//return map_values([](auto x){ return to_dist(seconds(x)); },g);
	};

	return Event_partial{
		make_dists(found1),
		make_dists(found2),
		make_dists(found3)
	};
}

struct Future{
	auto operator<=>(Future const&)const=default;
};

std::ostream& operator<<(std::ostream& o,Future){
	return o<<"Future";
}

template<typename A,typename B,typename C>
auto group(std::vector<std::variant<A,B,C>> a){
	std::tuple<std::vector<A>,std::vector<B>,std::vector<C>> r;
	for(auto elem:a){
		if(std::holds_alternative<A>(elem)){
			get<0>(r)|=get<A>(elem);
		}else if(std::holds_alternative<B>(elem)){
			get<1>(r)|=get<B>(elem);
		}else if(std::holds_alternative<C>(elem)){
			get<2>(r)|=get<C>(elem);
		}else{
			assert(0);
		}
	}
	return r;
}

template<typename T>
auto seconds(Interval<T> a){
	//this only works if you make some assumptions about the inputs.
	//but it does check if this goes wrong so you'll find out.
	return Interval{a.min.second,a.max.second};
}

Point median(Team_dist a){
	assert(!a.empty());
	auto target=sum(values(a))/2;
	double total=0;
	for(auto [k,v]:a){
		total+=v;
		if(total>=target){
			return k;
		}
	}
	assert(0);
}

using Event=tba::Event;

#define DISTRICT_CMP_COMPLEX(X)\
	X(Event,finals)\
	X(std::vector<Event>,divisions)\

//std::vector<Event> divisions; //may be empty
//Event finals; //won't have qual matches and picks if divisions exist
STRUCT_DECLARE(District_cmp_complex,DISTRICT_CMP_COMPLEX)

PRINT_STRUCT(District_cmp_complex,DISTRICT_CMP_COMPLEX)

bool complete(TBA_fetcher &f,tba::Event const& a){
	return complete(f,a.key);
}

bool complete(TBA_fetcher &f,District_cmp_complex const& a){
	return complete(f,a.finals);
}

#define EVENT_CATEGORIES(X)\
	X(std::vector<Event>,local)\
	X(std::vector<District_cmp_complex>,dcmp)

//std::vector<Event> local;//sort by date?
//std::vector<District_cmp_complex> dcmp;//sorted by Dcmp_index?
STRUCT_DECLARE(Event_categories,EVENT_CATEGORIES)

PRINT_STRUCT(Event_categories,EVENT_CATEGORIES)

tba::Event_type event_type(tba::Event const& a){
	return a.event_type;
}

Event_categories categorize_events(TBA_fetcher &f,tba::District_key const& district){
	auto e=events(f,district);
	auto g=GROUP(event_type,e);

	Event_categories r;
	r.local=g[tba::Event_type::DISTRICT];
	r.dcmp=mapf(
		[](auto x){ return District_cmp_complex(x,{}); },
		g[tba::Event_type::DISTRICT_CMP]
	);

	auto find=[&](tba::Event_key a)->District_cmp_complex&{
		for(auto &d:r.dcmp){
			if(d.finals.key==a){
				return d;
			}
		}
		assert(0);
	};

	for(auto event:g[tba::Event_type::DISTRICT_CMP_DIVISION]){
		assert(event.parent_event_key);
		auto &d=find(*event.parent_event_key);
		d.divisions|=event;
	}
	return r;
}

auto event_type(TBA_fetcher &f,tba::Event_points const& a){
	return event_type(f,a.event_key);
}

Run_input read_status(TBA_fetcher &f,tba::District_key const& district){
	//go look at all the event statuses 
	//and then go through the standings looking at teams with knowledge of the status of the events

	const auto event_partial1=event_partial(f);

	/*map<tba::Event_key,Rank_status<Tournament_status>> event_limits1;
	for(auto event:district_events_keys(f,district)){
		event_limits1[event]=event_limits(f,event);
	}*/

	auto event_limits1=dict(mapf(
		[&](auto x){ return make_pair(x,event_limits(f,x)); },
		district_events_keys(f,district)
	));

	auto team_event_status=[=](tba::Team_key team,tba::Event_points event)->std::variant<Team_dist,Future,std::nullopt_t>{
		//The options are:
		//1) future local
		//2) team_dist
		//3) wrong type of event
		//eventually may want this to tell you if awards won.

		auto f=event_limits1.find(event.event_key);
		if(f==event_limits1.end()){
			//then this was the dcmp or something
			return std::nullopt;
		}

		auto limits=f->second;

		switch(limits.status){
			case Tournament_status::FUTURE:
				return Future();
			case Tournament_status::QUAL_MATCHES_IN_PROGRESS:
				return event_partial1[Team_event_status_rank(seconds(limits.by_team[team]))];
			case Tournament_status::QUAL_MATCHES_COMPLETE:
			case Tournament_status::PICKING_IN_PROGRESS:
				return event_partial1[Team_event_status_post_rank(event.qual_points)];
			case Tournament_status::PICKING_COMPLETE:
			case Tournament_status::ELIMINATIONS_IN_PROGRESS:
				return event_partial1[Team_event_status_post_pick(
					event.qual_points+event.alliance_points
				)];
			case Tournament_status::ELIMINATIONS_COMPLETE:
			case Tournament_status::AWARDS_IN_PROGRESS:
				return event_partial1[Team_event_status_post_elims(
					event.qual_points+event.alliance_points+event.elim_points
				)];
			case Tournament_status::COMPLETE:{
				Team_dist r;
				r[event.total]=1;
				return r;
			}
			default:
				assert(0);
		}
	};

	Skill_estimates skill=calc_skill(f,district);

	auto team_dist_pre_dcmp=[&](auto team_info)->Team_dist{
		auto found=mapf(
			[&](auto x){ return team_event_status(team_info.team_key,x); },
			team_info.event_points
		);

		auto [known,futures,ignore]=group(found);
		known=take(2,known);
		auto future=futures.size();
		if(known.size()==0){
			return skill.pre_dcmp[team_info.team_key];
		}else if(known.size()==1){
			if(future==0){
				return known[0];
			}else{
				auto k=known[0];
				return convolve(k,skill.second_event[median(k)]);
			}
		}else if(known.size()==2){
			return convolve(known[0],known[1]);
		}
		assert(0);
	};

	auto dl=district_limits(f,district);

	auto team_dist=[&](auto team_info)->Team_dist{
		auto x=team_dist_pre_dcmp(team_info);
		
		auto found=filter(
			[&](auto x){ return event_type(f,x)!=tba::Event_type::DISTRICT; },
			team_info.event_points
		);

		assert(found.size()<=1);

		//TODO: Need to have a per-team DCMP done?
		//rather should have a per-dcmp dcmp-done.
		/*if(dl.status==District_status::COMPLETE){
			nyi
		}else{
			nyi
		}*/
		return x;
	};

	auto d=tba::district_rankings(f,district);
	assert(d);

	Run_input r;
	r.worlds_slots=worlds_slots(district);

	auto cat=categorize_events(f,district);

	for(auto [i,size]:enumerate(dcmp_size(district))){
		Dcmp_data d;
		d.size=size;
		d.played=complete(f,cat.dcmp.at(i));
		if(!d.played){
			d.dists=skill.at_dcmp;
		}else{
			//whatever number of points you have expect to get 0 more.
			for(auto i:range(500)){
				Team_dist n;
				n[0]=1;
				d.dists[i]=n;
			}
		}
		r.dcmp|=d;
	}

	auto cm=chairmans_winners(f,district);

	for(auto team_info:*d){
		auto team=team_info.team_key;
		//x.rookie_bonus;
		Team_status &t=r.by_team[team];
		t.district_chairmans=cm.count(team);
		t.point_dist=team_dist(team_info)+team_info.rookie_bonus;
		t.dcmp_home=calc_dcmp_home(f,team);

		t.already_earned=min_key(t.point_dist);
	}

	auto dcmp_options=to_set(mapf([](auto x){ return x.dcmp_home; },values(r.by_team)));

	return r;
}

int event_partial_demo(TBA_fetcher &f){
	for(auto district:districts(f)){
		categorize_events(f,district);
	}

	auto e=event_partial(f);
	//print_r(e);

	read_status(f,tba::District_key("2025pnw"));

	if(0){
		#define X(A,B) cout<<""#B<<"\n"; for(auto [k,v]:e.B){ cout<<"\t"<<k<<"\t"<<quartiles(v)<<"\n"; }
		EVENT_PARTIAL(X)
		#undef X
	}

	for(auto _:range(1000)){
		e[rand((Team_event_status*)0)];
	}
	return 0;
}
