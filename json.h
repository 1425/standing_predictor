#ifndef JSON_H
#define JSON_H

#include "../tba/vector_fixed.h"
#include "../tba/year.h"
#include "../tba/data.h"
#include "io.h"

template<typename T>
void serialize(simdjson::builder::string_builder&,T const& t){
	PRINT(type_string(t));
	nyi
}

using SB=simdjson::builder::string_builder;

template<typename K,typename V>
void serialize(SB&,std::map<K,V> const&);

template<typename T>
void serialize(SB&,std::vector<T> const&);

template<typename T,size_t N>
void serialize(SB&,tba::vector_fixed<T,N> const&);

template<typename T>
void serialize(SB &sb,std::optional<T> const& a);

void serialize(SB&,int);
void serialize(SB&,std::string const&);
void serialize(SB&,bool);
void serialize(SB&,double);
void serialize(simdjson::builder::string_builder& sb,tba::Year const&);
void serialize(SB&,tba::District_key const&);
void serialize(SB&,tba::District_abbreviation const&);
void serialize(SB&,tba::Team_key const&);
void serialize(SB&,tba::Event_key const&);
void serialize(SB&,tba::Event_type const&);
void serialize(SB&,tba::Webcast_type const&);
void serialize(SB&,tba::Playoff_type);
void serialize(SB&,tba::Award_type);
void serialize(SB&,std::chrono::year_month_day const&);

#define STRUCT_TO_JSON_INNER(A,B) {\
	if(first){\
		first=0;\
	}else{\
		sb.append_comma();\
	}\
	sb.append(""#B);\
	sb.append_colon();\
	serialize(sb,a.B);\
}\

#define STRUCT_TO_JSON(NAME,ITEMS)\
	void serialize(SB&,NAME const&);

STRUCT_TO_JSON(tba::API_Status_App_Version,TBA_API_STATUS_APP_VERSION)
STRUCT_TO_JSON(tba::Year_info,TBA_YEAR_INFO)
STRUCT_TO_JSON(tba::District_Ranking,TBA_DISTRICT_RANKING)
STRUCT_TO_JSON(tba::Event_points,TBA_EVENT_POINTS)
STRUCT_TO_JSON(tba::Event,TBA_EVENT)
STRUCT_TO_JSON(tba::District_List,TBA_DISTRICT_LIST)
STRUCT_TO_JSON(tba::Webcast,TBA_WEBCAST)
STRUCT_TO_JSON(tba::Team,TBA_TEAM)
STRUCT_TO_JSON(tba::Award,TBA_AWARD)
STRUCT_TO_JSON(tba::Award_Recipient,TBA_RECIPIENT)
STRUCT_TO_JSON(tba::Dcmp_history,TBA_DCMP_HISTORY)
STRUCT_TO_JSON(tba::Event_District_Points,TBA_EVENT_DISTRICT_POINTS)
STRUCT_TO_JSON(tba::Points,TBA_POINTS)
STRUCT_TO_JSON(tba::Tiebreaker,TBA_TIEBREAKER)

template<typename T>
void serialize(SB& sb,std::vector<T> const& a){
	sb.start_array();
	bool first=1;
	for(auto const& x:a){
		if(first){
			first=0;
		}else{
			sb.append_comma();
		}
		serialize(sb,x);
	}
	sb.end_array();
}

template<typename T,size_t N>
void serialize(SB& sb,tba::vector_fixed<T,N> const& a){
	sb.start_array();
	bool first=1;
	for(auto const& x:a){
		if(first){
			first=0;
		}else{
			sb.append_comma();
		}
		serialize(sb,x);
	}
	sb.end_array();
}

template<typename T>
void serialize(SB &sb,std::optional<T> const& a){
	if(a){
		serialize(sb,*a);
	}else{
		sb.append_null();
	}
}

void serialize(simdjson::builder::string_builder& sb,tba::API_Status const&);

template<typename K,typename V>
void serialize(SB &sb,std::map<K,V> const& a){
	sb.start_object();
	bool first=1;
	for(auto [k,v]:a){
		if(first){
			first=0;
		}else{
			sb.append_comma();
		}
		serialize(sb,k);
		sb.append_colon();
		serialize(sb,v);
	}
	sb.end_object();
}

/*std::string to_json(tba::API_Status const& a){
	simdjson::builder::string_builder sb;
	serialize(sb,a);
	return sb;
}*/

template<typename T>
std::string to_json(T const& a){
	simdjson::builder::string_builder sb;
	//print_r(a);
	serialize(sb,a);
	return sb;
}

#endif
