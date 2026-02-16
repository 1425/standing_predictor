#include "int_limited.h"

Int_limited<0,3> sum(std::tuple<bool,bool,bool> const& a){
	Int_limited<0,3> r;
	#define X(N) if(std::get<N>(a)) r++;
	X(0) X(1) X(2)
	#undef X
	return r;
}
