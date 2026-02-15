#ifndef ADDRESS_H
#define ADDRESS_H

#include<string>
#include<optional>
#include<set>
#include "util.h"
#include "ca.h"

namespace tba{
	struct Event;
	struct Event_key;
};

class TBA_fetcher;

struct Country{
	std::string s;

	bool operator==(const char *)const;
	bool operator==(Country const&)const;
	auto operator<=>(Country const&)const=default;
};

std::ostream& operator<<(std::ostream&,Country const&);

struct State_prov{
	std::string s;

	State_prov& operator=(std::string const&);

	auto operator<=>(State_prov const&)const=default;

	bool operator==(std::string const&)const;
};

std::ostream& operator<<(std::ostream&,State_prov const&);
bool operator==(State_prov const&,const char *);
bool operator==(std::optional<State_prov> const&,const char *);

#define ADDRESS(X)\
	X(Country,country)\
	X(std::optional<State_prov>,state)\
	X(std::optional<City>,city)

//Not making the address any more granular because it doesn't need to be for anything that is happening right now.
struct Address{
	ADDRESS(INST)
};

std::ostream& operator<<(std::ostream&,Address const&);

std::optional<Address> address(tba::Event const&);
std::optional<Address> address(TBA_fetcher&,tba::Event_key const&);

std::set<std::string> const& state_codes();
Country get_country(tba::Event const&);
std::optional<State_prov> normalize_state(std::string);
int check_address(TBA_fetcher&);

#endif
