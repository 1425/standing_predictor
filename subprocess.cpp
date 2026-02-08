#include "subprocess.h"
#include "vector_void.h"

std::ostream& operator<<(std::ostream& o,Subprocess_result const& a){
	o<<"Subprocess_result( ";
	#define X(A) o<<""#A<<":"<<a.A<<" ";
	X(status)
	X(out)
	X(error)
	#undef X
	return o<<")";
}

#ifndef __unix__

//Not even going to try to put together a Windows version of this file.
//Just put in do-nothing stubs.

Subprocess_result run(std::string prog,std::vector<std::string> const& args,std::string const& stdin_data){
	(void)prog;
	(void)args;
	(void)stdin_data;
	return Subprocess_result(1,"","Not implemented");
}

std::vector<Subprocess_result> run_jobs(std::vector<Job> const& a){
	return mapf(
		[](auto _){
			(void)_;
			return Subprocess_result(1,"","Not implemented");
		},
		a
	);
}
#else

#include<unistd.h>
#include<iostream>
#include<algorithm>
#include<cstdint>
#include<cstring>
#include<cassert>
#include<fcntl.h>
#include<signal.h>
#include<mutex>
#include<thread>
#include<sys/wait.h>
#include<sys/types.h>
#include<sys/signalfd.h>
#include "array.h"
#include "set_flat.h"
#include "util.h"
#include "run.h"
#include "queue.h"
#include "vector_void.h"

using namespace std;

void close_or_die(int fd){
	auto r=close(fd);
	if(r){
		perror("close");
		cerr<<"Error: close "<<fd<<"\n";
		exit(1);
	}
}

void close(std::array<int,2> p){
	mapf([](auto x){ close_or_die(x); },p);
}

template<typename T>
void close(std::vector<T> const& v){
	for(auto const& x:v){
		close(x);
	}
}

struct Signal_number{
	uint32_t data;
};

std::ostream& operator<<(std::ostream& o,Signal_number const& a){
	o<<"Signal_number(";
	#define X(A) if(a.data==A) return o<<""#A<<")";
	X(SIGCHLD)
	#undef X
	return o<<")";
}

std::ostream& operator<<(std::ostream& o,struct signalfd_siginfo const& a){
	o<<"signalfd_siginfo(\n";
	o<<"\t"<<Signal_number(a.ssi_signo)<<"\n";
	#define X(A) o<<"\t"#A<<":"<<a.A<<"\n";
	//X(ssi_signo)
	X(ssi_errno)
	X(ssi_code)
	X(ssi_pid)
	X(ssi_uid)
	X(ssi_fd)
	X(ssi_tid)
	X(ssi_band)
	X(ssi_overrun)
	X(ssi_trapno)
	X(ssi_status)
	X(ssi_int)
	X(ssi_ptr)
	X(ssi_utime)
	X(ssi_stime)
	X(ssi_addr)
	X(ssi_addr_lsb)
	//ignore pad
	#undef X
	return o<<")";
}

std::array<int,2> pipe(){
	std::array<int,2> a;
	int r=pipe(&a[0]);
	if(r){
		perror("pipe");
		throw "pipe failed";
	}
	return a;
}

void dup2_or_die(int a,int b){
	auto r=dup2(a,b);
	if(r==-1){
		cerr<<"Error: dup2\n";
		exit(1);
	}
}

void write_all(int fd,const char *s){
	auto len=strlen(s);
	while(len>0){
		ssize_t r=write(fd,s,len);
		if(r==-1){
			throw "write failed";
		}
		assert(r!=0);

		len-=r;
		s+=r;
	}
}

struct Select_info{
	set_flat<int> read,write,except;
};

std::ostream& operator<<(std::ostream& o,Select_info const& a){
	o<<"Select_results(";
	o<<a.read;
	o<<a.write<<a.except;
	return o<<")";
}

