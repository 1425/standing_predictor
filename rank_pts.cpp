#include "rank_pts.h"
#include<fstream>
#include<cassert>
#include<map>
#include<array>
#include "util.h"

//start generic code

template<size_t N,typename T>
std::array<T,N> to_array(std::vector<T> const& a){
	assert(a.size()==N);
	std::array<T,N> r;
	for(auto i:range(N)){
		r[i]=a[i];
	}
	return r;
}

template<typename Func,typename T,size_t N>
auto mapf(Func f,std::array<T,N> const& a){
	std::array<decltype(f(a[0])),N> r;
	for(auto i:range(N)){
		r[i]=f(a[i]);
	}
	return r;
}

//start program-specific code

using namespace std;

int rank_pts(unsigned teams,unsigned rank){
	//Teams=# of teams at the event
	assert(rank>0);
	assert(rank<=teams);

	static map<int,map<int,int>> cache;

	auto parse=[&](){
		//Could make this cache results.
		ifstream f("data/table/pts_"+as_string(teams)+".txt");
		map<int,int> m;
		while(f.good()){
			string s;
			getline(f,s);
			if(s.empty()){
				continue;
			}
			auto sp=split(s);
			assert(sp.size()==2);
			auto [k,v]=MAP(stoi,to_array<2>(sp));
			m[k]=v;
		}
		return m;
	};

	auto f=cache.find(teams);
	if(f==cache.end()){
		cache[teams]=parse();
		f=cache.find(teams);
	}

	return f->second[rank];
}

