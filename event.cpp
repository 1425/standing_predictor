#include "event.h"
#include<iostream>

using namespace std;

int dcmp_size(tba::District_key const& district){
	if(district=="2019chs") return 58;
	if(district=="2019isr") return 45;
	if(district=="2019fma") return 60;
	if(district=="2019fnc") return 32;
	if(district=="2019ont") return 80;
	if(district=="2019tx") return 64;
	if(district=="2019in") return 32;
	if(district=="2019fim") return 160;
	if(district=="2019ne") return 64;
	if(district=="2019pnw") return 64;
	if(district=="2019pch") return 45;

	//via the 2022 game manual v5
	if(district=="2022chs") return 60;
	if(district=="2022isr") return 36;
	if(district=="2022fma") return 60;
	if(district=="2022fnc") return 32;
	if(district=="2022ont") return 80;
	if(district=="2022fit") return 80; //Texas; previously "tx"
	if(district=="2022fin") return 32; //Previously "in"
	if(district=="2022fim") return 160;
	if(district=="2022ne") return 80;
	if(district=="2022pnw") return 50;
	if(district=="2022pch") return 32;

	if(district=="2023chs") return 60;
	if(district=="2023isr") return 40;
	if(district=="2023fma") return 60;
	if(district=="2023fnc") return 40;
	if(district=="2023ont") return 80;
	if(district=="2023fit") return 80;
	if(district=="2023fin") return 32;
	if(district=="2023fim") return 160;
	if(district=="2023ne") return 90;
	if(district=="2023pnw") return 50;
	if(district=="2023pch") return 50;

	cerr<<"Unknown event size for "<<district<<"\n";
	exit(1);
}

int worlds_slots(tba::District_key const& key){
	map<string,int> slots{
		{"2019chs",21},
		{"2019fim",87},
		{"2019isr",11},
		{"2019fma",21},
		{"2019in",10},
		{"2019ne",33},
		{"2019ont",29},
		{"2019tx",38},
		{"2019fnc",15},
		{"2019pnw",31},
		{"2019pch",17},
	};
	auto f=slots.find(key.get());
	if(f!=slots.end()){
		return f->second;
	}

	//Via https://www.firstinspires.org/resource-library/frc/championship-eligibility-criteria
	//Subtract out chairmans, EI, and Rookie All-Star winners
	//Note that Rookie All-Star is not always awarded. (which is the last number)
	if(key=="2022chs") return 16-2-2-1;
	if(key=="2022fim") return 64-4-1-1;
	if(key=="2022fma") return 18-2-2-1;
	if(key=="2022fin") return 8-1-2-1;
	if(key=="2022ne") return 25-3-2-1;
	if(key=="2022ont") return 11-1-1-1;
	if(key=="2022fit") return 23-3-2-2;
	if(key=="2022isr") return 9-1-1-1;
	if(key=="2022fnc") return 10-1-2-1;
	if(key=="2022pnw") return 18-2-1-1;
	if(key=="2022pch") return 10-1-2-1;

	if(key=="2023chs") return 19-2-2-1;
	if(key=="2023fim") return 82-5-1-2;
	if(key=="2023fma") return 23-2-2-1;
	if(key=="2023fin") return 10-1-1-1;
	if(key=="2023ne") return 32-4-2-1;
	if(key=="2023ont") return 23-3-1-1;
	if(key=="2023fit") return 30-3-2-2;
	if(key=="2023isr") return 11-1-1-1;
	if(key=="2023fnc") return 14-1-2-2;
	if(key=="2023pnw") return 22-2-2-1;
	if(key=="2023pch") return 17-2-2-2;

	cerr<<"Error: Unknown number of worlds slots for district:"<<key<<"\n";
	exit(1);
}

#if 0
void dcmp_awards(District_key district){
	map<District_key,int> chairmans{
		{"2019pnw",3/*chairmans*/+2/*ei*/+1/*ras*/},
	};
	//able to win DCMP EI without competing there with robot?
	//looks like 568 did last year in PNW
}
#endif

