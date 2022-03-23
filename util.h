#ifndef UTIL_H
#define UTIL_H

#include<cassert>
#include<set>
#include<sstream>
#include "../tba/util.h"

std::string slurp(std::string const& filename);

template<typename T>
void print_lines(T t){
	for(auto elem:t){
		std::cout<<elem<<"\n";
	}
}

template<typename T>
std::multiset<T>& operator|=(std::multiset<T>& a,T t){
	a.insert(t);
	return a;
}

template<typename T>
std::multiset<T>& operator|=(std::multiset<T>& a,std::multiset<T> b){
	for(auto elem:b){
		a|=elem;
	}
	return a;
}

template<typename T>
std::set<T>& operator|=(std::set<T>& a,T t){
	a.insert(t);
	return a;
}

template<typename K,typename V>
std::vector<V> seconds(std::map<K,V> a){
	std::vector<V> r;
	for(auto elem:a){
		r|=elem.second;
	}
	return r;
}

double sum(std::vector<double> v);

template<typename Func,typename T>
auto mapf(Func f,std::vector<T> const& v)->std::vector<decltype(f(v[0]))>{
	std::vector<decltype(f(v[0]))> r;
	for(auto elem:v){
		r|=f(elem);
	}
	return r;
}

template<typename Func,typename K,typename V>
auto mapf(Func f,std::map<K,V> const& v)->std::vector<decltype(f(*std::begin(v)))>{
	std::vector<decltype(f(*std::begin(v)))> r;
	for(auto p:v){
		r|=f(p);
	}
	return r;
}

template<typename K,typename V>
std::map<K,V> to_map(std::vector<std::pair<K,V>> v){
	std::map<K,V> r;
	for(auto p:v){
		auto f=r.find(p.first);
		assert(f==r.end());
		r[p.first]=p.second;
	}
	return r;
}

template<typename K,typename V>
auto values(std::map<K,V> a)->std::vector<V>{
	std::vector<V> r;
	for(auto p:a) r|=p.second;
	return r;
}

template<typename T>
std::map<T,size_t> count(std::multiset<T> const& a){
	std::map<T,size_t> r;
	for(auto elem:a){
		r[elem]=a.count(elem); //slow
	}
	return r;
}

template<typename Func,typename K,typename V>
auto map_values(Func f,std::map<K,V> m)->std::map<K,decltype(f(begin(m)->second))>{
	std::map<K,decltype(f(begin(m)->second))> r;
	for(auto [k,v]:m){
		r[k]=f(v);
	}
	return r;
}

template<typename T,typename Func>
std::vector<T> sorted(std::vector<T> v,Func f){
	sort(begin(v),end(v),[&](auto a,auto b){ return f(a)<f(b); });
	return v;
}

template<typename T>
std::vector<T> reversed(std::vector<T> a){
	reverse(begin(a),end(a));
	return a;
}

std::vector<std::string> split(std::string const&);

