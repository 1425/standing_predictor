#include<fstream>
#include "../tba/data.h"
#include "io.h"
#include "vector.h"
#include "vector_void.h"
#include "interval.h"
#include "optional.h"
#include "map.h"
#include "set_flat.h"

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
	static const int BOXES=10;
	auto lim=limits(a);
	auto box=[&](auto x){
		auto v=(x-lim.min)/lim.width();
		return int(v*BOXES);
	};
	auto c=count(mapf(box,a));
	auto labels=range(lim.min,lim.max,lim.width()/BOXES);
	auto z=zip(labels,values(c));
	print_lines(z);
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

int main(){
	auto f1="../standing_predictor_output/1/results.csv";
	auto x1=as_map(f1);
	auto x2=as_map("../standing_predictor_output/3/results.csv");
	
	auto k1=keys(x1);
	auto k2=keys(x2);
	assert(k1==k2);

	auto m=sorted(mapf(
		[=](auto t){
			auto v1=x1.at(t);
			auto v2=x2.at(t);
			return make_tuple(v2-v1,t,v1,v2);
		},
		k1&k2
	));
	print_lines(m);

	auto diffs=mapf([](auto x){ return std::get<0>(x); },m);
	histogram(diffs);
}
