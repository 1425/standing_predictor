#include "rp.h"
#include "io.h"
#include "../tba/tba.h"
#include "array.h"
#include "skill_opr.h"
#include "print_r.h"
#include "tba.h"

using namespace std;

template<typename T>
RP rp(T const& a){
	print_r(a);
	nyi
}

using Bonus_rp=Int_limited<0,3>;//depends on year

RP rp(tba::Match_Score_Breakdown_2025_Alliance const& a){
	return a.rp;
}

RP rp(tba::Match_Score_Breakdown_2024_Alliance const& a){
	return a.rp;
}

RP rp(tba::Match_Score_Breakdown_2023_Alliance const& a){
	return a.rp;
}

RP rp(tba::Match_Score_Breakdown_2022_Alliance const& a){
	return a.rp;
}

RP rp(tba::Match_Score_Breakdown_2020_Alliance const& a){
	return a.rp;
}

auto rp(tba::Match_Score_Breakdown_2017_Alliance const& a){
	//warning: this doesn't include the win/tie pts!
	Bonus_rp r=0;
	if(a.kPaRankingPointArchieved){
		//sic ,int
	}
	if(a.rotorRankingPointAchieved){
		r++;
	}
	return r;
}

auto rp(tba::Match_Score_Breakdown_2014_Alliance const&){
	//no bonus RP and not w/l/t info in here!
	return Bonus_rp(0);
}

auto rp(tba::Match_Score_Breakdown_2016_Alliance const& a){
	//warning: no w/l/t in here!
	Bonus_rp r=0;
	if(a.teleopTowerCaptured){
		r++;
	}
	if(a.teleopDefensesBreached){
		r++;
	}
	return r;
}

RP rp(tba::Match_Score_Breakdown_2015_Alliance const&){
	return 0;
}

#define X(NAME) auto rp(tba::Match_Score_Breakdown_##NAME const& a){\
	using T=decltype(rp(a.red));\
	return std::array<T,2>{{rp(a.red),rp(a.blue)}};\
}
X(2026)
X(2025)
X(2024)
X(2023)
X(2022)
X(2020)
X(2017)
X(2016)
X(2015)
X(2014)
#undef X

using RP_count=std::variant<std::array<RP,2>,std::array<Bonus_rp,2>>;

template<typename... Ts>
auto rp(std::variant<Ts...> const& a){
	return std::visit([](auto const& x){ return RP_count(rp(x)); },a);
}

template<typename T>
auto rp(std::optional<T> const& a){
	using N=decltype(rp(*a));
	using R=std::optional<N>;
	if(!a){
		return R(std::nullopt);
	}
	return R(rp(*a));
}

optional<array<RP,2>> rp(tba::Match const& a){
	if(a.comp_level!=tba::Competition_level::qm){
		return array<RP,2>{0,0};
	}

	//print_r(a);
	auto r1=rp(a.score_breakdown);
	if(!r1){
		return std::nullopt;
	}
	auto r=*r1;
	if(std::holds_alternative<std::array<RP,2>>(r)){
		return std::get<std::array<RP,2>>(r);
	}
	array<RP,2> total;
	auto bonus=std::get<std::array<Bonus_rp,2>>(r);
	total+=bonus;

	//assuming that if don't have the points in the score breakdown, this is an
	//older game with 2 RP for a win.
	switch(a.winning_alliance){
		case tba::Winning_alliance::red:
			total[0]+=2;
			break;
		case tba::Winning_alliance::blue:
			total[1]+=2;
			break;
		case tba::Winning_alliance::NONE:
			//assuming that this means a tie.
			total[0]++;
			total[1]++;
			break;
		default:
			assert(0);
	}
	return total;
}

RP max_rp_per_match(tba::Year const& a){
	const auto year=a.get();
	if(year<=2014){
		return 2;
	}
	if(year==2015){
		return 0;
	}
	if(year>=2016 || year<=2020){
		return 4;
	}
	if(year==2021){
		return 0;
	}
	if(year==2022){
		return 4;
	}
	if(year==2023){
		return 5;
	}
	if(year==2024){
		return 4;
	}
	if(year==2025 || year==2026){
		return 6;
	}
	return 6;
}



void rp_distribution(TBA_fetcher &f){
	PRINT(sizeof(RP));
	RP a;
	PRINT(a);
	a++;
	PRINT(a);
	return;

	for(auto year:years()){
		PRINT(year);
		std::multiset<optional<array<RP,2>>> found;
		for(auto event:events(f,year)){
			//PRINT(event.key);
			for(auto match:event_matches(f,event.key)){
				auto x=rp(match);
				/*if(!x) continue;
				PRINT(x);
				nyi*/
				if(!x){
					found|=x;
				}else{
					found|=sorted(*x);
				}
			}
		}
		PRINT(found.size());
		print_r(count(found));
	}
}


