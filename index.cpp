#include "index.h"
#include<filesystem>
#include<unistd.h>
#include "io.h"
#include "tba.h"
#include "names.h"

using namespace std;

auto link(std::filesystem::directory_entry const& ref,std::string const& body){
	return link(ref.path(),body);
}

int index(TBA_fetcher& f){
	//Should eventually get back a page with references like:
	//https://htmlpreview.github.io/?https://raw.githubusercontent.com/1425/standing_predictor_output/refs/heads/main/1/2026ca.html

	//This is maybe not the cleanest way to do this
	//as we are not ever restoring the current directory after the end of this function.
	{
		int r=chdir("../standing_predictor_output/");
		assert(r==0);
	}

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

