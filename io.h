#ifndef IO_H
#define IO_H

#include<iostream>
#include<set>
#include<vector>
#include<optional>
#include<map>
#include<variant>
#include<sstream>
#include<chrono>

#define PRINT(X) { std::cout<<""#X<<":"<<(X)<<"\n"; }
#define nyi { std::cout<<"nyi "<<__FILE__<<":"<<__LINE__<<"\n"; exit(44); }

void indent(int levels);

std::vector<std::string> split(std::string const&);
std::vector<std::string> split(std::string const&,char);

//definition in map.h.
template<typename K,typename V>
std::ostream& operator<<(std::ostream&,std::map<K,V> const&);

template<typename A,typename B>
std::ostream& operator<<(std::ostream&,std::pair<A,B> const&);

template<typename T>
std::ostream& operator<<(std::ostream&,std::optional<T> const&);

template<typename...Ts>
std::ostream& operator<<(std::ostream&,std::variant<Ts...> const&);

template<typename T>
std::ostream& operator<<(std::ostream&,std::set<T> const&);

template<typename T>
std::ostream& operator<<(std::ostream&,std::vector<T> const&);

using Time_ns=std::chrono::duration<long int,std::ratio<1,1000*1000*1000>>;

std::ostream& operator<<(std::ostream&,Time_ns const&);

std::ostream& operator<<(std::ostream&,std::chrono::time_zone const&);
std::ostream& operator<<(std::ostream&,std::chrono::time_zone const * const);
std::ostream& operator<<(std::ostream&,std::stringstream const&);

template<typename A,typename B>
std::ostream& operator<<(std::ostream& o,std::tuple<A,B> const& t){
	o<<"(";
	o<<std::get<0>(t)<<" ";
	o<<std::get<1>(t);
	return o<<")";
}

template<typename A,typename B,typename C>
std::ostream& operator<<(std::ostream& o,std::tuple<A,B,C> const& t){
	o<<"(";
	o<<std::get<0>(t)<<" ";
	o<<std::get<1>(t)<<" ";
	o<<std::get<2>(t);
	return o<<")";
}

template<typename A,typename B,typename C,typename D>
std::ostream& operator<<(std::ostream& o,std::tuple<A,B,C,D> const& t){
	o<<"(";
	o<<std::get<0>(t)<<" ";
	o<<std::get<1>(t)<<" ";
	o<<std::get<2>(t)<<" ";
	o<<std::get<3>(t);
	return o<<")";
}

template<typename A,typename B,typename C,typename D,typename E>
std::ostream& operator<<(std::ostream& o,std::tuple<A,B,C,D,E> const& t){
	o<<"(";
	o<<std::get<0>(t)<<" ";
	o<<std::get<1>(t)<<" ";
	o<<std::get<2>(t)<<" ";
	o<<std::get<3>(t)<<" ";
	o<<std::get<4>(t);
	return o<<")";
}

template<typename T>
std::ostream& operator<<(std::ostream& o,std::vector<T> const& v){
	o<<"[ ";
	for(auto const& x:v){
		o<<x<<" ";
	}
	return o<<"]";
}

template<typename A,typename B>
std::ostream& operator<<(std::ostream& o,std::pair<A,B> const& a){
	return o<<"("<<a.first<<","<<a.second<<")";
}

template<typename T>
std::ostream& operator<<(std::ostream& o,std::optional<T> const& a){
	if(a) return o<<*a;
	return o<<"NULL";
}

std::ostream& operator<<(std::ostream&,std::invalid_argument const&);

template<typename T>
void print_lines(T const& t){
	for(auto elem:t){
		std::cout<<elem<<"\n";
	}
}

template<typename T,size_t N>
std::ostream& operator<<(std::ostream& o,std::array<T,N> const& a){
	o<<"[ ";
	for(auto elem:a){
		o<<elem<<" ";
	}
	return o<<"]";
}

template<typename...Ts>
std::ostream& operator<<(std::ostream& o,std::variant<Ts...> const& a){
	std::visit([&](auto const& x){ o<<x; },a);
	return o;
}

template<typename T>
std::string as_string(T const& t){
	std::stringstream ss;
	ss<<t;
	return ss.str();
}

template<typename T>
std::string join(std::vector<T> const& a){
	std::stringstream ss;
	for(auto elem:a){
		ss<<elem;
	}
	return ss.str();
}

template<typename T,size_t N>
std::string join(std::array<T,N> const& a){
	std::stringstream ss;
	for(auto const& x:a){
		ss<<x;
	}
	return ss.str();
}

template<typename A,typename B,typename C,typename D,typename E>
std::string join(std::tuple<A,B,C,D,E> const& t){
	std::stringstream ss;
	#define X(N) ss<<std::get<N>(t);
	X(0) X(1) X(2) X(3) X(4)
	#undef X
	return ss.str();
}

template<typename A,typename B,typename C,typename D,typename E,typename F>
std::string join(std::tuple<A,B,C,D,E,F> const& t){
	std::stringstream ss;
	#define X(N) ss<<std::get<N>(t);
	X(0) X(1) X(2) X(3) X(4) X(5)
	#undef X
	return ss.str();
}

template<typename A,typename B>
std::string join(std::pair<A,B> const& p){
	std::stringstream ss;
	ss<<p.first;
	ss<<p.second;
	return ss.str();
}

template<typename T>
std::string tag(std::string const& name,T const& body){
	std::stringstream ss;
	ss<<"<"<<name<<">"<<body<<"</"<<split(name).at(0)<<">";
	return ss.str();
}

template<typename T>
auto tr(T t){ return tag("tr",t); }

template<typename T>
auto td(T t){ return tag("td",t); }

std::string td1(std::string const&);

template<typename T>
std::string table(T const& body){ return tag("table",body); }

template<typename T>
auto h2(T const& t){ return tag("h2",t); }

template<typename T>
auto h3(T const& t){ return tag("h3",t); }

template<typename T>
auto th(T const& t){ return tag("th",t); }

std::string th1(std::string const&);

std::string link(std::string const& url,std::string const& body);

template<typename A,typename B,typename C,typename D,typename E,typename F>
std::ostream& operator<<(std::ostream& o,std::tuple<A,B,C,D,E,F> const& a){
	o<<"t(";
	#define X(N) o<<get<N>(a)<<"/";
	X(0) X(1) X(2) X(3) X(4) X(5)
	#undef X
	return o<<")";
}

namespace tba{
	struct Event_key;
	struct Event;
};

std::string link(tba::Event_key const&,std::string const&);
std::string link(tba::Event const&,std::string const&);

#endif
