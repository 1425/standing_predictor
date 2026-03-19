#ifndef SPREADSHEET_H
#define SPREADSHEET_H

#include "probability.h"

namespace tba{
	class District_key;
	class Team_key;
}

class TBA_fetcher;

int make_spreadsheet(
	TBA_fetcher &f,
	std::map<tba::District_key,std::map<tba::Team_key,std::pair<Pr,Pr>>> const&,
	std::string const& output_dir
);

#endif
