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

void print_r(int n,frc_api::Match const& a){
	indent(n);
	cout<<"Match\n";
	n++;
	#define X(A,B) indent(n); cout<<""#B<<"\n"; print_r(n+1,a.B);
	FRC_API_MATCH(X)
	#undef X
}

void print_r(int n,frc_api::TeamListings const& a){
	indent(n++);
	cout<<"TeamListings\n";
	#define X(A,B) indent(n); cout<<""#B<<"\n"; print_r(n+1,a.B);
	FRC_API_TEAMLISTINGS(X)
	#undef X
}

void print_r(int n,frc_api::Event const& a){
	indent(n++);
	cout<<"Event\n";
	#define X(A,B) indent(n); cout<<""#B<<"\n"; print_r(n+1,a.B);
	FRC_API_EVENT(X)
	#undef X
}
void print_r(int n,tba::Team const& a){
	indent(n++);
	std::cout<<"Team\n";
	#define X(A,B) indent(n); std::cout<<""#B<<"\n"; print_r(n+1,a.B);
	TBA_TEAM(X)
	#undef X
}


void print_r(int n,tba::Match const& a){
	indent(n++);
	std::cout<<"Match\n";
	#define X(A,B) indent(n); std::cout<<""#B<<"\n"; print_r(n+1,a.B);
	TBA_MATCH(X)
	#undef X
}

#define PRINT_R_INNER(A,B) indent(n); cout<<""#B<<"\n"; print_r(n+1,a.B);

#define PRINT_R_ITEM(NAME,ITEMS) \
        void print_r(int n,NAME const& a){\
                indent(n);\
                cout<<""#NAME<<"\n";\
                n++;\
                ITEMS(PRINT_R_INNER)\
        }

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