template<typename T>
std::string tag(std::string name,T body){
	std::stringstream ss;
	ss<<"<"<<name<<">"<<body<<"</"<<split(name).at(0)<<">";
	return ss.str();
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
std::string as_string(T t){
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

template<typename T>
auto tr(T t){ return tag("tr",t); }

template<typename T>
auto td(T t){ return tag("td",t); }

template<typename Func,typename A,typename B,typename C,typename D>
auto mapf(Func f,std::tuple<A,B,C,D> t)
	#define G(N) decltype(f(std::get<N>(t)))
	-> std::tuple<G(0),G(1),G(2),G(3)>
	#undef G
{
	return make_tuple(
		#define X(N) f(std::get<N>(t))
		X(0),X(1),X(2),X(3)
		#undef X
	);
}

template<typename Func,typename A,typename B,typename C,typename D,typename E>
auto mapf(Func f,std::tuple<A,B,C,D,E> t)
	#define G(N) decltype(f(std::get<N>(t)))
	-> std::tuple<G(0),G(1),G(2),G(3),G(4)>
	#undef G
{
	return make_tuple(
		#define X(N) f(std::get<N>(t))
		X(0),X(1),X(2),X(3),X(4)
		#undef X
	);
}

template<typename Func,typename A,typename B,typename C,typename D,typename E,typename F>
auto mapf(Func f,std::tuple<A,B,C,D,E,F> t)
	#define G(N) decltype(f(std::get<N>(t)))
	-> std::tuple<G(0),G(1),G(2),G(3),G(4),G(5)>
	#undef G
{
	return make_tuple(
		#define X(N) f(std::get<N>(t))
		X(0),X(1),X(2),X(3),X(4),X(5)
		#undef X
	);
}

#define MAP(F,X) mapf([&](auto a){ return (F)(a); },(X))

template<typename Func,typename T>
T filter_unique(Func f,std::vector<T> a){
	std::vector<T> found;
	for(auto elem:a){
		if(f(elem)){
			found|=elem;
		}
	}
	assert(found.size()==1);
	return found[0];
}

template<typename Func,typename A,typename B>
auto mapf(Func f,std::pair<A,B> p){
	return make_pair(
		f(p.first),
		f(p.second)
	);
}

template<typename A,typename B>
std::string join(std::pair<A,B> p){
	std::stringstream ss;
	ss<<p.first;
	ss<<p.second;
	return ss.str();
}

template<typename T>
std::string table(T body){ return tag("table",body); }

template<typename T>
auto h2(T t){ return tag("h2",t); }

template<typename T>
auto th(T t){ return tag("th",t); }

std::string th1(std::string const&);

template<typename A,typename B,typename C,typename D,typename E>
std::tuple<B,C,D,E> tail(std::tuple<A,B,C,D,E> const& t){
	return make_tuple(std::get<1>(t),std::get<2>(t),std::get<3>(t),std::get<4>(t));
}

template<typename T>
std::vector<T> operator+(std::vector<T> a,std::vector<T> b){
	for(auto elem:b){
		a|=elem;
	}
	return a;
}

template<typename T>
std::vector<T> operator+(std::vector<T> a,std::tuple<T,T,T,T> t){
	a|=std::get<0>(t);
	a|=std::get<1>(t);
	a|=std::get<2>(t);
	a|=std::get<3>(t);
	return a;
}

template<typename A,typename B,typename C,typename D,typename E>
std::tuple<A,B,C,D,E> operator|(std::tuple<A> a,std::tuple<B,C,D,E> b){
	return make_tuple(
		std::get<0>(a),
		std::get<0>(b),
		std::get<1>(b),
		std::get<2>(b),
		std::get<3>(b)
	);
}

template<typename A1,typename A,typename B,typename C,typename D,typename E>
std::tuple<A1,A,B,C,D,E> operator|(std::tuple<A1,A> a,std::tuple<B,C,D,E> b){
	return make_tuple(
		std::get<0>(a),
		std::get<1>(a),
		std::get<0>(b),
		std::get<1>(b),
		std::get<2>(b),
		std::get<3>(b)
	);
}

std::string link(std::string const& url,std::string const& body);

template<typename T>
std::vector<std::pair<size_t,T>> enumerate_from(size_t start,std::vector<T> v){
	std::vector<std::pair<size_t,T>> r;
	for(auto elem:v){
		r|=std::make_pair(start++,elem);
	}
	return r;
}

std::string td1(std::string const&);

template<typename T>
std::vector<T> operator+(std::vector<T> a,T b){
	a|=b;
	return a;
}

template<typename A,typename B,typename C,typename D,typename E>
std::vector<B> seconds(std::vector<std::tuple<A,B,C,D,E>> v){
	std::vector<B> r;
	for(auto t:v){
		r|=std::get<1>(t);
	}
	return r;
}

template<typename F,typename T>
std::vector<T> filter(F f,std::vector<T> v){
	std::vector<T> r;
	for(auto elem:v){
		if(f(elem)){
			r|=elem;
		}
	}
	return r;
}

template<typename T>
T choose(std::vector<T> v){
	return v[rand()%v.size()];
}

template<typename T>
std::vector<T> sorted(std::vector<T> a){
	std::sort(begin(a),end(a));
	return a;
}

template<typename T>
T max(std::vector<T> v){
	assert(v.size());
	T r=v[0];
	for(auto elem:v){
		r=std::max(r,elem);
	}
	return r;
}

template<typename T>
T min(std::vector<T> v){
	assert(v.size());
	T r=v[0];
	for(auto elem:v){
		r=std::min(r,elem);
	}
	return r;
}

double sum(std::vector<int> const&);

template<typename T>
double mean(std::vector<T> v){
	return sum(v)/v.size();
}

template<typename T>
T median(std::vector<T> v){
	assert(v.size());
	return sorted(v)[v.size()/2];
}

#endif
