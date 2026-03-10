#include<fstream>
#include<cmath>
#include<boost/tokenizer.hpp>
#include "../tba/data.h"
#include "io.h"
#include "vector.h"
#include "vector_void.h"
#include "interval.h"
#include "optional.h"
#include "map.h"
#include "set_flat.h"
#include "probability.h"

using namespace std;

struct Line{
	tba::Team_key team;
	tba::Event_key event;
	std::string event_name;
	double pr;
	//interestingly, does not include cmp probability
};

std::optional<Line> parse_line(std::string const& a){
	if(a.empty()){
		return std::nullopt;
	}
	auto sp=split(a,',');
	assert(sp.size()>=4);
	std::string_view s;
	return Line{
		//tba::Team_ketba::decode2(std::string_view(sp[0]),(tba::Team_key*)0),
		//tba::decode2(s,(tba::Team_key*)0),
		tba::Team_key(stoi(sp[0])),
		tba::Event_key(string_view(sp[1])),
		sp[2],
		stof(last(sp))
	};
}

std::vector<Line> parse_file(std::string path){
	ifstream f(path);
	std::vector<Line> r;
	
	{
		std::string header;
		getline(f,header);
	}

	while(f.good()){
		std::string s;
		getline(f,s);
		auto p=parse_line(s);
		if(p){
			r|=p;
		}
	}
	return r;
}

void histogram(std::vector<double> a){
	static const int BOXES=100;
	auto lim=limits(a);
	auto box=[&](auto x){
		auto v=(x-lim.min)/lim.width();
		return int(v*BOXES);
	};
	auto c=count(mapf(box,a));
	auto labels=range(lim.min,lim.max,lim.width()/BOXES);
	auto z=zip(labels,values(c));
	//print_lines(z);
	for(auto [a,b]:z){
		cout<<a<<","<<b<<"\n";
	}
}

using Team=tba::Team_key;

void result_table(std::vector<std::pair<Team,double>> v){
	static const int BOXES=20;
	const int box_size=v.size()/BOXES+1;
	//std::sort(v.begin(),v.end(),[](auto a,auto b){ return a.second<b.second; },v);
	v=sort_by(v,[](auto x){ return x.second; });
	while(!v.empty()){
		auto here=take(box_size,v);
		v=skip(box_size,v);

		cout<<limits(seconds(here))<<"\t"<<here.size()<<"\t"<<take(5,firsts(here))<<"\n";
	}
}

void examine_file(){
	auto file=parse_file("../standing_predictor_output/2/results.csv");
	//would be interesting to know what the distribution of probabilities looks like
	//and how much they change each week
	//are the set of teams seen different? (and how?)
	auto prs=mapf([](auto x){ return x.pr; },file);
	PRINT(sum(prs));
	PRINT(limits(prs));
	PRINT(quartiles(prs));
	PRINT(deciles(prs));
	histogram(prs);
}

auto as_map(std::string path){
	return dict(mapf([](auto x){ return make_pair(x.team,x.pr); },parse_file(path)));
}

#define LINE2(X)\
	X(tba::Team_key,team)\
	X(tba::Event_key,event)\
	X(std::string,event_name)\
	X(Pr,dcmp)\
	X(Pr,cmp)\

STRUCT_DECLARE(Line2,LINE2)

PRINT_STRUCT(Line2,LINE2)

auto parse_line2(std::string const& path){
	ifstream file(path);
	string line;
	std::vector<Line2> r;

	{
		string header;
		getline(file,header);
	}

	while(getline(file,line)){
		boost::tokenizer<boost::escaped_list_separator<char>> tk(line);
		std::vector<std::string> fields;
		for (auto i = tk.begin(); i != tk.end(); ++i) {
			fields.push_back(*i);
		}

		r|=Line2{
			tba::Team_key(stoi(fields[0])),
			tba::Event_key(fields[1]),
			fields[2],
			stod(fields[3]),
			stod(fields[4])
		};
	}
	return r;
}

int main1(){
	auto f1="../standing_predictor_output/0/results.csv";
	auto x1=as_map(f1);
	auto x2=as_map("../standing_predictor_output/4/results.csv");
	
	auto k1=keys(x1);
	auto k2=keys(x2);

	diff(k1,k2);
	//assert(k1==k2);

	auto m=sorted(mapf(
		[=](auto t){
			auto v1=x1.at(t);
			auto v2=x2.at(t);
			return make_tuple(v2-v1,t,v1,v2);
		},
		k1&k2
	));
	m=sort_by(m,[](auto x){ return fabs(std::get<0>(x)); });
	print_lines(m);

	auto diffs=mapf([](auto x){ return std::get<0>(x); },m);
	histogram(diffs);

	result_table(mapf([](auto x){ return make_pair(get<1>(x),get<0>(x)); },m));

	{
		ofstream f("diff.csv");
		for(auto [diff,team,v1,v2]:m){
			f<<team<<","<<v1<<","<<v2<<"\n";
		}
	}

	return 0;
}

int main(){
	/*auto p=parse_line2("results.csv");
	print_lines(p);*/
	return main1();
}
