#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include<string>
#include<vector>
#include<span>
#include<memory>
#include<functional>
#include "../tba/data.h"

std::string decode(std::span<char*>,std::string const*);
tba::District_key decode(std::span<char*>,tba::District_key const*);
tba::Year decode(std::span<char*>,tba::Year const*);
bool decode(std::span<char*>,bool const*);

template<typename T>
std::optional<T> decode(std::span<char*> s,std::optional<T> const*){
	return decode(s,(T*)0);
}

class Argument_parser{
	class Impl;
	std::unique_ptr<Impl> impl;

	void add(
		std::string,
		std::vector<std::string>,
		std::string,
		std::function<void(std::span<char*>)>
	);

	public:
	explicit Argument_parser(std::string);
	~Argument_parser();

	template<typename T>
	void add(std::string name,std::vector<std::string> args,std::string help,T& out){
		add(
			std::move(name),
			std::move(args),
			std::move(help),
			[&](std::span<char*> s){ out=decode(s,&out); }
		);
	}

	void parse(int argc,char **argv);
};

#endif
