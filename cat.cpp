#include "cat.h"
#include<fstream>
#include "util.h"
#include "vector.h"
#include "vector_void.h"
#include "io.h"
#include "map.h"
#include "tba.h"

using namespace std;

struct Any{
	auto operator<=>(Any const&)const=default;
};

std::ostream& operator<<(std::ostream& o,Any){
	return o<<"*";
}

std::ostream& operator<<(std::ostream& o,Now const&){
	return o<<"Now";
}

std::ostream& operator<<(std::ostream& o,Static const&){
	return o<<"Static";
}

std::optional<Cat_result> cat(std::string const& url){
	const string p="https://www.thebluealliance.com/api/v3/";
	assert(prefix(url,p));
	auto rest=url.substr(p.size(),url.size());
	if(rest[0]=='/'){
		rest=skip(1,rest);
	}
	auto sp=split(rest,'/');

	if(sp.size()==2 && sp[0]=="districts"){
		return Static{};
	}

	if(sp.size()==3 && sp[0]=="district" && sp[2]=="events"){
		return Static{};
	}

	static const tba::Year current_year(2026);

	if(sp.size()==3 && sp[0]=="event" && sp[2]=="matches"){
		auto event=tba::Event_key(sp[1]);
		if(year(event)<current_year){
			return Static{};
		}else{
			return Now();
		}
	}

	if(
		sp.size()==3 && 
		sp[0]=="event" && 
		(
		 	sp[2]=="alliances"
			|| sp[2]=="rankings"
		)
	){
		tba::Event_key event(sp[1]);
		if(year(event)<current_year){
			return Static{};
		}else{
			return Now();
		}
	}

	if(sp.size()==4 && sp[0]=="event" && sp[2]=="teams" && sp[3]=="keys"){
		return Static();
	}

	if(sp.size()==2 && sp[0]=="event"){
		return Static();
	}

	if(sp.size()==4 && sp[0]=="district" && sp[2]=="events" && sp[3]=="keys"){
		tba::Event_key event(sp[1]);
		if(year(event)<current_year){
			return Static();
		}else{
			return Now();
		}
	}

	if(sp.size()==3 && sp[0]=="district" && sp[2]=="rankings"){
		tba::District_key d(sp[1]);
		if(year(d)<current_year){
			return Static();
		}else{
			return Now();
		}
	}

	if(sp.size()==3 && sp[0]=="event" && sp[2]=="awards"){
		tba::Event_key event(sp[1]);
		if(year(event)<current_year){
			return Static();
		}else{
			return Now();
		}
	}

	if(sp.size()==5 && sp[0]=="team" && sp[2]=="events" && sp[4]=="keys"){
		return Static();
	}

	if(sp.size()==3 && sp[0]=="district" && sp[2]=="history"){
		return Now();
	}

	if(sp.size()==1 && sp[0]=="status"){
		//correct for the stuff we're doing, anyway
		//for other uses, this would definitely be a problem.
		return Static();
	}

	if(sp.size()==3 && sp[0]=="district" && sp[2]=="dcmp_history"){
		return Now();
	}

	if(sp.size()==3 && sp[0]=="teams"){
		return Static();
	}

	if(sp.size()==4 && sp[0]=="team" && sp[2]=="events"){
		//1=team
		tba::Year year(stoi(sp[3]));
		if(year<current_year){
			return Static();
		}else{
			return Static();
		}
	}

	if(sp.size()==3 && sp[0]=="district" && sp[2]=="teams"){
		return Static();
	}

	if(sp.size()==4 && sp[0]=="district" && sp[2]=="events" && sp[3]=="simple"){
		return Static();
	}

	if(sp.size()==3 && sp[0]=="event" && sp[2]=="district_points"){
		tba::Event_key event(sp[1]);
		if(year(event)<current_year){
			return Static();
		}else{
			return Now();
		}
	}

	if(sp.size()==3 && sp[0]=="events" && sp[2]=="simple"){
		return Static();
	}
	if(sp.size()==3 && sp[0]=="event" && sp[2]=="oprs"){
		tba::Event_key event(sp[1]);
		if(year(event)<current_year){
			return Static();
		}else{
			return Now();
		}
	}

	if(sp.size()==4 && sp[0]=="district" && sp[2]=="teams" && sp[3]=="keys"){
		return Static();
	}

	if(sp.size()==2 && sp[0]=="events"){
		return Static();
	}

	#if 0
	PRINT(sp);
	nyi
	#else
	return std::nullopt;
	#endif
}

int cat_demo(){
	ifstream f("url.txt");
	string s;
	std::vector<string> v;
	while(getline(f,s)){
		v|=s;
	}

	string p="https://www.thebluealliance.com/api/v3/";
	auto m=to_set(mapf(
		[=](auto x){
			assert(prefix(x,p));
			auto s=x.substr(p.size(),1000);
			if(s[0]=='/'){
				s=s.substr(1,1000);
			}
			return s;
		},
		v
	));

	auto m2=MAP([](auto x){ return split(x,'/'); },m);

	auto g=GROUP(car,m2);

	using Item=std::variant<std::string,Any>;
	using Pattern=std::vector<Item>;

	vector<Pattern> vp;

	vp|=Pattern{"district",Any{},"advancement"};
	vp|=Pattern{"district",Any{},"awards"};
	vp|=Pattern{"district",Any{},"dcmp_history"};
	vp|=Pattern{"district",Any{},"events"};
	vp|=Pattern{"district",Any{},"events","keys"};
	vp|=Pattern{"district",Any{},"events","simple"};
	vp|=Pattern{"district",Any{},"history"};
	vp|=Pattern{"district",Any{},"insights"};
	vp|=Pattern{"district",Any{},"rankings"};
	vp|=Pattern{"district",Any{},"teams"};
	vp|=Pattern{"district",Any{},"teams","keys"};
	vp|=Pattern{"district",Any{},"teams","simple"};
	vp|=Pattern{"districts"};
	vp|=Pattern{"event",Any{}};
	for(auto x:{"alliances","awards","district_points","insights","matches"}){
		(void)x;
		nyi//vp|=Pattern("event",Any{},x);
	}
	
	//anyway, can put all the patterns in and pull the calls back out
	//need to have a function of URL->expected update speed
	//expected update categories: 1) annual 2) continuous
	//and for some of the things it's going to be something like 

	for(auto [k,v]:g){
		PRINT(k);
		auto v2=to_set(mapf([](auto x){ return skip(2,x); },v));
		if(v2.size()<20){
			print_lines(v2);
		}else{
			nyi
		}
	}
	return 0;
}
