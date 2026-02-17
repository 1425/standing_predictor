#include<filesystem>
#include<fstream>
#include<span>
#include<gumbo.h>
#include "../tba/db.h"
#include "../tba/tba.h"
#include "arguments.h"
#include "set.h"
#include "map.h"
#include "tba.h"
#include "util.h"

using namespace std;

//start generic code

template<typename K,typename V>
map<K,V> remove_keys(map<K,V> m,set<K> s){
	for(auto elem:s){
		m.erase(elem);
	}
	return m;
}

vector<std::filesystem::directory_entry> to_vec(std::filesystem::directory_iterator a){
	return vector<std::filesystem::directory_entry>{begin(a),end(a)};
}

template<typename Func>
auto filter(Func f,std::filesystem::directory_iterator a){
	return filter(f,to_vec(a));
}

bool numeric(string const& s){
	try{
		stoi(s);
		return 1;
	}catch(...){
		return 0;
	}
}

/*bool suffix(string const& needle,string const& haystack){
	if(needle.size()>haystack.size()) return 0;
	return needle==haystack.c_str()+(haystack.size()-needle.size());
}*/

string rm_suffix(string const& sub,string const& whole){
	assert(suffix(sub,whole));
	return whole.substr(0,whole.size()-sub.size());
}

string take(size_t n,string const& s){
	return s.substr(0,n);
}

template<typename T>
span<T> skip(size_t n,span<T> s){
	if(n>=s.size()) return span<T>{};
	return span<T>{s.begin()+n,s.end()};
}

template<typename T>
vector<pair<size_t,T>> enumerate(vector<T>)nyi

template<typename A,typename B>
vector<pair<A,B>> zip(vector<A> const& a,span<B> b){
	vector<pair<A,B>> r;
	for(auto i:range(min(a.size(),b.size()))){
		r|=make_pair(a[i],b[i]);
	}
	return r;
}

template<typename T>
vector<pair<size_t,T>> enumerate(span<T> a){
	return zip(range(a.size()),a);
}

template<typename Func,typename T>
auto mapf(Func f,T const& t){
	vector<decltype(f(*begin(t)))> r;
	for(auto elem:t){
		r|=f(elem);
	}
	return r;
}

ostream& operator<<(ostream& o,pair<int,double> const& p){
	return o<<"("<<p.first<<","<<p.second<<")";
}

std::ostream& operator<<(std::ostream& o,GumboTag a){
	#define X(A) if(a==GUMBO_TAG_##A) return o<<""#A;
	X(HTML)
	X(HEAD)
	X(TITLE)
	X(BODY)
	X(H1)
	X(H2)
	X(TABLE)
	X(TR)
	X(TH)
	X(TD)
	X(BR)
	X(A)
	X(TBODY)
	#undef X
	return o<<"(other tag)";
}

std::ostream& operator<<(std::ostream&,GumboInternalNode const&);

std::ostream& operator<<(std::ostream& o,GumboVector const& a){
	o<<"[ ";
	for(auto i:range(a.length)){
		o<<*(GumboInternalNode*)a.data[i]<<" ";
	}
	return o<<"]";
}

std::ostream& operator<<(std::ostream& o,GumboElement const& a){
	o<<"GumboElement(";
	o<<a.tag<<" ";
	o<<a.children;
	return o<<")";
}

std::ostream& operator<<(std::ostream& o,GumboText const& a){
	return o<<a.text;
}

std::ostream& operator<<(std::ostream& o,GumboInternalNode const& a){
	switch(a.type){
		case GUMBO_NODE_DOCUMENT:
			nyi
		case GUMBO_NODE_ELEMENT:
			return o<<a.v.element;
		case GUMBO_NODE_TEXT:
			return o<<a.v.text;
		case GUMBO_NODE_CDATA:
			nyi
		case GUMBO_NODE_COMMENT:
			nyi
		case GUMBO_NODE_WHITESPACE:
			nyi
		case GUMBO_NODE_TEMPLATE:
			nyi
		default:
			PRINT(a.type);
			nyi
	}
}

//start program-specific code

using Team=tba::Team_key;
using Pr=double;

