#ifndef UTIL_H
#define UTIL_H

#include<cassert>
#include<set>
#include<sstream>
#include<iostream>
#include<vector>
#include<map>
#include<optional>
#include<algorithm>

#define PRINT(X) { std::cout<<""#X<<":"<<(X)<<"\n"; }
#define nyi { std::cout<<"nyi "<<__FILE__<<":"<<__LINE__<<"\n"; exit(44); }

std::string slurp(std::string const& filename);

template<typename T>
void print_lines(T const& t){
	for(auto elem:t){
		std::cout<<elem<<"\n";
	}
}

template<typename T>
std::vector<T>& operator|=(std::vector<T> &a,T t){
	a.push_back(t);
	return a;
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
	for(auto x:v){
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
std::vector<T> operator+(std::vector<T> a,std::vector<T> const& b){
	a.insert(a.end(),b.begin(),b.end());
	return a;
}

template<typename T>
std::vector<T> operator+(std::vector<T> a,std::tuple<T,T,T,T> const& t){
	a|=std::get<0>(t);
	a|=std::get<1>(t);
	a|=std::get<2>(t);
	a|=std::get<3>(t);
	return a;
}

template<typename A,typename B,typename C,typename D,typename E>
std::tuple<A,B,C,D,E> operator|(std::tuple<A> const& a,std::tuple<B,C,D,E> const& b){
	return make_tuple(
		std::get<0>(a),
		std::get<0>(b),
		std::get<1>(b),
		std::get<2>(b),
		std::get<3>(b)
	);
}

template<typename A1,typename A,typename B,typename C,typename D,typename E>
std::tuple<A1,A,B,C,D,E> operator|(std::tuple<A1,A> const& a,std::tuple<B,C,D,E> const& b){
	return make_tuple(
		std::get<0>(a),
		std::get<1>(a),
		std::get<0>(b),
		std::get<1>(b),
		std::get<2>(b),
		std::get<3>(b)
	);
}

template<typename T>
std::vector<T> operator+(std::vector<T> a,T b){
	a|=b;
	return a;
}

template<typename K,typename V>
std::vector<V> seconds(std::map<K,V> const& a){
	std::vector<V> r;
	for(auto elem:a){
		r|=elem.second;
	}
	return r;
}

double sum(std::vector<double> const& v);

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

template<typename Func,typename A,typename B,typename C,typename D>
auto mapf(Func f,std::tuple<A,B,C,D> const& t)
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
auto mapf(Func f,std::tuple<A,B,C,D,E> const& t)
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
auto mapf(Func f,std::tuple<A,B,C,D,E,F> const& t)
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

template<typename Func,typename A,typename B>
auto mapf(Func f,std::pair<A,B> const& p){
	return make_pair(
		f(p.first),
		f(p.second)
	);
}

#define MAP(F,X) ::mapf([&](auto a){ return (F)(a); },(X))

template<typename Func,typename K,typename V>
auto map_values(Func f,std::map<K,V> m)->std::map<K,decltype(f(begin(m)->second))>{
	std::map<K,decltype(f(begin(m)->second))> r;
	for(auto [k,v]:m){
		r[k]=f(v);
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

void indent(int levels);

std::vector<std::string> split(std::string const&);
std::vector<std::string> split(std::string const&,char);

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
auto th(T const& t){ return tag("th",t); }

std::string th1(std::string const&);

std::string link(std::string const& url,std::string const& body);

template<typename A,typename B,typename C,typename D,typename E>
std::tuple<B,C,D,E> tail(std::tuple<A,B,C,D,E> const& t){
	return make_tuple(std::get<1>(t),std::get<2>(t),std::get<3>(t),std::get<4>(t));
}

template<typename T>
std::vector<std::pair<size_t,T>> enumerate_from(size_t start,std::vector<T> const& v){
	std::vector<std::pair<size_t,T>> r;
	for(auto elem:v){
		r|=std::make_pair(start++,elem);
	}
	return r;
}

template<typename T>
auto enumerate(T const& t){
	return enumerate_from(0,t);
}

template<typename A,typename B,typename C,typename D,typename E>
std::vector<B> seconds(std::vector<std::tuple<A,B,C,D,E>> const& v){
	std::vector<B> r;
	for(auto t:v){
		r|=std::get<1>(t);
	}
	return r;
}

template<typename A,typename B,typename C,typename D,typename E,typename F,typename G,typename H,typename I>
std::vector<B> seconds(std::vector<std::tuple<A,B,C,D,E,F,G,H,I>> const& v){
	std::vector<B> r;
	for(auto t:v){
		r|=std::get<1>(t);
	}
	return r;
}

template<typename F,typename T>
std::vector<T> filter(F f,std::vector<T> const& v){
	std::vector<T> r;
	std::copy_if(v.begin(),v.end(),std::back_inserter(r),f);
	return r;
}

#define FILTER(A,B) filter([&](auto x){ return (A)(x); },(B))

template<typename Func,typename T>
T filter_unique(Func f,std::vector<T> const& a){
	auto found=filter(f,a);
	assert(found.size()==1);
	return found[0];
}

template<typename T>
T choose(std::vector<T> const& v){
	return v[rand()%v.size()];
}

template<typename T>
std::vector<T> sorted(std::vector<T> a){
	std::sort(begin(a),end(a));
	return a;
}

template<typename T>
T max(std::vector<T> const& v){
	assert(v.size());
	T r=v[0];
	for(auto elem:v){
		r=std::max(r,elem);
	}
	return r;
}

template<typename T>
T min(std::vector<T> const& v){
	assert(v.size());
	T r=v[0];
	for(auto elem:v){
		r=std::min(r,elem);
	}
	return r;
}

double sum(std::vector<int> const&);

template<typename T>
double mean(std::vector<T> const& v){
	return sum(v)/v.size();
}

template<typename T>
T median(std::vector<T> const& v){
	assert(v.size());
	return sorted(v)[v.size()/2];
}

template<typename K,typename V>
std::vector<std::pair<K,V>> to_vec(std::map<K,V> const& m){
	return std::vector<std::pair<K,V>>{m.begin(),m.end()};
}

template<typename T>
std::vector<T> range(T start,T lim){
	std::vector<T> r;
	for(auto i=start;i<lim;i++){
		r|=i;
	}
	return r;
}

template<typename T>
std::vector<T> range(T lim){
	return range(T{0},lim);
}

template<typename T>
std::vector<T> skip(size_t n,std::vector<T> const& a){
	if(n>a.size()) return {};
	return std::vector<T>{a.begin()+n,a.end()};
}

template<typename T>
T last(std::vector<T> const& a){
	assert(a.size());
	return a[a.size()-1];
}

std::string tolower(std::string const&);
bool prefix(std::string const& whole,std::string const& p);

template<typename A,typename B>
std::vector<std::pair<A,B>> zip(std::vector<A> const& a,std::vector<B> const& b){
	return mapf(
		[&](auto i){ return std::make_pair(a[i],b[i]); },
		range(std::min(a.size(),b.size()))
	);
}

template<typename Func,typename T>
auto group(Func f,std::vector<T> const& v){
	using K=decltype(f(v[0]));
	std::map<K,std::vector<T>> r;
	for(auto x:v){
		r[f(x)]|=x;
	}
	return r;
}

template<typename T>
std::vector<T> range_inclusive(T start,T lim){
	std::vector<T> r;
	for(auto i=start;i<=lim;++i){
		r|=i;
		if(i==lim){
			return r;
		}
	}
	return r;
}

std::vector<char> to_vec(std::string const&);

template<typename T>
bool all_equal(std::vector<T> const& a){
	for(auto elem:a){
		if(elem!=a[0]){
			return 0;
		}
	}
	return 1;
}

template<typename Func,typename T>
std::vector<T> sort_by(Func f,std::vector<T> a){
	sort(
		a.begin(),
		a.end(),
		[=](auto a,auto b){
			return f(a)<f(b);
		}
	);
	return a;
}

template<typename K,typename V>
std::ostream& operator<<(std::ostream& o,std::map<K,V> const& a){
	return o<<to_vec(a);
}

template<typename K,typename V>
std::map<K,V>& operator+=(std::map<K,V>& a,std::map<K,V> const& b){
	for(auto [k,v]:b){
		a[k]+=v;
	}
	return a;
}

#endif
