#include<iostream>
#include "io.h"
#include<fstream>
#include "vector.h"
#include "run.h"
#include "flat_map2.h"
#include "event_limits.h"

using namespace std;

//start generic

auto p(auto a){
	return tag("p",a);
}

auto html_head(auto x){
	return tag("head",x);
}

int system(std::string const& s){
	return system(s.c_str());
}

//start program-specific

class HTML_visitor{
	public:
	std::ostream& o;

	private:
	int used=0;

	public:

	explicit HTML_visitor(std::ostream& o1):o(o1){}

	HTML_visitor(HTML_visitor const&)=delete;

	std::string name(){
		std::stringstream ss;
		ss<<"u"<<used;
		used++;
		return ss.str();
	}

	int indent_level=0;

	void indent(){
		for(auto _:range(indent_level)){
			(void)_;
			o<<"\t";
		}
	}

	void line(std::string a){
		indent();
		o<<a<<"\n";
	}
};

void as_html(HTML_visitor&,Output_tuple const&);

template<typename T,size_t N>
void as_html(HTML_visitor &,std::array<T,N> const&);

void as_html(HTML_visitor &a,auto b){
	a.indent();
	a.o<<p(b)<<"\n";
}

template<typename K,typename V>
void as_html(HTML_visitor &a,flat_map2<K,V> const& b){
	//as_html(a,to_vec(b));
	//do two column layout with keys and values
	
	a.line("<table border>");
	a.indent_level++;

	for(auto [k,v]:b){
		a.line("<tr>");
		a.indent_level++;
		a.line(td(k));

		a.line("<td>");
		a.indent_level++;
		as_html(a,v);
		a.indent_level--;
		a.line("</td>");

		a.indent_level--;
		a.line("</tr>");
	}

	a.indent_level--;
	a.line("</table>");
}

template<typename K,typename V>
void as_html(HTML_visitor &a,std::map<K,V> const& b){
	as_html(a,to_flat_map2(b));
}

template<typename T>
void as_html(HTML_visitor &a,Interval<T> const& b){
	as_html(a,make_pair(b.min,b.max));
}

void as_tr(HTML_visitor &a,auto const& b){
	a.line("<tr><td>");
	a.indent_level++;
	as_html(a,b);
	a.indent_level--;
	a.line("</td></tr>");
}

template<typename A,typename B>
void as_html(HTML_visitor &a,std::pair<A,B> const& b){
	a.line("<table><tr><td>");

	a.indent_level++;
	a.line("pair");
	a.indent_level--;

	a.line("</td><td>");
	a.indent_level++;

	a.line("<table border>");
	a.indent_level++;
	as_tr(a,b.first);
	as_tr(a,b.second);
	a.indent_level--;
	a.line("</table>");

	a.indent_level--;
	a.line("</td></tr></table>");
}

template<typename T,size_t N>
void as_html(HTML_visitor &a,std::array<T,N> const& b){
	a.line("<table><tr><td>");
	a.indent_level++;

	a.indent();
	a.o<<"array\n";

	a.indent_level--;
	a.line("</td><td>");
	a.indent_level++;

	a.indent();
	a.o<<"<table border>\n";

	a.indent_level++;
	for(auto const& x:b){
		a.indent();
		a.o<<"<tr><td>\n";

		a.indent_level++;
		as_html(a,x);
		a.indent_level--;

		a.indent();
		a.o<<"</td></tr>";
	}
	a.indent_level--;

	a.indent();
	a.o<<"</table>\n";

	a.indent_level--;
	a.line("</td></tr></table>");
}

template<typename T>
void as_html(HTML_visitor &a,std::vector<T> const& b){
	a.line("<table><tr><td>");
	a.indent_level++;

	a.line("vector");

	a.indent_level--;
	a.line("</td><td>");
	a.indent_level++;

	a.indent();
	//a.o<<"<table border class='hidden'>\n";
	a.o<<"<table border>\n";
	a.indent_level++;
	for(auto const& x:b){
		a.indent();
		a.o<<"<tr><td>\n";
		a.indent_level++;
		as_html(a,x);
		a.indent_level--;
		a.indent();
		a.o<<"</td></tr>\n";
	}
	a.indent_level--;
	a.indent();
	a.o<<"</table>\n";
	/*(void)b;
	auto &o=a.o;
	auto name=a.name();
	a.indent();
	o<<tag("span onclick=\"toggle_viz('"+name+"')\"","vector")<<"\n";

	a.indent();
	o<<tag(
		"table border class='hidden' id=\""+name+"\"",
		tr(td("here"))
	)<<"\n";*/

	//o<<tag("div id=\""+name+"\"","div11");

	/*o<<"<button onclick=\"toggle_viz('content')\">Toggle content</button>";

	o<<tag(
		"table border",
		tr(tag("td onclick=\"toggle_viz('foo1')\"","elem1"))+
		tag("tr id=\"foo1\" class=\"hidden\"",td("normally hidden"))+
		tr(td("after"))
	);

	o<<"<div id=\"content\" class=\"hidden\">";
	o<<"this is hidden be default\n";
	o<<"</div>";*/

	a.indent_level--;
	a.line("</td></tr></table>");
}