map<Team,Pr> parse_page(std::filesystem::directory_entry const& path){
	std::ifstream f(path.path());
	std::stringstream buffer;
	buffer<<f.rdbuf();
	GumboOutput* output = gumbo_parse(buffer.str().c_str());
	assert(output);
	assert(output->root);
	auto at=output->root;
	auto traverse_inner=[&](auto &at,auto name,int skip=0){
		assert(at);
		assert(at->type==GUMBO_NODE_ELEMENT);
		auto &from=at->v.element.children;
		for(auto i:range(from.length)){
			assert(from.data[i]);
			auto& here=*(GumboNode*)from.data[i];
			if(here.type==GUMBO_NODE_ELEMENT){
				auto x=here.v.element;
				if(x.tag==name){
					if(skip){
						skip--;
					}else{
						return &here;
					}
				}
			}
		}
		cout<<"Failed to find "<<name<<"\n";
		exit(1);
	};
	auto traverse=[&](auto name,int skip=0){
		at=traverse_inner(at,name,skip);
	};
	traverse(GUMBO_TAG_BODY);
	traverse(GUMBO_TAG_TABLE);
	traverse(GUMBO_TAG_TBODY);

	auto parse_row=[&](GumboNode *a){
		assert(a);
		auto v=a->v.element.children;
		assert(v.length==2);
		auto f=[=](int x){
			auto n=(GumboNode*)(v.data)[x];
			assert(n);
			assert(n->type==GUMBO_NODE_ELEMENT);
			auto c=n->v.element.children;
			assert(c.length==1);
			auto tn=(GumboNode*)(c.data)[0];
			assert(tn);
			assert(tn->type==GUMBO_NODE_TEXT);
			return tn->v.text.text;
		};
		return make_pair(stoi(f(0)),stod(f(1)));
	};

	auto dcmp_cutoff=mapf(
		parse_row,
		skip(1,span<GumboNode*>{
			(GumboNode**)(at->v.element.children.data),
			at->v.element.children.length
		})
	);
	//print_lines(v);

	at=output->root;
	traverse(GUMBO_TAG_BODY);
	auto pre_table=at;
	traverse(GUMBO_TAG_TABLE,1);
	traverse(GUMBO_TAG_TBODY);

	auto child=[](auto x){
		assert(x);
		auto c=x->v.element.children;
		assert(c.length);
		return (GumboNode*)(c.data)[0];
	};

	auto parse_team_row=[=](GumboNode *a)->pair<Team,double>{
		assert(a);
		auto v=a->v.element.children;
		//rank,p,team,name,...
		if(v.length<=3){
			throw std::invalid_argument("wrong table");
		}
		auto f=[=](int i){
			auto x=(GumboNode*)(v.data)[i];
			assert(x);
			auto c=x->v.element.children;
			assert(c.length==1);
			auto tn=(GumboNode*)(x->v.element.children.data)[0];
			assert(tn);
			//cout<<*tn<<"\n";
			return tn->v.text.text;
		};
		return make_pair(
			[=](){
				//nyi
				auto td=traverse_inner(a,GUMBO_TAG_TD,2);
				auto a=traverse_inner(td,GUMBO_TAG_A);
				auto tn=child(a);
				//PRINT(*tn)
				assert(tn->type==GUMBO_NODE_TEXT);
				auto t=tn->v.text.text;
				//PRINT(t);
				return Team{"frc"+as_string(stoi(t))};
			}(),
			stof(f(1))
		);
	};

	try{
		auto r=to_map(mapf(
			parse_team_row,
			skip(1,
				span<GumboNode*>{
					(GumboNode**)(at->v.element.children.data),
					at->v.element.children.length
				}
			)
		));

		gumbo_destroy_output(&kGumboDefaultOptions, output);

		return r;
	}catch(std::invalid_argument const&){
		at=pre_table;
		traverse(GUMBO_TAG_TABLE,2);
		traverse(GUMBO_TAG_TBODY);
		auto r=to_map(mapf(
			parse_team_row,
			skip(1,
				span<GumboNode*>{
					(GumboNode**)(at->v.element.children.data),
					at->v.element.children.length
				}
			)
		));

		gumbo_destroy_output(&kGumboDefaultOptions, output);

		return r;
	}
}

enum class Season_result{
	MISS_DCMP,DCMP,CMP
};

