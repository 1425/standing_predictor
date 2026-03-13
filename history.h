#ifndef HISTORY_H
#define HISTORY_H

#include<map>
#include "probability.h"

class TBA_fetcher;

std::map<std::pair<int,bool>,std::map<int,std::map<int,Pr>>> read_dist();

#define HISTORY_ITEM(X)\
	X(tba::Team_key,team)\
	X(tba::Year,year)\
	X(tba::Date,date)\
	X(size_t,appearances)\
	X(Pr,dcmp_pr)\
	X(Pr,cmp_pr)

STRUCT_DECLARE(History_item,HISTORY_ITEM)

std::vector<History_item> read_history();

int history_demo(TBA_fetcher&);

#endif
