#include "team_history.h"
#include "optional.h"
#include "history.h"
#include "plot.h"
#include "print_r.h"
#include "query.h"

using namespace std;
using Team_key=tba::Team_key;
using Year=tba::Year;

std::optional<Plot_setup> team_history(TBA_fetcher& f,std::vector<History_item> const& h,Team_key team,Year year){
	(void)f;
	auto found=filter([&](auto x){ return x.team==team && x.year==year; },h);
	if(found.empty()){
		return std::nullopt;
	}

	//for each team/season combo
	//look at probabilities over time during that season
	//plot two lines of CMP and DCMP probabilities over that season
	//need to interpolate the points -> when day is not listed assume that's because no chage on that day
	//need to look up events that the team was at to put in the vertical lines
	//for each of the events, put in the date on which it ends
	
	Plot_line line_cmp;
	Plot_line line_dcmp;

	std::vector<std::chrono::year_month_day> event_dates;

	size_t seen=0;
	for(auto elem:found){
		if(elem.appearances!=seen){
			seen=elem.appearances;
			event_dates|=elem.date;
		}
		line_cmp|=make_tuple(elem.date,elem.cmp_pr,0);
		line_dcmp|=make_tuple(elem.date,elem.dcmp_pr,0);
	}

	Plot_lines pl{
		team+" "+year,
		"Date",
		"Probability",
		{line_cmp,line_dcmp},
		event_dates
	};

	Plot_setup ps{pl,"img title"};

	return ps;

}

int team_history_demo(TBA_fetcher &f){
	auto team=Team_key("frc5167");
	//auto year=Year(2024);
	std::vector<Plot_setup> v;
	auto h=read_history(f);
	PRINT(h.size());
	for(auto year:years()){
		v|=team_history(f,h,team,year);
	}
	auto p=plot(v);
	static const string PATH="out.html";
	{
		ofstream f(PATH);
		for(auto const& x:p){
			f<<x;
		}
	}
	return system("firefox "+PATH);
	
}