std::ostream& operator<<(std::ostream& o,Season_result a){
	switch(a){
		case Season_result::MISS_DCMP:
			return o<<"MISS_DCMP";
		case Season_result::DCMP:
			return o<<"DCMP";
		case Season_result::CMP:
			return o<<"CMP";
		default:
			assert(0);
	}
}

optional<map<Team,Season_result>> season_results(tba::District_key const& district){
	//do district at a time?
	auto f=TBA_fetcher_config{}.get();

	auto at_cmp=to_set(event_teams_keys(f,tba::Event_key{"2022cmptx"}));

	auto d=district_rankings(f,district);
	assert(d);
	bool any_dcmp=0;
	auto r=to_map(::mapf(
		[&](tba::District_Ranking const& a){
			auto team=a.team_key;
			//double pre_dcmp=a.rookie_bonus;
			double at_dcmp=0;
			for(auto x:a.event_points){
				if(x.district_cmp){
					any_dcmp=1;
					at_dcmp+=x.total;
				}else{
					//pre_dcmp+=x.total;
				}
			}
			return make_pair(
				team,
				[=](){
					if(at_dcmp==0) return Season_result::MISS_DCMP;
					if(at_cmp.count(team)) return Season_result::CMP;
					return Season_result::DCMP;
				}()
			);
		},
		*d
	));
	if(any_dcmp) return r;
	return std::nullopt;
}

double compare(map<Team,bool> a,map<Team,double> b){
	//a=actual
	//b=predicted
	{
		//make teams that do not appear in the predictions implicitly a 0% chance.
		auto a_only=keys(a)-keys(b);
		for(auto k:a_only){
			b[k]=0;
		}
	}

	{
		//make teams that do not appear in the final results as misses
		//this actually happens when teams are scheduled for events and then do not
		//appear in any.
		auto b_only=keys(b)-keys(a);
		for(auto k:b_only){
			a[k]=0;
		}
	}

	auto ak=keys(a);
	auto bk=keys(b);

	if(ak!=bk){
		auto a_only=ak-bk;
		if(a_only.size()){
			cout<<"Only in a:"<<remove_keys(a,bk)<<"\n";
		}
		PRINT(ak-bk);
		PRINT(bk-ak);

		for(auto k:bk-ak){
			cout<<k<<"\n";
			PRINT(a[k]);
			PRINT(b[k]);
		}
	}
	assert(ak==bk);
	return mean(mapf(
		[&](auto team){
			auto x=a[team]-b[team];
			return x*x;
		},
		ak
	));
}

int main1(int argc,char **argv){
	Argument_parser parser("Calculate Brier scores of previous predictions");
	tba::Year year{2022};
	parser.add("--year",{"YEAR"},"Year to analyze",year);
	parser.parse(argc,argv);

	map<tba::District_key,vector<std::filesystem::directory_entry>> m;
	for(auto subdir:
		//sorted(to_vec(std::filesystem::directory_iterator("results/2022")))
		sorted(filter(
			[](auto x){
				return x.is_directory() && numeric(std::filesystem::path(x).filename());
			},
			std::filesystem::directory_iterator("results/"+::as_string(year))
		))
	){
		for(auto x:
			filter(
				[](auto x){ return suffix(".html",x.path()); },
				std::filesystem::directory_iterator(subdir)
			)
		){
			assert(x.is_regular_file());
			auto base=rm_suffix(".html",x.path().filename());
			try{
				auto k=tba::District_key(base);
				m[k]|=x;
			}catch(...){
			}
		}
	}
	//print_lines(m);

	cout<<std::setprecision(2);

	for(auto [district,results]:m){
		//parse each of the pages
		auto predicted=::mapf(parse_page,results);
		//PRINT(predicted);

		//read the results from TBA
		auto s=season_results(district);
		if(s){
			//calculate Brier scores, etc.
			auto a=::mapf(
				[=](auto x){
					return compare(
						map_values([](auto x){ return x!=Season_result::MISS_DCMP; },*s),
						x
					);
				},
				predicted
			);
			cout<<district<<"\t"<<a<<"\n";
		}else{
			cout<<district<<"\tNo dcmp yet.\n";
		}
	}
	return 0;
}

int main(int argc,char **argv){
	try{
		return main1(argc,argv);
	}catch(std::invalid_argument const& a){
		cout<<a<<"\n";
		return 1;
	}
}
