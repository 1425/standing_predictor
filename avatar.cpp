#include "avatar.h"
#include<sstream>
#include "util.h"
#include "plot.h"
#include "../tba/data.h"
#include "io.h"

std::string avatar(tba::Team_key const& team){
	auto path=[&](){
		std::stringstream ss;
		ss<<"data/avatar/2026/"<<team<<".png";
		return ss.str();
	}();
	std::string s;
	try{
		s=slurp(path);
	}catch(File_not_found const&){
	}
	if(s.empty()){
		return "";
	}
	return png_tag(s,::as_string(team));
}
