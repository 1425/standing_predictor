#include "decode.h"

tba::Team_key decode(std::string a,tba::Team_key const* b){
	return decode2(a,b);
}

tba::Date decode(std::string const& a,tba::Date const* b){
	return tba::decode(a,b);
}

size_t decode(std::string const& a,size_t const*){
	try{
		return stoi(a);
	}catch(...){
		assert(0);
	}
}

int decode(std::string const& a,int const*){
	return stoi(a);
}

bool decode(std::string const& a,bool const*){
	auto x=stoi(a);
	if(x>=0 && x<=1){
		return x;
	}
	assert(0);
}

double decode(std::string const& a,double const*){
	return stod(a);
}

