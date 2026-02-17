#include "ca.h"
#include<fstream>
#include<set>
#include "../tba/util.h"
#include "../tba/data.h"
#include "util.h"
#include "zipcodes.h"
#include "set.h"
#include "io.h"
#include "vector_void.h"
#include "../tba/tba.h"
#include "tba.h"

using namespace std;

std::ostream& operator<<(std::ostream& o,Zipcode const& a){
	return o<<a.data;
}

bool City::operator==(std::string const& s)const{
	return data==s;
}

bool City::operator==(City const& a)const{
	return data==a.data;
}

std::ostream& operator<<(std::ostream& o,City const& a){
	return o<<a.data;
}

std::ostream& operator<<(std::ostream& o,California_region a){
	#define X(A) if(a==California_region::A) return o<<""#A;
	X(NORTH)
	X(SOUTH)
	#undef X
	assert(0);
}

#define ZIP_CODE_DATA(X)\
	X(Zipcode,zipcode)\
	X(City,town)\
	X(std::string,county)

TBA_MAKE_INST(Zip_code_data,ZIP_CODE_DATA)

std::ostream& operator<<(std::ostream& o,Zip_code_data const& a){
	o<<"("<<a.zipcode<<" "<<a.town<<" "<<a.county<<")";
	return o;
}

auto read_inner(){
	return mapf(
		[](auto x){
			return Zip_code_data(
				Zipcode(get<0>(x)),
				City(get<1>(x)),
				get<2>(x)
			);
		},
		zipcode_data()
	);
}

auto const& read(){
	static auto r=read_inner();
	return r;
}

auto counties(){
	auto m=mapf([](auto x){ return x.county; },read());
	auto s=to_set(m);
	return s;
}

std::set<std::string> const& socal(){
	static const std::set<std::string> r{
		"San Luis Obispo","Kern","San Bernardino",
		"Santa Barbara","Ventura","Los Angeles","Orange",
		"Riverside","San Diego","Imperial"
	};
	return r;
}

std::set<std::string> const& norcal(){
	static const auto r=counties()-socal();
	return r;
}

using County=std::string;

California_region california_region(County const& county){
	if(norcal().count(county)){
		return California_region::NORTH;
	}
	if(socal().count(county)){
		return California_region::SOUTH;
	}
	assert(0);
}

California_region california_region(Zipcode const& zipcode){
	auto f=filter([=](auto const& x){ return x.zipcode==zipcode; },read());
	assert(f.size()==1);
	auto county=f[0].county;
	return california_region(county);
}

California_region california_region(City const& city){
	//A city is sometimes in more than one county, but both counties are always in the same region.
	auto f=filter([=](auto const& x){ return x.town==city; },read());
	auto m=to_set(mapf([](auto x){ return x.county; },f));
	auto m2=to_set(mapf([](auto x){ return california_region(x); },m));
	assert(m2.size()==1);
	return *begin(m2);
}

California_region california_region(tba::Team const& team){
	if(team.postal_code){
		return california_region(Zipcode(*team.postal_code));
	}
	if(team.city){
		return california_region(City(*team.city));
	}
	assert(0);
}

auto california_region(tba::Event const& event){
	if(event.postal_code){
		return california_region(Zipcode(*event.postal_code));
	}
	PRINT(event);
	nyi
}

Dcmp_home calc_dcmp_home(TBA_fetcher &fetcher,tba::Team_key const& team_key){
	auto t=team(fetcher,team_key);
	if(t.state_prov!="California"){
		return 0;
	}
	auto c=california_region(t);
	return Dcmp_home(int(c));
}

