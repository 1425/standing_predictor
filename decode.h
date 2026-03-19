#ifndef DECODE_H
#define DECODE_H

#include "../tba/data.h"

tba::Team_key decode(std::string a,tba::Team_key const* b);
tba::Date decode(std::string const& a,tba::Date const* b);
size_t decode(std::string const& a,size_t const*);
int decode(std::string const& a,int const*);
bool decode(std::string const& a,bool const*);
double decode(std::string const& a,double const*);

#endif
