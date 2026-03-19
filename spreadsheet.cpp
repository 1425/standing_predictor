#include "spreadsheet.h"
#include<sys/wait.h>
#include "district_championship_assignment.h"
#include "query.h"
#include "../tba/tba.h"
#include "tba.h"

using namespace std;

std::vector<pair<tba::Event_key,std::string>> championship_event(auto &f,tba::District_key const& d){
	auto f1=filter(
		[](auto x)->bool{
			//This is here because there is an event whose code is "mbfrc25"
			//And is named "DO NOT APPLY - FRC Test Event for BV"
			//But it says that it is a second 2025 New England district championship.
			if(prefix(x.name,"DO NOT APPLY - FRC Test Event")){
				return 0;
			}

			return x.event_type==tba::Event_type::DISTRICT_CMP;
		},
		district_events_simple(f,d)
	);
	assert(f1.size()<=MAX_DCMPS);
	return mapf(
		[](auto x){ return make_pair(x.key,x.name); },
		f1
	);
}

int team_number(tba::Team_key const& a){
	return atoi(a.str().c_str()+3);
}

int make_spreadsheet(
	TBA_fetcher &f,
	map<tba::District_key,map<tba::Team_key,std::pair<Pr,Pr>>> const& m,
	string const& output_dir
){
	//write a csv with all the data, then use LibreOffice to convert it to an Excel spreadsheet
	//This output exists specifically for the use in this thread:
	//https://www.chiefdelphi.com/t/2022-frc-robot-data-is-beautiful-see-if-you-can-find-your-team/405227

	string filename="results.csv";
	{
		ofstream o(output_dir+"/"+filename);
		o<<"Team #,CMP,Event,P(DCMP),P(CMP)\n";
		for(auto [district,teams]:m){
			//auto [event_key,event_name]=championship_event(f,district);
			auto e=championship_event(f,district);
			for(auto [team,p]:teams){
				auto x=calc_dcmp_home(f,team,year(district));
				if(x){
					auto [event_key,event_name]=[&](){
						assert(*x<e.size());
						return e[*x];
					}();
					o<<team_number(team)<<","<<event_key<<",\""<<event_name<<"\",";
					o<<p.first<<","<<p.second<<"\n";
				}
			}
		}
	}

	//This is a call to LibreOffice or OpenOffice
	//If neither of those is installed or in the path, this may error out
	//Assuming that this is found though, it will produce some onscreen output that is not especially informative.
	//It wouldn't be the worst thing to send that to /dev/null.
	int pid=fork();
	if(pid==-1){
		return -1;
	}
	if(pid==0){
		//avoid random on-screen messages by literally closing the pipes to the screen.
		close(1);
		close(2);
		int r=system( ("cd "+output_dir+"; soffice --convert-to xlsx "+filename).c_str() );
		exit(r);
	}
	{
		int status;
		pid_t p=waitpid(pid,&status,0);
		assert(p==pid);
		//Note that we're not looking at status; if it failed there's nothing we can do to fix it.
	}
	return 0;
}