Select_info select(Select_info const& input){
	fd_set read_set,write_set,except_set;
	int nfds=0;
	auto init=[&](fd_set& set,auto const& to_add){
		FD_ZERO(&set);
		for(auto fd:to_add){
			FD_SET(fd,&set);
			nfds=max(nfds,fd+1);
		}
	};
	init(read_set,input.read);
	init(write_set,input.write);
	init(except_set,input.except);
	//PRINT(nfds);
	{
		int r=select(nfds,&read_set,&write_set,&except_set,NULL);
		if(r==-1){
			perror("select");
			exit(1);
		}
		assert(r!=-1);
	}

	Select_info r;
	auto read=[](fd_set& set,auto const& to_check,auto &out){
		for(auto fd:to_check){
			if(FD_ISSET(fd,&set)){
				out|=fd;
			}
		}
	};
	read(read_set,input.read,r.read);
	read(write_set,input.write,r.write);
	read(except_set,input.except,r.except);
	return r;
}

void set_nonblocking(int fd){
	int flags=fcntl(fd,F_GETFL,0);
	if(flags==-1){
		throw "failed to get flags";
	}
	flags|=O_NONBLOCK;
	if(fcntl(fd,F_SETFL,flags)==-1){
		throw "failed to set flags";
	}
}

//this is meant to be used to keep multiple things from trying to do things with signalfd at the same time.
std::mutex signalfd_lock;

Subprocess_result run(std::string prog,std::vector<std::string> const& args,std::string const& stdin_data){
	{
		auto r=signal(SIGPIPE,SIG_IGN);
		if(r==SIG_ERR){
			perror("signal");
			exit(1);
		}
	}

	auto child_stdin=pipe();
	//set_nonblocking(child_stdin[1]);

	auto child_stdout=pipe();
	auto child_stderr=pipe();

	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask,SIGCHLD);

	{
		int r=sigprocmask(SIG_BLOCK,&mask,NULL);
		if(r==-1){
			perror("sigprocmask");
			exit(1);
		}
	}

	std::lock_guard<std::mutex> lock(signalfd_lock);

	int signalfd_fd=signalfd(-1,&mask,0);
	if(signalfd_fd==-1){
		perror("signalfd");
		exit(1);
	}

	pid_t f=fork();
	switch(f){
		case -1:
			throw "Error: Failed to fork.\n";
		case 0:{
			//child
			close_or_die(child_stdin[1]);
			close_or_die(child_stdout[0]);
			close_or_die(child_stderr[0]);
			close_or_die(signalfd_fd);

			dup2_or_die(child_stdin[0],0);
			dup2_or_die(child_stdout[1],1);
			dup2_or_die(child_stderr[1],2);

			std::vector<const char *> new_args;
			new_args|=prog.c_str();
			for(auto const& s:args){
				new_args|=s.c_str();
			}
			new_args|=nullptr;
			char * const *a=(char * const*)&(new_args[0]);
			int r=execve(prog.c_str(),a,NULL);
			perror("execve");
			cerr<<"Error: execve: "<<r<<"\n";
			exit(1);
		}
		default:
		       break;
	}

	//parent
	close_or_die(child_stdin[0]);
	close_or_die(child_stdout[1]);
	close_or_die(child_stderr[1]);

	const char *out_buf=stdin_data.c_str();

	Select_info select_in;
	select_in.read|=child_stdout[0];
	select_in.read|=child_stderr[0];
	select_in.read|=signalfd_fd;

	select_in.write|=child_stdin[1];

	select_in.except|=child_stdout[0];
	select_in.except|=child_stderr[0];
	select_in.except|=signalfd_fd;
	select_in.except|=child_stdin[1];

	std::stringstream from_stdout,from_stderr,from_signalfd;
	while(1){
		Select_info sr=select(select_in);

		auto check_reader=[&](int fd,std::stringstream &ss){
			if(!sr.read.count(fd)){
				return 0;
			}
			std::array<char,sizeof(struct signalfd_siginfo)> buf;
			ssize_t r=read(fd,&buf[0],buf.size());
			switch(r){
				case 0:
					//EOF
					select_in.read.erase(fd);
					select_in.except.erase(fd);
					close_or_die(fd);
					break;
				case -1:
					perror("read");
					exit(1);
				default:
					ss.write(&buf[0],r);
			}
			return 1;
		};
		if(check_reader(child_stdout[0],from_stdout)){
			continue;
		}
		if(check_reader(child_stderr[0],from_stderr)){
			continue;
		}
		if(check_reader(signalfd_fd,from_signalfd)){
			auto s=from_signalfd.str();
			auto found=*(struct signalfd_siginfo*)s.c_str();
			assert(found.ssi_signo==SIGCHLD);
			assert(found.ssi_pid==(unsigned)f);

			for(auto fd:select_in.read|select_in.except){
				close_or_die(fd);
			}

			{
				//still need to reap the child process
				int status;
				int r=waitpid(f,&status,0);
				assert(r==f);
				assert(status==found.ssi_status);
			}

			return Subprocess_result(found.ssi_status,from_stdout.str(),from_stderr.str());
		}
		if(sr.write.count(child_stdin[1])){
			int r=write(child_stdin[1],out_buf,strlen(out_buf));
			if(r==-1){
				switch(errno){
					case EPIPE:
						select_in.write.erase(child_stdin[1]);
						select_in.except.erase(child_stdin[1]);
						close_or_die(child_stdin[1]);
						break;
					default:
						perror("write");
						exit(1);
				}
			}else{
				assert(r>0);
				out_buf+=r;
				if(strlen(out_buf)==0){
					select_in.write.erase(child_stdin[1]);
					select_in.except.erase(child_stdin[1]);
					close_or_die(child_stdin[1]);
				}
			}
			continue;
		}
	}
}

