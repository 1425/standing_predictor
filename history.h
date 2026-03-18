#ifndef HISTORY_H
#define HISTORY_H

#include<map>
#include "probability.h"

class TBA_fetcher;

std::map<std::pair<int,bool>,std::map<int,std::map<int,Pr>>> transition_table(TBA_fetcher&);

#define HISTORY_ITEM(X)\
	X(tba::Team_key,team)\
	X(tba::Year,year)\
	X(tba::Date,date)\
	X(size_t,appearances)\
	X(Pr,dcmp_pr)\
	X(Pr,cmp_pr)

STRUCT_DECLARE(History_item,HISTORY_ITEM)

std::vector<History_item> read_history(TBA_fetcher&);

int history_demo(TBA_fetcher&);

tba::Team_key decode(std::string a,tba::Team_key const* b);
tba::Date decode(std::string const& a,tba::Date const* b);
size_t decode(std::string const& a,size_t const*);
int decode(std::string const& a,int const*);
bool decode(std::string const& a,bool const*);
double decode(std::string const& a,double const*);

#endif
