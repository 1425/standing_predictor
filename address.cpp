#include "address.h"
#include "io.h"
#include "../tba/tba.h"
#include "print_r.h"
#include "optional.h"
#include "vector_void.h"
#include "tba.h"
#include "set_fixed.h"

using namespace std;

string rm_comma(string a){
	if(a.size() && a[a.size()-1]==','){
		return a.substr(0,a.size()-1);
	}
	return a;
}

vector<string> rm_commas(vector<string> a){
	return mapf(rm_comma,a);
}

std::optional<Address> address(TBA_fetcher &f,tba::Event_key const& key){
	return address(tba::event(f,key));
}

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

bool operator==(City const& a,const char *s){
	if(!s){
		return 0;
	}
	return a==string(s);
}

bool operator==(std::optional<City> const& a,const char *s){
	if(a){
		return *a==s;
	}

	return !s;
}

bool Country::operator==(const char *s1)const{
	if(!s1){
		return 0;
	}
	return s==s1;
}

bool Country::operator==(Country const& a)const{
	return s==a.s;
}

std::ostream& operator<<(std::ostream& o,Country const& a){
	return o<<a.s;
}

State_prov& State_prov::operator=(std::string const& a){
	s=a;
	return *this;
}

bool State_prov::operator==(State_prov const& a)const{
	return s==a.s;
}

bool State_prov::operator==(std::string const& a)const{
	return s==a;
}

std::ostream& operator<<(std::ostream& o,State_prov const& a){
	return o<<a.s;
}

bool operator==(State_prov const& a,const char *b){
	if(!b){
		return 0;
	}
	return a==string(b);
}

bool operator==(std::optional<State_prov> const& a,const char *b){
	if(!a){
		return !b;
	}
	return *a==b;
}

/*#define ADDRESS(X)\
	X(Country,country)\
	X(std::optional<State_prov>,state)\
	X(std::optional<City>,city)

struct Address{
	ADDRESS(INST)
};*/

PRINT_STRUCT(Address,ADDRESS)

static const std::set<std::string> STATE_CODES{
	"AL", "AK", "AZ", "AR", "CA", "CO", "CT", "DE", "FL", "GA",
	"HI", "ID", "IL", "IN", "IA", "KS", "KY", "LA", "ME", "MD",
	"MA", "MI", "MN", "MS", "MO", "MT", "NE", "NV", "NH", "NJ",
	"NM", "NY", "NC", "ND", "OH", "OK", "OR", "PA", "RI", "SC",
	"SD", "TN", "TX", "UT", "VT", "VA", "WA", "WV", "WI", "WY",
	"DC" //omitting outlying territories, etc.
};

std::set<std::string> const& state_codes(){
	return STATE_CODES;
}

std::optional<Country> normalize_country(std::optional<std::string> s1){
	if(!s1){
		return std::nullopt;
	}
	auto s=*s1;
	if(s=="US" || s=="United States" || s=="Usa" || s=="usa" || s=="YSA" || s=="San Jose"){
		s="USA";
	}
	if(s=="Turkiye" || s=="Tu00fcrkiye" || s=="Türkiye"){
		s="Turkey";
	}
	if(s=="CN"){
		s="China";
	}
	if(s=="AU" || s=="Austrialia" || s=="Australia "){
		s="Australia";
	}
	if(s=="Chinese Taipei" || s=="XX"){
		s="Taiwan";
	}
	if(s=="CA"){
		s="Canada";
	}
	if(s=="Northern Israel"){
		s="Israel";
	}

	//std::set<std::string> known{
	constexpr set_fixed<std::string_view,9> known{
		"USA","Canada","Israel","Mexico",
		"China","Brazil","Australia","Turkey",
		"Taiwan"
	};
	if(known.count(s)){
		return Country(s);
	}
	try{
		//looks like someone put a zipcode in instead.
		stoi(s);
		return std::nullopt;
	}catch(...){}

	//obviously invalid; replace with correct value
	if(s=="San Jose" || s=="Suffolk "){
		return Country("USA");
	}

	//PRINT(s1);
	//throw "what?";
	assert(s1);
	return Country(*s1);
}