#define JOB_STATUS(X)\
	X(pid_t,pid)\
	X(std::string,to_stdin)\
	X(int,fd_stdin)\
	X(int,fd_stdout)\
	X(int,fd_stderr)\
	X(std::optional<int>,result)\
	X(std::stringstream,from_stdout)\
	X(std::stringstream,from_stderr)

struct Job_status{
	JOB_STATUS(INST)

	/*std::string to_stdin;
	int fd_stdin,fd_stdout,fd_stderr;

	std::optional<int> result;//this is only filled in once the job is done.
	std::stringstream from_stdout,from_stderr;*/
};

std::ostream& operator<<(std::ostream& o,Job_status const& a){
	o<<"Job_status( ";
	#define X(A,B) o<<""#B<<":"<<a.B<<" ";
	JOB_STATUS(X)
	#undef X
	return o<<")";
}

void print_short(Job_status const& a){
	ostream& o=std::cout;
	o<<"("<<a.pid;
	o<<" in:"<<a.to_stdin.size();
	o<<" fin"<<a.fd_stdin;
	o<<" fout"<<a.fd_stdout;
	o<<" ferr"<<a.fd_stderr;
	o<<" res:"<<a.result;
	o<<" out:"<<a.from_stdout.str().size();
	o<<" err:"<<a.from_stderr.str().size();
	o<<")\n";
}

void close(Job_status const& a){
	if(a.fd_stdin!=-1){
		close_or_die(a.fd_stdin);
	}
	if(a.fd_stdout!=-1){
		close_or_die(a.fd_stdout);
	}
	if(a.fd_stderr!=-1){
		close_or_die(a.fd_stderr);
	}
}

void set_cloexec(int fd){
	int flags=fcntl(fd,F_GETFD);
	if(flags==-1){
		perror("fcntl");
		throw "fcntl fail";
	}
	flags|=FD_CLOEXEC;
	int r=fcntl(fd,F_SETFD,flags);
	if(r==-1){
		perror("fcntl");
		throw "fcntl fail";
	}
}

