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



