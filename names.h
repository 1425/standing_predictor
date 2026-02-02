#ifndef NAMES_H
#define NAMES_H

#include<string>
#include<set>

struct TBA_fetcher;

namespace tba{
	struct Team;
};

std::string parse_event_name(TBA_fetcher&,std::string const&);

struct Team_name_contents{
	std::set<std::string> sponsors,orgs;

	auto operator<=>(Team_name_contents const&)const=default;
};

Team_name_contents parse_name(std::string const&);
Team_name_contents parse_name(tba::Team const&);

#endif