std::optional<State_prov> normalize_state(std::string s){
	s=strip(s);
	using R=std::optional<State_prov>;
	if(s=="Florida" || s=="Fl"){
		return R("FL");
	}
		if(s=="Oregon"){
			return R("OR");
		}
		if(s=="California" || s=="CA " || s=="California "){
			return R("CA");
		}
		if(s=="New York"){
			return R("NY");
		}
		if(s=="Maryland"){
			return R("MD");
		}
		if(s=="Minnesota" || s=="mn" || s=="Mn"){
			return R("MN");
		}
		if(s=="Illinois"){
			return R("IL");
		}
		if(s=="Colorado"){
			return R("CO");
		}
		if(s=="Ohio" || s=="OHI"){
			return R("OH");
		}
		if(s=="MS "){
			return R("MS");
		}
		if(s=="Missouri"){
			return R("MO");
		}
		if(s=="Louisiana"){
			return R("LA");
		}
		if(s=="Michigan"){
			return R("MI");
		}
		if(s=="North Carolina"){
			return R("NC");
		}
		if(s=="Arkansas"){
			return R("AR");
		}
		if(s=="Wisconsin"){
			return R("WI");
		}
		if(s=="St. Louis"){
			return R("MO");
		}

	//Canada
	if(s=="Ontario"){
		return R("ON");
	}

	if(s=="XX"){
		return std::nullopt;
	}

	//Austrailia section
	if(s=="Victoria"){
		return R("VIC");
	}
	if(s=="New South Wales"){
		return R("NSW");
	}

	return State_prov(s);
}

std::optional<State_prov> normalize_state(std::optional<std::string> const& a){
	if(!a){
		return std::nullopt;
	}
	return normalize_state(*a);
}

auto normalize_state(State_prov const& a){
	return normalize_state(a.s);
}

std::optional<State_prov> normalize_state(std::optional<State_prov> const& a){
	if(!a){
		return std::nullopt;
	}
	return normalize_state(*a);
}

Country get_country(tba::Event const& x){
			if(x.key=="2007br"){
				return Country("Brazil");
			}
			if(x.key=="2006ca"){
				return Country("USA");
			}
			if(x.key=="2007az"){
				return Country("USA");
			}
			if(x.key=="2007ct"){
				return Country("USA");
			}
			if(x.key=="2007co"){
				return Country("USA");
			}
			if(x.key=="2007fl"){
				return Country("USA");
			}
			if(x.key=="2007ca"){
				return Country("USA");
			}
			if(x.key=="2013mhsl"){
				return Country("USA");
			}
			if(x.key=="2014bfbg"){
				return Country("USA");
			}
			if(x.key=="2013rsr"){
				return Country("USA");
			}
			if(x.key=="2014mshsl"){
				return Country("USA");
			}
			if(x.key=="2013mshsl"){
				return Country("USA");
			}
			if(x.key=="2013mm"){
				return Country("USA");
			}
			if(x.key=="2014audd"){
				return Country("Australia");
			}
			if(x.key=="2017onsc" || x.key=="2022oncne" || x.key=="2022onsc" || x.key=="2022onsc2"){
				return Country("Canada");
			}
			if(x.key=="2022xxma2" || x.key=="2022xxmac"){
				return Country("Australia");
			}
			if(x.key=="2022xxrio" || x.key=="2023xxrio"){
				return Country("Brazil");
			}
			if(x.key=="2023mexas"){
				return Country("Mexico");
			}
			if(x.key=="2023oncne" || x.key=="2023onsc" || x.key=="2023onsc3"){
				return Country("Canada");
			}
			if(x.key=="2024isios" || x.key=="2024isos2"){
				return Country("Israel");
			}
			if(x.key=="2024tacy"){
				return Country("Taiwan");
			}
			//print_r(x);
			auto n=normalize_country(x.country);
			if(!n){
				if(!x.state_prov){
					print_r(x);
					return Country("");
				}
				assert(x.state_prov);
				if(STATE_CODES.count(*x.state_prov)){
					return Country("USA");
				}else{
					nyi
				}
			}

			/*try{
				return make_tuple(normalize_country(x.country),*x.state_prov,*x.city);
			}catch(...){
				cout<<"country: \""<<x.country<<"\"\n";
				print_r(x);
				//assert(0);
				return nullopt;
			}*/
			return *n;
}

auto normalize_state(tba::Event const& a){
	return normalize_state(a.state_prov);
}

