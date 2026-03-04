#ifndef ANNOTATED_COMPLEX_H
#define ANNOTATED_COMPLEX_H

#include "../tba/data.h"
#include "event_limits.h"
#include "print_r.h"
#include "event_categories.h"

namespace tba{
	struct Event;
	class District_key;
};

class TBA_fetcher;

template<typename T>
struct Event_annotated{
	tba::Event data;
	T extra;

	auto operator<=>(Event_annotated const&)const=default;
};

#define EVENT_ANNOTATED(X)\
	X(tba::Event,data)\
	X(T,extra)\

template<typename T>
PRINT_R_ITEM(Event_annotated<T>,EVENT_ANNOTATED)

template<typename T>
std::ostream& operator<<(std::ostream& o,Event_annotated<T> const& a){
	o<<"Event_annotated(";
	o<<a.data;
	o<<a.extra;
	return o<<")";
}

template<typename A1,typename A2>
struct District_cmp_complex_annotated{
	Event_annotated<A1> finals;
	std::vector<Event_annotated<A1>> divisions;
	A2 extra;

	auto operator<=>(District_cmp_complex_annotated const&)const=default;
};

template<typename A1,typename A2>
std::ostream& operator<<(std::ostream& o,District_cmp_complex_annotated<A1,A2> const& a){
	o<<"District_cmp_complex_annotated(";
	o<<a.finals;
	o<<a.divisions;
	o<<a.extra;
	return o<<")";
}

template<typename A1,typename A2,typename A3>
struct Event_categories_annotated{
	std::vector<Event_annotated<A1>> local;
	std::vector<District_cmp_complex_annotated<A1,A2>> dcmp;
	A3 extra;

	auto operator<=>(Event_categories_annotated const&)const=default;
};

template<typename A1,typename A2,typename A3>
std::ostream& operator<<(std::ostream& o,Event_categories_annotated<A1,A2,A3> const& a){
	o<<"Event_categories_annotated(";
	o<<a.local;
	o<<a.dcmp;
	o<<a.extra;
	return o<<")";
}

//to start with probably just want to annotate things with string to get pushed into the HTML.
using Annotated=Event_categories_annotated<
	Rank_status<Tournament_status>,
	Tournament_status,
	Rank_status<District_status>
>;
Annotated annotated(TBA_fetcher&,tba::District_key const&);

template<typename Func>
auto mapf_preserve(Func f,tba::Event const& a){
	return Event_annotated{a,f(a)};
}

template<typename Func>
auto mapf_preserve(Func f,District_cmp_complex const& a){
	auto m1=mapf_preserve(f,a.finals);
	auto m2=mapf_preserve(f,a.divisions);
	return District_cmp_complex_annotated{
		m1,m2,f(m1,m2)
	};
}

template<typename Func>
auto mapf_preserve(Func f,Event_categories const& in){
	auto a=mapf(f,in.local);
	auto b=mapf(f,in.dcmp);
	return Event_categories_annotated{
		a,b,f(a,b)
	};
}

int annotated_complex_demo(TBA_fetcher&);

#endif
