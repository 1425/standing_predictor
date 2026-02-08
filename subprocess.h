#ifndef SUBPROCESS_H
#define SUBPROCESS_H

#include<string>
#include<vector>

struct Subprocess_result{
	int status;
	std::string out,error;
};

std::ostream& operator<<(std::ostream&,Subprocess_result const&);

Subprocess_result run(std::string prog,std::vector<std::string> const& args,std::string const& stdin_data);

struct Job{
	std::string cmd;
	std::vector<std::string> args;
	std::string stdin_data;
};

std::vector<Subprocess_result> run_jobs(std::vector<Job> const&);

#endif
