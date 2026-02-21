#include "arguments.h"
#include "io.h"
#include "vector_void.h"
#include "vector.h"

using namespace std;

template<typename T>
T min(T a,T b,T c){
	return std::min(
		std::min(a,b),
		c
	);
}

size_t levenshtein_distance(std::string a,std::string b){
	//Warning: This is written with no regard to performance.
	//Will explode on large input.

	if(a.empty()) return b.size();
	if(b.empty()) return a.size();

	auto a0=a[0];
	auto ar=a.substr(1,a.size());
	auto b0=b[0];
	auto br=b.substr(1,b.size());

	if(a0==b0){
		return levenshtein_distance(ar,br);
	}
	
	return 1+min(
		//insert
		levenshtein_distance(a,br),

		//delete
		levenshtein_distance(ar,b),

		//replace
		levenshtein_distance(ar,br)
	);
}

bool decode(span<char*> s,bool const*){
	if(s.empty()){
		return 1;
	}
	nyi
}

string decode(span<char*> s,string const*){
	assert(s.size()==1);
	assert(s[0]);
	return s[0];
}

tba::District_key decode(span<char*> s,tba::District_key const*){
	assert(s.size()==1);
	assert(s[0]);
	return tba::District_key{s[0]};
}

tba::Year decode(span<char*> s,tba::Year const*){
	assert(s.size());
	assert(s[0]);
	return tba::Year{stoi(s[0])};
}

struct Flag{
	string name;
	vector<string> args;
	string help;
	std::function<void(std::span<char*>)> set;
};

struct Argument_parser::Impl{
	string description;
	vector<Flag> flags;

	void help(char **argv)const{
		cout<<argv[0];
		for(auto const& flag:flags){
			cout<<" ["<<flag.name;
			for(auto arg:flag.args){
				cout<<" "<<arg;
			}
			cout<<"]";
		}
		cout<<"\n\n";

		cout<<description<<"\n\n";

		for(auto const& flag:flags){
			cout<<flag.name;
			for(auto x:flag.args) cout<<" "<<x;
			cout<<"\n";
			cout<<"\t"<<flag.help<<"\n";
		}
		cout<<"--help\n";
		cout<<"\tShow this message.\n";
		exit(0);
	}

	Flag *find(string const& s){
		for(auto &flag:flags){
			if(flag.name==s){
				return &flag;
			}
		}
		return nullptr;
	}
};

Argument_parser::Argument_parser(string s):
	impl(new Argument_parser::Impl{std::move(s),vector<Flag>{}})
{}

Argument_parser::~Argument_parser()=default;

void Argument_parser::add(
	std::string a,
	vector<string> b,
	string c,
	std::function<void(std::span<char*>)> f
){
	impl->flags|=Flag{std::move(a),std::move(b),std::move(c),std::move(f)};
}

void Argument_parser::parse(int argc,char **argv){
	set<string> used;
	for(int i=1;i<argc;){
		string s=argv[i];
		if(used.count(s)){
			cerr<<"Already set:"<<s<<"\n";
			exit(1);
		}
		used|=s;

		if(s=="--help"){
			impl->help(argv);
		}
		auto f=impl->find(s);
		if(!f){
			cerr<<"Error: Unrecognized argument: "<<argv[i]<<"\n";
	
			auto flags=mapf([](auto x){ return x.name; },impl->flags);
			auto m=sort_by(
				flags,
				[=](auto x){ return make_pair(levenshtein_distance(argv[i],x),x); }
			);
			cerr<<"\nAvailable arguments:\n";
			for(auto const& elem:m){
				cout<<"\t"<<elem<<"\n";
			}
			exit(1);
		}
		i++;

		unsigned available=argc-i;
		if(available<f->args.size()){
			auto missing=skip(available,f->args);
			cerr<<"Error: Missing argument";
			if(missing.size()>1) cerr<<"s";
			cerr<<" to "<<f->name<<":";
			for(auto x:missing) cerr<<" "<<x;
			cerr<<"\n";
			exit(1);
		}
		try{
			f->set(span{argv+i,argv+i+f->args.size()});
		}catch(std::invalid_argument const& e){
			cerr<<"Invalid argument to "<<f->name<<": "<<e.what()<<"\n";
			exit(1);
		}
		i+=f->args.size();
	}
}
