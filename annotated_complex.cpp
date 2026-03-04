#include "annotated_complex.h"
#include "event_limits.h"
#include "tba.h"

using namespace std;

/*template<typename T>
void print_r(int n,Event_annotated<T> const& a){
	indent(n);
	cout<<"Event_annotated\n";
	n++;

	indent(n);
	cout<<"data\n";
	print_r(n+1,a.data.key);//for the sake of space

	indent(n);
	cout<<"extra\n";
	print_r(n+1,a.extra);
}*/

template<typename A,typename B>
void print_r(int n,District_cmp_complex_annotated<A,B> const& a){
	indent(n);
	cout<<"District_cmp_complex_annotated\n";
	n++;

	indent(n);
	cout<<"finals\n";
	print_r(n+1,a.finals);

	indent(n);
	cout<<"divisions\n";
	print_r(n+1,a.divisions);

	indent(n);
	cout<<"extra\n";
	print_r(n+1,a.extra);
}

template<typename A,typename B,typename C>
void print_r(int n,Event_categories_annotated<A,B,C> const& a){
	indent(n);
	cout<<"Event_categories_annotated\n";
	n++;

	indent(n);
	cout<<"local\n";
	print_r(n+1,a.local);

	indent(n);
	cout<<"dcmp\n";
	print_r(n+1,a.dcmp);

	indent(n);
	cout<<"extra\n";
	print_r(n+1,a.extra);
}

template<typename T>
auto extras(T const& a){
	return mapf([](auto const& x){ return x.extra; },a);
}

struct Walker{
	TBA_fetcher &f;

	auto operator()(tba::Event const& a){
		return Event_annotated{a,event_limits(f,a.key)};
	}

	auto operator()(std::vector<tba::Event> const& a){
		return mapf([&](auto const& x){ return (*this)(x); },a);
	}

	auto operator()(District_cmp_complex const& a){
		auto finals=(*this)(a.finals);
		auto divisions=(*this)(a.divisions);

		auto calc=[&](){
			if(!divisions.empty()){
				auto last_division_status=min(mapf([&](auto x){ return x.extra.status; },divisions));
				if(last_division_status!=Tournament_status::COMPLETE){
					return last_division_status;
				}
			}
			return finals.extra.status;
		};

		return District_cmp_complex_annotated{finals,divisions,calc()};
	}

	template<typename T1,typename T2>
	std::variant<
		District_status_future,
		District_status_dcmp_in_progress,
		District_status_complete
	> dcmp_status(std::vector<District_cmp_complex_annotated<T1,T2>> const& a){
		//this basically duplidates dcmp_status from event_limits.cpp
		if(a.empty()){
			return District_status_complete();
		}
		//auto m=map_preserve([&](auto x){return 
		auto opts=to_set(mapf([](auto const& x){ return x.extra; },a));
		if(opts==std::set<Tournament_status>{Tournament_status::FUTURE}){
			return District_status_future();
		}
		if(opts==std::set<Tournament_status>{Tournament_status::COMPLETE}){
			return District_status_complete();
		}
		return District_status_dcmp_in_progress([&](){
			std::map<Tournament_status,std::vector<tba::Event_key>> r;
			for(auto const& x:a){
				auto event=x.finals.data.key;
				auto result=x.extra;
				r[result]|=event;
			}
			return r;
		}());
	}

	template<typename T1,typename T2>
	auto operator()(
		std::vector<Event_annotated<T1>> const& locals,
		std::vector<District_cmp_complex_annotated<T1,T2>> const& dcmp
	){
		(void)dcmp;

		//this is basically duplicating district_limits()
		auto local_status=to_set(mapf([](auto x){ return x.extra.status; },locals));

		Rank_status<District_status> r;
		map<tba::Team_key,int> plays;

		for(auto const& here:extras(locals)){
			for(auto [k,v]:here.by_team){
				auto &p=plays[k];
				if(p<2){
					r.by_team[k]+=v;
				}
				p++;
			}
			r.unclaimed+=here.unclaimed;
		}

		r.status=[&]()->District_status{
			if(local_status==set<Tournament_status>{Tournament_status::FUTURE}){
				return District_status_future();
			}
			if(local_status!=set<Tournament_status>{Tournament_status::COMPLETE}){
				auto k2=dict(mapf(
					[](auto const& x){ return make_pair(x.data.key,x.extra.status); },
					locals
				));
				return District_status_locals_in_progress(invert(k2));
			}
			auto d=dcmp_status(dcmp);
			return std::visit([](auto x){ return District_status(x); },d);
		}();
		return r;
	}
};

Event_categories_annotated<
	Rank_status<Tournament_status>,
	Tournament_status,
	Rank_status<District_status>
> annotated(TBA_fetcher &f,tba::District_key const& a){
	return mapf_preserve(Walker{f},categorize_events(f,a));
}

int annotated_complex_demo(TBA_fetcher& f){
	for(auto district:districts(f)){
		annotated(f,district);
	}

	auto a=annotated(f,tba::District_key("2026pnw"));
	print_r(a);
	return 0;
}

