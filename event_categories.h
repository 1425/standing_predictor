#ifndef EVENT_CATEGORIES_H
#define EVENT_CATEGORIES_H

#include "util.h"
#include "../tba/data.h"

class TBA_fetcher;

#define DISTRICT_CMP_COMPLEX(X)\
	X(tba::Event,finals)\
	X(std::vector<tba::Event>,divisions)\

//std::vector<Event> divisions; //may be empty
//Event finals; //won't have qual matches and picks if divisions exist
STRUCT_DECLARE(District_cmp_complex,DISTRICT_CMP_COMPLEX)

#define EVENT_CATEGORIES(X)\
	X(std::vector<tba::Event>,local)\
	X(std::vector<District_cmp_complex>,dcmp)\
	X(std::vector<District_cmp_complex>,cmp)\

//std::vector<Event> local;//sorted by date
//std::vector<District_cmp_complex> dcmp;//sorted by Dcmp_index?
STRUCT_DECLARE(Event_categories,EVENT_CATEGORIES)

Event_categories categorize_events(TBA_fetcher&,tba::District_key const&);

#endif
