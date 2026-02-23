#include "plot.h"
#include<fstream>
#include<boost/beast/core/detail/base64.ipp>
#include "rand.h"
#include "subprocess.h"
#include "vector_void.h"
#include "io.h"

using namespace std;

PRINT_STRUCT(Plot_setup,PLOT_SETUP)

ELEMENTWISE_RAND(Plot_setup,PLOT_SETUP)

Job job(Plot_setup const& a){
	//PRINT(a);
	stringstream ss;
	for(auto [k,v]:a.data){
		ss<<k<<","<<v<<"\n";
	}
	return Job("./plot.py",{"--x","Points","--y","Probability"},ss.str());
}

auto base64_encode(std::string const& in){
	auto target_size=boost::beast::detail::base64::encoded_size(in.size());
	vector<char> v(target_size);
	auto enc=boost::beast::detail::base64::encode(&v[0],in.c_str(),in.size());
	return std::string(v.begin(),v.begin()+enc);
}

std::string plot(std::vector<std::pair<int,double>> const& data,std::optional<std::string> title){
	//returns an HTML tag that will show the scatter plot.
	(void)title;

	auto r=run(
		"./plot.py",
		{"--x","Points","--y","Probability"},
		[=](){
			std::stringstream ss;
			for(auto [x,y]:data){
				ss<<x<<","<<y<<"\n";
			}
			return ss.str();
		}()
	);
	if(r.status){
		//PRINT(r);
		//throw "failed to plot";
		return "";
	}

	auto p=base64_encode(r.out);
	stringstream out;
	out<<"<img src=\"data:image/png;base64, "<<p<<"\"";
	if(title){
		out<<" alt=\""<<title<<"\"";
	}
	out<<">\n";
	return out.str();
}

std::vector<std::string> plot(std::vector<Plot_setup> const& a){
	auto jobs=mapf(job,a);
	auto result=run_jobs(jobs);
	//status,out,error
	auto error=mapf([](auto x){ return x.status!=0; },result);
	return mapf(
		[](auto pair)->string{
			auto [in,x]=pair;
			if(x.status){
				//plotting failed.  Just return an empty string.
				return "";
			}
			auto p=base64_encode(x.out);
			stringstream out;
			out<<"<img src=\"data:image/png;base64, "<<p<<"\"";
			if(in.title){
				out<<" alt=\""<<in.title<<"\"";
				out<<" title=\""<<in.title<<"\"";
			}
			out<<">\n";
			return out.str();
		},
		zip(a,result)
	);
}

int main1(){
	std::vector<std::pair<int,double>> data;
	for(auto i:range(1000)){
		auto n=i-500;
		data|=make_pair(i,n*n);
	}

	auto p=plot(data);

	ofstream f("out.html");
	f<<"<html>\n";
	f<<"<body>\n";
	f<<p;
	f<<"</body>\n";
	f<<"</html>\n";
	return 0;
}

/*void plot_demo(){
	cout<<"Plot demo\n";
	vector<Plot_setup> v;
	for(auto _:range(50)){
		v|=rand((Plot_setup*)0);
	}

	plot(v);
	exit(0);
}*/