Job_status start_job(Job const& job){
	Job_status r;
	r.to_stdin=job.stdin_data;

	auto child_stdin=pipe();
	auto child_stdout=pipe();
	auto child_stderr=pipe();

	r.pid=fork();
	if(r.pid==-1){
		perror("fork");
		throw "fork failed";
	}
	if(r.pid==0){
		//child
		close_or_die(child_stdin[1]);
		close_or_die(child_stdout[0]);
		close_or_die(child_stderr[0]);

		dup2_or_die(child_stdin[0],0);
		dup2_or_die(child_stdout[1],1);
		dup2_or_die(child_stderr[1],2);

		std::vector<const char *> new_args;
		new_args|=job.cmd.c_str();
		for(auto const& s:job.args){
			new_args|=s.c_str();
		}
		new_args|=nullptr;
		char * const *a=(char * const*)&(new_args[0]);
		int r=execve(job.cmd.c_str(),a,NULL);
		perror("execve");
		cerr<<"Error: execve: "<<r<<"\n";
		exit(1);
	}

	//parent
	close_or_die(child_stdin[0]);
	close_or_die(child_stdout[1]);
	close_or_die(child_stderr[1]);

	//If these next few lines are not set then this will take so much longer
	//because each of the children will have a copy of the earlier pipes
	//which they theoretically could use, making it so that only the last 
	//process started can theoretically be done.
	set_cloexec(child_stdin[1]);
	set_cloexec(child_stdout[0]);
	set_cloexec(child_stderr[0]);

	r.fd_stdin=child_stdin[1];
	r.fd_stdout=child_stdout[0];
	r.fd_stderr=child_stderr[0];

	if(r.to_stdin.empty()){
		close_or_die(r.fd_stdin);
		r.fd_stdin=-1;
	}

	return r;
}

