#include "print_r.h"
#include<sys/ioctl.h>
#include<unistd.h>
#include "../frc_api/data.h"
#include "../tba/data.h"

using namespace std;

int terminal_width(){
	#ifdef __unix__
	struct winsize w;
	ioctl(STDIN_FILENO,TIOCGWINSZ,&w);
	auto found=w.ws_col;
	//PRINT(found);
	if(!(found>0 && found<1000)){
		PRINT(found);
		cout.flush();
	}
	assert(found>0 && found<1000);
	return found;
	#else
	return 80;
	#endif
}

std::string abbreviate(int max_width,std::string const& s){
	assert(max_width>0);
	if(max_width<=0){
		return "";
	}

	if(s.size()<=size_t(max_width)){
		return s;
	}

	return s.substr(0,max_width-3)+"...";
}

PRINT_R_ITEM(frc_api::Match,FRC_API_MATCH)
PRINT_R_ITEM(frc_api::TeamListings,FRC_API_TEAMLISTINGS)
PRINT_R_ITEM(frc_api::Event,FRC_API_EVENT)

PRINT_R_ITEM(tba::Team,TBA_TEAM)
PRINT_R_ITEM(tba::Match,TBA_MATCH)

PRINT_R_ITEM(tba::Match_Score_Breakdown_2024_Alliance,TBA_MATCH_SCORE_BREAKDOWN_2024_ALLIANCE)
PRINT_R_ITEM(tba::Match_Score_Breakdown_2023_Alliance,TBA_MATCH_SCORE_BREAKDOWN_2023_ALLIANCE)
PRINT_R_ITEM(tba::Match_Score_Breakdown_2022_Alliance,TBA_MATCH_SCORE_BREAKDOWN_2022_ALLIANCE)
PRINT_R_ITEM(tba::Match_Score_Breakdown_2020_Alliance,TBA_MATCH_SCORE_BREAKDOWN_2020_ALLIANCE)
PRINT_R_ITEM(tba::Match_Score_Breakdown_2017_Alliance,TBA_MATCH_SCORE_BREAKDOWN_2017_ALLIANCE)
PRINT_R_ITEM(tba::Match_Score_Breakdown_2016_Alliance,TBA_MATCH_SCORE_BREAKDOWN_2016_ALLIANCE)
//PRINT_R_ITEM(tba::Match_Score_Breakdown_2014_Alliance,TBA_MATCH_SCORE_BREAKDOWN_2014_ALLIANCE)

//#define BREAKDOWN_IMPL(YEAR) PRINT_R_ITEM(tba::Match_Score_Breakdown_##YEAR##_Alliance,TBA_MATCH_SCORE_BREAKDOWN_##YEAER##_ALLIANCE)

//BREAKDOWN_IMPL(2023)

PRINT_R_ITEM(tba::Elimination_Alliance,TBA_ELIMINATION_ALLIANCE)

