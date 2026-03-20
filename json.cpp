#include "json.h"

using SB=simdjson::builder::string_builder;

template<typename K,typename V>
void serialize(SB&,std::map<K,V> const&);

template<typename T>
void serialize(SB&,std::vector<T> const&);

template<typename T,size_t N>
void serialize(SB&,tba::vector_fixed<T,N> const&);

template<typename T>
void serialize(SB &sb,std::optional<T> const& a);

void serialize(SB& sb,int a){
	sb.append(a);
}

void serialize(SB& sb,std::string const& a){
	sb.escape_and_append_with_quotes(a);
}

void serialize(SB& sb,bool a){
	sb.append(a);
}

void serialize(SB& sb,double a){
	sb.append(a);
}

void serialize(simdjson::builder::string_builder& sb,tba::Year const& a){
	sb.append(a.get());
}

void serialize(SB& sb,tba::District_key const& a){
	sb.append(a.get());
}

void serialize(SB& sb,tba::District_abbreviation const& a){
	sb.append(a.get());
}

void serialize(SB& sb,tba::Team_key const& a){
	sb.append(a.str());
}

void serialize(SB& sb,tba::Event_key const& a){
	sb.append(a.get());
}

void serialize(SB& sb,tba::Event_type const& a){
	#define X(A,B) if(a==tba::Event_type::A){ sb.append(B); return; }
	TBA_EVENT_TYPES(X)
	#undef X
	assert(0);
}

void serialize(SB& sb,tba::Webcast_type const& a){
	#define X(A) if(a==tba::Webcast_type::A){ sb.append(""#A); return; }
	TBA_WEBCAST_TYPES(X)
	#undef X
	assert(0);
}

void serialize(SB& sb,tba::Playoff_type a){
	#define X(A,B,C) if(a==tba::Playoff_type::B){ sb.append(A); return; }
	TBA_PLAYOFF_TYPES(X)
	#undef X
	assert(0);
}

void serialize(SB& sb,tba::Award_type a){
	#define X(A,B) if(a==tba::Award_type::A){ sb.append(B); return; }
	TBA_AWARD_TYPES(X)
	#undef X
	assert(0);
}

void serialize(SB& sb,std::chrono::year_month_day const& a){
	std::stringstream ss;
	ss<<a;
	sb.append(ss.str());
}

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

#define STRUCT_TO_JSON1(NAME,ITEMS)\
	void serialize(SB& sb,NAME const& a){\
		sb.start_object();\
		bool first=1;\
		ITEMS(STRUCT_TO_JSON_INNER)\
		sb.end_object();\
	}\

STRUCT_TO_JSON1(tba::API_Status_App_Version,TBA_API_STATUS_APP_VERSION)
STRUCT_TO_JSON1(tba::Year_info,TBA_YEAR_INFO)
STRUCT_TO_JSON1(tba::District_Ranking,TBA_DISTRICT_RANKING)
STRUCT_TO_JSON1(tba::Event_points,TBA_EVENT_POINTS)
STRUCT_TO_JSON1(tba::Event,TBA_EVENT)
STRUCT_TO_JSON1(tba::District_List,TBA_DISTRICT_LIST)
STRUCT_TO_JSON1(tba::Webcast,TBA_WEBCAST)
STRUCT_TO_JSON1(tba::Team,TBA_TEAM)
STRUCT_TO_JSON1(tba::Award,TBA_AWARD)
STRUCT_TO_JSON1(tba::Award_Recipient,TBA_RECIPIENT)
STRUCT_TO_JSON1(tba::Dcmp_history,TBA_DCMP_HISTORY)
STRUCT_TO_JSON1(tba::Event_District_Points,TBA_EVENT_DISTRICT_POINTS)
STRUCT_TO_JSON1(tba::Points,TBA_POINTS)
STRUCT_TO_JSON1(tba::Tiebreaker,TBA_TIEBREAKER)

/*void serialize(SB& sb,tba::API_Status_App_Version const& a){
	sb.start_object();
	#define X(A,B) sb.append(""#B); sb.append_colon(); serialize(sb,a.B); sb.append_comma();
	TBA_API_STATUS_APP_VERSION(X)
	#undef X
	sb.end_object();
}*/

void serialize(simdjson::builder::string_builder& sb,tba::API_Status const& a){
	sb.start_object();
	//#define X(A,B) sb.append_key_value(""#B,a.B); sb.append_comma();
	bool first=1;
	#define X(A,B){\
		if(first){\
			first=0;\
		}else{\
			sb.append_comma();\
		}\
		sb.append(""#B); \
		sb.append_colon(); \
		serialize(sb,a.B); \
	}
	TBA_API_STATUS(X)
	#undef X
	sb.end_object();
}
