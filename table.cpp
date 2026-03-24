#include "table.h"
#include "output.h"

using namespace std;

struct Script_namer{
	size_t i=0;

	string operator()(){
		std::stringstream ss;
		ss<<"n"<<i;
		i++;
		return ss.str();
	}
};

std::string as_table(tba::Event const&);

template<typename K,typename V>
std::string as_table(std::map<K,V> const& a);

template<typename K,typename V>
std::string as_table(flat_map2<K,V> const& a);

template<typename T>
std::string as_table(T const& t){
	return ::as_string(t);
}

template<typename T>
auto tr_hide(std::string name,T const& contents);

template<typename K,typename V>
std::string as_table(flat_map2<K,V> const& a){
	std::stringstream ss;
	ss<<"<table border>";
	for(auto const& [k,v]:a){
		ss<<tr_hide(as_string(k),v);
	}
	ss<<"</table>";
	return ss.str();
}

template<typename K,typename V>
std::string as_table(std::map<K,V> const& a){
	std::stringstream ss;
	ss<<"<table border>";
	for(auto const& [k,v]:a){
		ss<<tr_hide(::as_string(k),v);
	}
	ss<<"</table>";
	return ss.str();
}

template<typename T>
std::string as_table(std::vector<T> const& a){
	std::stringstream ss;
	ss<<"<table border>";
	for(auto const& x:a){
		ss<<tr(td(as_table(x)));
	}
	ss<<"</table>";
	return ss.str();
}

template<typename T>
auto tr_hide(std::string name,T const& contents){
	std::string inner=as_table(contents);
	if(inner.size()<100){
		return tr(td(name)+td(inner));
	}
	auto name1="x"+as_string(rand());
	return tr(
		td_top(tag("a href=\"\" onclick=\"toggle_viz('"+name1+"');event.preventDefault();\"",name))+
		tag("td class=hidden id=\""+name1+"\"",inner)
	);
}

#define TR_HIDE(A,B) ss<<tr_hide(""#B,a.B);
#define TR_HIDE1(A) ss<<tr_hide(""#A,a.A);

template<typename T>
std::string as_table(Rank_status<T> const& a){
	std::stringstream ss;
	ss<<"<table border>";
	RANK_STATUS(TR_HIDE)
	ss<<"</table>";
	return ss.str();
}

std::string as_table(tba::Event const& a){
	auto name="x"+as_string(rand());
	std::stringstream ss;
	ss<<tag("a href=\"\" onclick=\"toggle_viz('"+name+"');event.preventDefault();\"",a.short_name);
	ss<<"<table border class=hidden id=\""<<name<<"\">";
	#define X(A,B) ss<<tr(td_top(""#B)+td(as_table(a.B)));
	TBA_EVENT(X)
	#undef X
	ss<<"</table>";
	return ss.str();
}

template<typename T>
std::string as_table(Event_annotated<T> const& a){
	std::stringstream ss;
	ss<<"<table border>";
	TR_HIDE1(data)
	TR_HIDE1(extra)
	ss<<"</table>";
	return ss.str();
}

template<typename A,typename B>
std::string as_table(District_cmp_complex_annotated<A,B> const& a){
	std::stringstream ss;
	ss<<"<table border>";
	TR_HIDE1(finals)
	TR_HIDE1(divisions)
	TR_HIDE1(extra)
	ss<<"</table>";
	return ss.str();
}

auto click(std::string target,std::string body){
	return tag("a href=\"\" onclick=\"toggle_viz('"+target+"');event.preventDefault();\"",body);
}

std::string as_table(Team_points_used const& a){
	std::stringstream ss;
	ss<<"<table border>";
	TEAM_POINTS_USED(TR_HIDE)
	ss<<"</table>";
	return ss.str();
}

//template<typename A,typename B,typename C>
//std::string as_table(Event_categories_annotated<A,B,C> const& a){
//template<typename A,typename B,typename C>
std::string as_table(Annotated const& a){
	stringstream ss;
	ss<<"<table border>";
	TR_HIDE1(local)
	TR_HIDE1(dcmp)
	TR_HIDE1(extra)
	ss<<"</table>";
	return ss.str();
}

std::string as_table(Skill_estimates const& a){
	std::stringstream ss;
	ss<<"<table border>";
	SKILL_ESTIMATES(TR_HIDE)
	ss<<"</table>";
	return ss.str();
}

