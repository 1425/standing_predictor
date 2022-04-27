#include "arguments.h"
#include "util.h"

using namespace std;

Flag_base::Flag_base(string a,vector<string> b,string c):
	already_set(0),
	name(a),
	args(b),
	help(c)
{}

void Flag_base::set(std::span<char*> s){
	if(already_set){
		std::cerr<<"Error: Already set: "<<name<<"\n";
		exit(1);
	}
	already_set=1;
	set_inner(s);
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

Flag_base *Argument_parser::find(char *s){
	assert(s);
	for(auto &flag:flags){
		assert(flag);
		if(flag->name==s){
			return &*flag;
		}
	}
	return nullptr;
}

void Argument_parser::help(char **argv)const{
	cout<<argv[0];
	for(auto const& flag:flags){
		cout<<" ["<<flag->name;
		for(auto arg:flag->args){
			cout<<" "<<arg;
		}
		cout<<"]";
	}
	cout<<"\n\n";

	cout<<description<<"\n\n";

	for(auto const& flag:flags){
		cout<<flag->name;
		for(auto x:flag->args) cout<<" "<<x;
		cout<<"\n";
		cout<<"\t"<<flag->help<<"\n";
	}
	cout<<"--help\n";
	cout<<"\tShow this message.\n";
	exit(0);
}

Argument_parser::Argument_parser(string desc):description(desc){}

void Argument_parser::parse(int argc,char **argv){
	for(int i=1;i<argc;){
		if(argv[i]==string{"--help"}){
			help(argv);
		}
		auto f=find(argv[i]);
		if(!f){
			cerr<<"Error: Unrecognized argument: "<<argv[i]<<"\n";
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