optional<Address> address(tba::Event const& event){
	Address r;
	r.country=get_country(event);
	r.state=normalize_state(event);
	if(event.city){
		r.city=City(strip(*event.city));
	}
	if(r.country=="Australia" && !r.state){
		std::map<string,State_prov> m{
			{"Melbourne",State_prov("Victoria")},
			{"Macquarie Park",State_prov("New South Wales")},
			{"Sydney",State_prov("New South Wales")}
		};
		for(auto [k,v]:m){
			if(k==event.city){
				r.state=v;
			}
		}
	}
	if(r.country=="Canada" && !r.state){
		std::map<string,State_prov> m{
			{"Hamilton",State_prov("ON")},
			{"Toronto",State_prov("ON")}
		};
		for(auto [k,v]:m){
			if(k==event.city){
				r.state=v;
			}
		}
	}

	if(r.country=="Mexico" && r.city=="Mexicali"){
		r.state=State_prov("BC");
	}
	if(r.country=="Mexico" && r.state=="DIF"){
		//r.state="DF";//Districto Federal
	}
	if(r.country=="Mexico" && !r.state){
		if(r.city=="Monterrey"){
			r.state="NLE";//Nuevo Leon
		}else{
			PRINT(event);
			PRINT(r);
			nyi
		}
	}
	if(r.country=="Mexico" && r.city=="Leon"){
		r.state="GUA";
	}
	if(!r.state && r.city=="Taipei"){
		r.country=Country("Taiwan");
	}

	if(event.key==tba::Event_key("2014bfbg")){
		r.state=State_prov("KY");
		r.city=City("Corbin");//guess, based on home of team 3844.
	}
	if(event.key==tba::Event_key("2013rsr")){
		r.state=State_prov("LA");
	}
	if(!r.state && split(event.name).at(0)=="Minnesota"){
		r.state=State_prov("MN");
	}
	if(!r.state && event.name=="Monty Madness"){
		r.state=State_prov("NJ");
	}
	if(!r.state && split(event.name).at(0)=="Florida"){
		r.state=State_prov("FL");
	}
	if(!r.state && contains(split(event.name),"Connecticut")){
		r.state=State_prov("CT");
	}
	if(!r.state && contains(split(event.name),"Colorado")){
		r.state=State_prov("CO");
	}
	if(!r.state && contains(split(event.name),"Arizona")){
		r.state=State_prov("AZ");
	}
	if(!r.state && event.name=="Los Angeles Regional"){
		r.state=State_prov("CA");
	}
	if(!r.state && contains(split(event.name),"California")){
		r.state=State_prov("CA");
	}
	if(r.city=="Cuiabu00e1 - MT")nyi
	if(r.city=="Cuiabá")nyi
	if(r.country=="Brazil" && r.city && r.city->data.substr(0,5)=="Cuiab"){
		r.state=State_prov("Mato Grosso");
	}
	if(event.key=="2014audd"){
		r.state=State_prov("New South Wales");
	}
	if(
		r.country=="Brazil" &&
		(!r.state || r.state=="XX") &&
		(r.city=="Rio de Janeiro" || r.city=="Rio De Janeiro")
	){
		r.state="Rio de Janeiro" ;//or RJ
	}
	if(r.country=="Brazil" && !r.state){
		if(
			event.address &&
			contains(rm_commas(split(*event.address)),"RS")
		){
			r.state="RS";
		}else{
			print_r(event);
			print_r(r);
			cout<<"addr \""<<event.address<<"\"\n";
			nyi
		}
	}
	if(prefix(event.address,"Arizona Veterans Coliseum")){
		r.city=City("Phoenix");
	}

	if(r.state=="AZ" && !r.city){
		PRINT(event);
		print_r(r);
		nyi
	}
	if(r.state=="AZ" && (r.city=="Pheonix" || r.city=="Phoeniz")){
		r.city=City("Phoenix");
	}
	/*if(r.city=="Texas"){
		PRINT(event);
		print_r(r);
		nyi
	}*/
	r.state=normalize_state(r.state);
	return r;
}

int number(tba::Team_key const& a){
	return stoi(a.str().substr(3,100));
}

std::optional<Address> address(tba::Team const& team){
	Address r;

	if(!team.country){
		return std::nullopt;
	}
	r.country=Country(*normalize_country(*team.country));

	//assert(team.state_prov);
	r.state=[=](){
		auto n=normalize_state(team.state_prov);
		//assert(n);
		return n;
	}();

	assert(team.city);
	r.city=City(*team.city);
	return r;
}

int check_address(TBA_fetcher &f){
	auto ta=MAP(address,teams(f));
	auto c=mapf([](auto x)->optional<Country>{ if(x) return x->country; return std::nullopt; },ta);
	print_r(count(c));
	/*
	for(auto team:all_teams(f)){
		auto a=address(team);
		//PRINT(a);
		//print_r(team);
		//nyi
	}*/

	for(auto event:reversed(events(f))){
		auto a=address(event);
		if(a && a->state) continue;
		if(a && (
			//a->country=="Brazil" ||
			a->country=="China" ||
			a->country=="Taiwan" ||
			a->country=="Israel" ||
			0//a->country=="Mexico"
			)
		) continue;
		PRINT(event.key);
		PRINT(a);
		print_r(event);
		//if(event.
		//nyi
	}
	return 0;
}


