#include "plot.h"
#include<fstream>
#include<boost/beast/core/detail/base64.ipp>
#include<simdjson.h>
#include "rand.h"
#include "subprocess.h"
#include "vector_void.h"
#include "io.h"
#include "json.h"

using namespace std;

PRINT_STRUCT(Plot_lines,PLOT_LINES)

PRINT_STRUCT(Plot_setup,PLOT_SETUP)

ELEMENTWISE_RAND(Plot_setup,PLOT_SETUP)

Job job(Plot_lines const& a){
	return Job("./plot_lines.py",{},to_json(a));
}

Job job(Plot_setup const& a){
	if(std::holds_alternative<std::vector<Plot_point2>>(a.data)){
		auto const& g=std::get<std::vector<Plot_point2>>(a.data);
		stringstream ss;
		for(auto [k,v]:g){
			ss<<k<<","<<v<<"\n";
		}
		return Job("./plot.py",{"--x","Points","--y","Probability"},ss.str());
	}
	if(std::holds_alternative<std::vector<Plot_point3>>(a.data)){
		auto const& g=std::get<std::vector<Plot_point3>>(a.data);
		std::stringstream ss;
		for(auto [x,y,z]:g){
			ss<<x<<","<<y<<","<<z<<"\n";
		}
		return Job("./plot3.py",{"--x","Input (Points)","--y","Output (Points)","--z","Probability"},ss.str());
	}
	if(std::holds_alternative<Plot_lines>(a.data)){
		return job(std::get<Plot_lines>(a.data));
	}
	assert(0);
}

auto base64_encode(std::string const& in){
	auto target_size=boost::beast::detail::base64::encoded_size(in.size());
	vector<char> v(target_size);
	auto enc=boost::beast::detail::base64::encode(&v[0],in.c_str(),in.size());
	return std::string(v.begin(),v.begin()+enc);
}

std::string png_tag(std::string const& png_data,std::optional<std::string> const& title){
	std::stringstream ss;
	ss<<"<img src=\"data:image/png;base64, "<<base64_encode(png_data)<<"\"";
	if(title){
		ss<<" alt=\""<<title<<"\"";
	}
	ss<<">\n";
	return ss.str();
}

std::string plot(std::vector<std::pair<int,double>> const& data,std::optional<std::string> title){
	//returns an HTML tag that will show the scatter plot.

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
	return png_tag(r.out,title);
}

std::vector<std::string> plot(std::vector<Plot_setup> const& a){
	auto jobs=MAP(job,a);
	auto result=run_jobs(jobs);
	//status,out,error
	auto error=mapf([](auto x){ return x.status!=0; },result);
	return mapf(
		[](auto pair)->string{
			auto [in,x]=pair;
			if(x.status){
				//cerr<<"Warning: Plot failed: "<<pair<<"\n";
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
	for(auto i:range_st<1000>()){
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
	for(auto _:range_st<50>()){
		v|=rand((Plot_setup*)0);
	}

	plot(v);
	exit(0);
}*/

using SB=simdjson::builder::string_builder;

/*template<typename T>
std::string serialize(SB&,T const&){
	nyi
}*/

ELEMENTWISE_RAND(Plot_lines,PLOT_LINES)

/*void serialize(SB& sb,Plot_lines const& a){
	
}*/

STRUCT_TO_JSON1(Plot_lines,PLOT_LINES)

int plot_demo(){
	auto data=rand((Plot_lines*)0);
	Plot_setup ps{data,"example title"};
	auto p=plot({ps});
	assert(p.size()==1);
	auto h=p[0];
	static const std::string PATH="out.html";
	{
		ofstream f(PATH);
		f<<h;
	}
	system("firefox "+PATH);
	return 0;
}
