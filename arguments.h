#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include<string>
#include<vector>
#include<span>
#include<iostream>
#include<memory>
#include "../tba/data.h"

class Flag_base{
	bool already_set;

	public:
	std::string name;
	std::vector<std::string> args;
	std::string help;

	Flag_base(std::string,std::vector<std::string>,std::string);

	void set(std::span<char*>);
	virtual void set_inner(std::span<char*>)=0;
};

std::string decode(std::span<char*>,std::string const*);
tba::District_key decode(std::span<char*>,tba::District_key const*);
tba::Year decode(std::span<char*> s,tba::Year const*);

template<typename T>
std::optional<T> decode(std::span<char*> s,std::optional<T> const*){
	return decode(s,(T*)0);
}

template<typename T>
class Flag:public Flag_base{
	T *data;//not owned!

	public:
	explicit Flag(std::string name,std::vector<std::string> args,std::string help,T &data1):
		Flag_base(name,args,help),
		data(&data1)
	{}

	void set_inner(std::span<char*> s){
		*data=decode(s,(T*)0);
	}
};

class Argument_parser{
	std::string description;
	std::vector<std::unique_ptr<Flag_base>> flags;

	Flag_base *find(char*);

	void help(char **argv)const;

	public:
	explicit Argument_parser(std::string);

	template<typename T>
	void add(std::string name,std::vector<std::string> args,std::string help,T& out){
		flags.push_back(make_unique<Flag<T>>(name,args,help,out));
	}

	void parse(int argc,char **argv);
};

#endif