std::vector<Subprocess_result> run_jobs(std::vector<Job> const& jobs){
	/*Might make sense to have a map of fd -> what it is
	 * in order to reduce the number of linear lookups
	 * map<int,pair<int(index into jobs),which part?>>
	*/

	//set up all the pipes, etc.
	{
		auto r=signal(SIGPIPE,SIG_IGN);
		if(r==SIG_ERR){
			perror("signal");
			exit(1);
		}
	}

	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask,SIGCHLD);

	{
		int r=sigprocmask(SIG_BLOCK,&mask,NULL);
		if(r==-1){
			perror("sigprocmask");
			exit(1);
		}
	}

	std::lock_guard<std::mutex> lock(signalfd_lock);

	//the close on exec is important to prevent the signals from being sent to child processes
	//and effectively lost.
	int signalfd_fd=signalfd(-1,&mask,SFD_CLOEXEC);

	if(signalfd_fd==-1){
		perror("signalfd");
		exit(1);
	}
	/*
	 * Limit the number of jobs that are active at the same time for two reasons:
	 * 1) Memory use
	 * 2) Running out of file descriptors with which to talk to them
	 *
	 * #2 is actually a more pressing problem.
	 * */

	std::queue<Job> todo;
	todo|=jobs;

	vector<Job_status> m;

	auto active_jobs=[&](){
		size_t i=0;
		for(auto const& x:m){
			if(!x.result){
				i++;
			}
		}
		return i;
	};

	const auto MAX_JOBS=std::thread::hardware_concurrency();

	while(1){
		while(active_jobs()<MAX_JOBS && !todo.empty()){
			m|=start_job(pop(todo));
			//cout<<"started job "<<m.size()<<"\n";
		}

		//PRINT(active_jobs());

		/*if(active_jobs()<=4){
			for(auto const& x:m){
				if(x.result) continue;
				print_short(x);
			}
		}*/

		{
			auto results=mapf([](auto const& x){ return x.result; },m);
			if(all(results)){
				close(m);
				close_or_die(signalfd_fd);
				//done
				return mapf(
					[](auto const& x){
						return Subprocess_result(
							*x.result,
							x.from_stdout.str(),
							x.from_stderr.str()
						);
					},
					m
				);
			}
		}

		Select_info select_info;
		select_info.read|=signalfd_fd;
		select_info.except|=signalfd_fd;

		for(auto const& m1:m){
			if(m1.result){
				//already finished
				//cout<<"Finished...\n";
				continue;
			}

			if(m1.to_stdin.size()){
				assert(m1.fd_stdin!=-1);
				select_info.write|=m1.fd_stdin;
				select_info.except|=m1.fd_stdin;
			}

			if(m1.fd_stdout!=-1){
				select_info.read|=m1.fd_stdout;
				select_info.except|=m1.fd_stdout;
			}
			if(m1.fd_stderr!=-1){
				select_info.read|=m1.fd_stderr;
				select_info.except|=m1.fd_stderr;
			}
		}

		/*for(auto const& x:m){
			print_short(x);
		}*/
		//PRINT(select_info);

		auto sr=select(select_info);
		//PRINT(sr);
		if(!sr.except.empty()){
			nyi
		}

		for(auto& m1:m){
			if(m1.result){
				continue;
			}
			if(sr.write.count(m1.fd_stdin)){
				//cout<<"write\n";
				auto fd=m1.fd_stdin;
				int r=write(fd,m1.to_stdin.c_str(),m1.to_stdin.size());
				switch(r){
					case -1:
						//Error
						nyi
					case 0:
						//EOF
						nyi
					default:
						m1.to_stdin=m1.to_stdin.substr(r,m1.to_stdin.size());
						if(m1.to_stdin.empty()){
							close_or_die(m1.fd_stdin);
							m1.fd_stdin=-1;
						}
				}
			}
			auto do_read=[&](int &fd,std::stringstream &ss){
				if(!sr.read.count(fd)) return;
				//cout<<"Going to read "<<fd<<"\n";
				std::array<char,100*1024> buf;
				int r=read(fd,&buf[0],buf.size());
				switch(r){
					case 0:
						//EOF
						close_or_die(fd);
						fd=-1;
						break;
					case -1:
						perror("read");
						exit(1);
					default:
						ss.write(&buf[0],r);
				}
			};
			do_read(m1.fd_stdout,m1.from_stdout);
			do_read(m1.fd_stderr,m1.from_stderr);
		}

		if(sr.read.count(signalfd_fd)){
			struct signalfd_siginfo buf;
			int r=read(signalfd_fd,&buf,sizeof(buf));
			switch(r){
				case -1:
					nyi
				case 0:
					nyi
				case sizeof(buf):{
					assert(buf.ssi_signo==SIGCHLD);
					int found=0;
					for(auto &m1:m){
						if(m1.pid==(pid_t)buf.ssi_pid){
							found++;
							if(m1.result){
								//in this case, already found below.
								continue;
							}
							m1.result=buf.ssi_status;
							int status;
							//cout<<"Going to waitpid\n";
							int r=waitpid(m1.pid,&status,0);
							//cout<<"waited\n";
							assert(r!=-1);
							assert(status==m1.result);
						}
					}
					assert(found==1);

					if(active_jobs()){
						int status;
						int r=waitpid(-1,&status,WNOHANG);
						switch(r){
							case 0:
								//none found; go again
								break;
							case -1:
								//error
								perror("waitpid");
								assert(0);
							default:{
								//found something
								int found=0;
								for(auto &m1:m){
									if(m1.pid==r){
										found++;
										assert(!m1.result);
										m1.result=status;
										//could try to close the pipes if they
										//aren't already.
									}
								}
								if(found!=1){
									PRINT(found);
								}
								assert(found==1);
							}
						}
					}
					break;
				}
				default:
					//didn't think partial reads of this were possible
					nyi
			}
		}
		if(sr.except.count(signalfd_fd)){

			nyi
		}
	}
}
#endif
