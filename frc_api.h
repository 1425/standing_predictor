#ifndef FRC_API_H
#define FRC_API_H

#include<map>
#include<set>
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
        simdjson::dom::parser parser;
        simdjson::padded_string str(g.second);
        auto doc=parser.parse(str);
        try{
                switch(doc.type()){
                        case simdjson::dom::element_type::ARRAY:
                                return decode(doc.get_array(),(T*)nullptr);
                        case simdjson::dom::element_type::OBJECT:
                                return decode(doc.get_object(),(T*)nullptr);
                        case simdjson::dom::element_type::NULL_VALUE:
                                return decode(nullptr,(T*)nullptr);
                        default:
                                throw Decode_error{typeid(T).name(),"","unexpected type"};
                }
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

std::set<frc_api::Team_number> chairmans_winners(
	FRC_api_fetcher&,
	frc_api::Season,
	frc_api::District_code const&
);

std::set<frc_api::Team_number> chairmans_winners(auto &f,frc_api::Season a,frc_api::District_code const& b){
	FRC_api_fetcher_impl f1(&f);
	return chairmans_winners(static_cast<FRC_api_fetcher&>(f1),a,b);
}

#endif
