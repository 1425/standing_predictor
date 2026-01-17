#include "ca.h"
#include<fstream>
#include<set>
#include "../tba/util.h"
#include "../tba/data.h"
#include "util.h"
#include "zipcodes.h"
#include "set.h"

using namespace std;

std::ostream& operator<<(std::ostream& o,Zipcode const& a){
	return o<<a.data;
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

/*template<typename Func,typename T>
auto filter(Func f,std::vector<T> const& a){
	std::vector<T> r;
	for(auto const& x:a){
		if(f(x)){
			r|=x;
		}
	}
	return r;
}*/

/*template<typename T>
std::ostream& operator<<(std::ostream& o,std::set<T> const& a){
	o<<"{ ";
	for(auto const& x:a){
		o<<x<<" ";
	}
	return o<<"}";
}*/

/*template<typename T>
std::set<T> operator|(std::set<T> a,std::set<T> const& b){
	for(auto const& x:b){
		a.insert(x);
	}
	return a;
}*/

/*template<typename T>
std::set<T> operator-(std::set<T> a,std::set<T> const& b){
	for(auto const& x:b){
		a.erase(x);
	}
	return a;
}*/

/*template<typename T>
auto to_set(std::vector<T> const& a){
	return std::set<T>{a.begin(),a.end()};
}*/

#define ZIP_CODE_DATA(X)\
	X(Zipcode,zipcode)\
	X(City,town)\
	X(std::string,county)

TBA_MAKE_INST(Zip_code_data,ZIP_CODE_DATA)

std::ostream& operator<<(std::ostream& o,Zip_code_data const& a){
	o<<"("<<a.zipcode<<" "<<a.town<<" "<<a.county<<")";
	return o;
}

/*auto split(std::string const& s,char delim){
	std::vector<std::string> r;
	stringstream ss;
	for(auto x:s){
		if(x==delim){
			r|=ss.str();
			ss.str("");
		}else{
			ss<<x;
		}
	}
	r|=ss.str();
	return r;
}*/

std::string data(){
	nyi/*std::string r{
		#embed "ca_zipcodes.csv"
	};
	return r;*/
}

/*std::vector<Zip_code_data> read_inner1(){
	//ifstream f("ca_zipcodes.csv");
	std::stringstream f;
	f<<data();

	std::vector<Zip_code_data> r;
	std::string s;
	while(f.good()){
		getline(f,s);
		if(s.empty()) continue;
		auto sp=split(s,',');
		if(sp.size()!=3){
			PRINT(s);
			PRINT(sp);
		}
		assert(sp.size()==3);
		r|=Zip_code_data(sp[0],sp[1],sp[2]);
	}
	return r;
}*/

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
	//return read_inner();
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

template<typename T>
auto count(std::vector<T> const& a){
	std::map<T,size_t> r;
	for(auto x:a){
		r[x]++;
	}
	return r;
}

#if 0
int main(){
	auto r=read();
	//TBA_PRINT(r);
	//PRINT(norcal());
	//PRINT(socal());

	auto z=mapf([](auto x){ return x.zipcode; },read());
	auto m=mapf(california_region,z);
	PRINT(m);
	PRINT(count(m));
	//PRINT(data().size());
	//PRINT(data().substr(0,100));
	return 0;
}
#endif

/*int main(){
	tba::main();
}*/
