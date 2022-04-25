#ifndef FRC_API_H
#define FRC_API_H

#include<map>
#include "../frc_api/query.h"
#include "../frc_api/data.h"
#include "../frc_api/db.h"

struct FRC_api_fetcher{
	using Out=std::pair<std::optional<frc_api::HTTP_Date>,frc_api::Data>;
	using URL=frc_api::URL;
	virtual Out fetch(URL const&)=0;
};

template<typename T>
class FRC_api_fetcher_impl:public FRC_api_fetcher{
	T *t; //not owned!

	public:
	explicit FRC_api_fetcher_impl(T *t1):t(t1){
		assert(t);
	}

	Out fetch(URL const& a){
		return t->fetch(a);
	}
};

using Team=frc_api::Team_number;

std::map<frc_api::District_code,std::vector<Team>> teams(FRC_api_fetcher&,frc_api::Season);

std::map<frc_api::District_code,std::vector<Team>> teams(auto& f,frc_api::Season year){
	FRC_api_fetcher_impl f1{&f};

	return teams(static_cast<FRC_api_fetcher&>(f1),year);
}

std::optional<std::map<Team,std::pair<std::vector<int>,std::optional<int>>>> analyze_district(
	FRC_api_fetcher &,
	frc_api::Season,
	frc_api::District_code
);

std::optional<std::map<Team,std::pair<std::vector<int>,std::optional<int>>>> analyze_district(
	auto &f,
	frc_api::Season year,
	frc_api::District_code district
){
	FRC_api_fetcher_impl f1{&f};
	return analyze_district(static_cast<FRC_api_fetcher&>(f1),year,district);
}

namespace frc_api{
template<typename Fetcher,typename T>
auto run(Fetcher &fetcher,frc_api::URL url,const T*){
	//PRINT(url);
	auto g=fetcher.fetch(url);
	rapidjson::Document a;
	a.Parse(g.second.c_str());
	try{
		return decode(a,(T*)nullptr);
	}catch(...){
		std::cout<<url<<"\n";
		throw;
        }
}

#define X(A,B) \
	template<typename Fetcher>\
	B run(Fetcher &fetcher,A a){\
		return run(fetcher,url(a),(B*)nullptr);\
	}
FRC_API_QUERY_TYPES(X)
#undef X
}

#endif