void as_html(HTML_visitor &a,Run_result const& b){
	auto name=a.name();
	a.indent();
	a.o<<tag("span onclick=\"toggle_viz('"+name+"')\"","Run_result")<<"\n";
	//a.o<<"Run_result\n";

	a.indent();
	//a.o<<"<table border id='"+name+"' class='hidden'>\n";
	a.o<<"<table border id='"+name+"'>\n";
	a.indent_level++;
	#define X(A,B) {\
		auto name=a.name();\
		a.indent(); \
		a.o<<"<tr><td>\n"; \
		a.indent_level++;\
		a.indent(); \
		a.o<<tag("span onclick=\"toggle_viz('"+name+"')\"",""#B)<<"\n"; \
		a.indent();\
		a.o<<"<div class='hidden' id=\""<<name<<"\">\n";\
		a.indent_level++;\
		as_html(a,b.B); \
		a.indent_level--;\
		a.indent();\
		a.o<<"</div>\n";\
		a.indent_level--;\
		a.indent(); \
		a.o<<"</td></tr>\n";\
	}
	RUN_RESULT_ITEMS(X)
	#undef X
	a.indent_level--;
	a.o<<"</table>\n";
}

void table2(HTML_visitor &a,auto const& b,auto const& c){
	//a.line(table(tr(td(b)+td(c))));
	a.line("<table><tr><td>");
	as_html(a,b);
	a.line("</td><td>");
	as_html(a,c);
	a.line("</td></tr></table>");
}

struct Runner{
	HTML_visitor &out;

	Runner(HTML_visitor &a,std::string name):out(a){
		out.line("<table><tr><td>");
		out.indent_level++;
		out.line(name);

		out.indent_level--;
		out.line("</td><td>");
		out.indent_level++;

		out.line("<table border>");
		out.indent_level++;
	}

	~Runner(){
		out.indent_level--;
		out.line("</table>");

		out.indent_level--;
		out.line("</td></tr></table>");
	}

	void set(std::string name,auto value){
		out.line("<tr><td>");
		out.indent_level++;

		table2(out,name,value);
		//out.line(name);
		//as_html(out,value);

		out.indent_level--;
		out.line("</td></tr>");
	}
};

void as_html(HTML_visitor& a,Output_tuple const& b){
	Runner r(a,"Output_tuple");
	#define X(A,B) r.set(""#B,b.B);
	OUTPUT_TUPLE(X)
	#undef X
}

template<typename T>
void as_html(HTML_visitor& a,Rank_status<T> const& b){
	Runner r(a,"Rank_status");
	#define X(A,B) r.set(""#B,b.B);
	RANK_STATUS(X)
	#undef X
}

void as_html(HTML_visitor& a,Run_input const& b){
	Runner r(a,"Run_input");
	#define X(A,B) r.set(""#B,b.B);
	RUN_INPUT_ITEMS(X)
	#undef X
}

#define TO_HTML_INNER(A,B) r.set(""#B,b.B);

#define TO_HTML(NAME,ITEMS)\
	void as_html(HTML_visitor& a,NAME const& b){\
		Runner r(a,""#NAME);\
		ITEMS(TO_HTML_INNER)\
	}

TO_HTML(Team_status,TEAM_STATUS)
TO_HTML(Dcmp_data,DCMP_DATA)

int toggle_demo(){
	auto filename="tmp.html";
	{
		ofstream f(filename);
		HTML_visitor v(f);
		auto title_str="Example title";
		f<<"<html>\n";
		f<<"<head>\n";
		f<<title(title_str);
		f<<"<style>\n";
		f<<".hidden { display: none; }\n";
		f<<"</style>\n";
		f<<"<script>\n";
		f<<"function toggle_viz(name){\n";
		f<<"  var content=document.getElementById(name);\n";
		f<<"  content.classList.toggle(\"hidden\");\n";
		f<<"}\n";
		f<<"</script>\n";
		f<<"</head>";
		f<<"<body>\n";
		f<<h1(title_str)<<"\n";
		as_html(v,"foo");
		f<<"\n\n";
		auto x=range(20);
		as_html(v,x);

		auto x2=rand((Run_result*)0);
		as_html(v,x2);

		as_html(v,rand((Rank_status<Tournament_status>*)0));
		as_html(v,rand((Run_input*)0));

		f<<"\n\n";
		f<<"</body>\n";
		f<<"</html>\n";
	}
	return system(string()+"firefox "+filename);
}
