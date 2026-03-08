#include "index.h"
#include<filesystem>
#include<unistd.h>
#include "io.h"
#include "tba.h"
#include "names.h"
#include "subprocess.h"

using namespace std;

auto link(std::filesystem::directory_entry const& ref,std::string const& body){
	return link(ref.path(),body);
}

static void chdir_throw(const char *s){
	int r=chdir(s);
	if(r!=0){
		throw "chdir failed";
	}
}

class chdir_tmp{
	//temporarily changes working directory, then changes back afterwards
	
	std::unique_ptr<char> old_path;

	public:
	explicit chdir_tmp(std::string const& new_path):
		old_path(get_current_dir_name())
	{
		assert(old_path);
		chdir_throw(new_path.c_str());
	}

	chdir_tmp(chdir_tmp const&)=delete;
	chdir_tmp& operator=(chdir_tmp const&)=delete;

	~chdir_tmp(){
		chdir_throw(old_path.get());
	}
};

static const std::string OUTPUT_BASE="../standing_predictor_output/";

std::filesystem::path next_dir(){
	for(auto i:range(1000)){
		std::filesystem::path path=OUTPUT_BASE+as_string(i)+"/";
		if(!std::filesystem::exists(path)){
			return path;
		}
	}
	throw "Out of paths";
}

void run_throw(string cmd,std::vector<std::string> const& args){
	auto a=run(cmd,args,"");
	if(a.status!=0){
		std::stringstream ss;
		ss<<"Command failed: "<<cmd<<args;
		ss<<a;
		throw ss.str();
	}
}

int index_page(TBA_fetcher& f){
	//Should eventually get back a page with references like:
	//https://htmlpreview.github.io/?https://raw.githubusercontent.com/1425/standing_predictor_output/refs/heads/main/1/2026ca.html

	//This is maybe not the cleanest way to do this
	//also, this makes this function not thread-safe.
	chdir_tmp chdir(OUTPUT_BASE);

	std::filesystem::path base{"."};

	using Entry=std::pair<std::string,std::filesystem::path>;
	using Dir=std::pair<std::string,std::vector<Entry>>;

	std::vector<Dir> found;
	for(auto const& entry:std::filesystem::directory_iterator{base}){
		bool dotfile=entry.path().filename().c_str()[0]=='.';
		if(dotfile){
			continue;
		}
		if(!entry.is_directory()){
			continue;
		}
		vector<Entry> found_here;
		for(auto const& e2:std::filesystem::directory_iterator{entry}){
			auto x=tba::District_key::parse(e2.path().filename().stem());
			if(x){
				found_here|=make_pair(name(f,*x),e2.path());
			}
		}
		found|=Dir(entry.path().filename(),found_here);
	}

	//TODO: Make the destination of this controllable.
	ofstream o("index.html");

	auto title_s="FRC 2026 Predictions";
	o<<"<html>\n";
	o<<tag("head",title(title_s));
	o<<"<body>\n";
	o<<h1(title_s)<<"\n";
	o<<"<table border>\n";
	for(auto [dir,items]:found){
		std::sort(items.begin(),items.end());

		o<<"<tr>\n";
		o<<td(dir);
		o<<"<td>\n";
		for(auto item:items){
			o<<link(item.second,item.first)<<"<br>\n";
		}
		o<<"</td>\n</tr>\n";
	}
	o<<"</table>\n";
	o<<"</body>\n</html>\n";
	return 0;
}

int index(TBA_fetcher &fetcher){
	//run once with no plotting --quick and --tba_refresh
	//run once with new output dir
	//run index
	//index(f);
	//do a git commit with the message of the date
	auto new_dir=next_dir();
	auto start_time=std::chrono::system_clock::now();

	//ought to just make this a function call instead.
	run_throw("./outline",{"--quick","1","--plot","0","--tba_refresh"});

	//ought to just make this a function call instead.
	run_throw("./outline",{"--dir",new_dir});

	//create index here
	//auto fetcher=TBA_fetcher_config{}.get();
	index_page(fetcher);

	chdir_tmp chdir_lock(OUTPUT_BASE);

	run_throw("git",{"add",new_dir.filename()});

	auto end_time=std::chrono::system_clock::now();
	cout<<"Elapsed time:"<<(end_time-start_time);

	run_throw("git",{"commit","-a","-m","Auto update "+as_string(start_time)});
	//run_throw("git",{"push"});

	return 0;
}
