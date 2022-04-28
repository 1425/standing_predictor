#include "print_r.h"
#include "../frc_api/data.h"

using namespace std;

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

