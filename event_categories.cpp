#include "event_categories.h"
#include "io.h"
#include "tba.h"

PRINT_STRUCT(District_cmp_complex,DISTRICT_CMP_COMPLEX)

//maybe this should move to tba.h.
bool complete(TBA_fetcher &f,tba::Event const& a){
	return complete(f,a.key);
}

bool complete(TBA_fetcher &f,District_cmp_complex const& a){
	return complete(f,a.finals);
}

PRINT_STRUCT(Event_categories,EVENT_CATEGORIES)

Event_categories categorize_events(TBA_fetcher &f,tba::District_key const& district){
	auto e=events(f,district);
	auto g=GROUP(event_type,e);

	Event_categories r;
	r.local=g[tba::Event_type::DISTRICT];
	std::sort(r.local.begin(),r.local.end(),[](auto const& a,auto const& b){ return a.start_date<b.start_date; });

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


