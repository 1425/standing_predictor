#ifndef PLOT_H
#define PLOT_H

#include<string>
#include<vector>
#include<optional>
#include<variant>
#include "util.h"

std::string png_tag(std::string const& png_data,std::optional<std::string> const& title);

std::string plot(std::vector<std::pair<int,double>> const&,std::optional<std::string> title=std::nullopt);

using Plot_point2=std::pair<int,double>;
using Plot_point3=std::tuple<int,int,double>;

using Plot_data=std::variant<
	std::vector<Plot_point2>,
	std::vector<Plot_point3>
>;

#define PLOT_SETUP(X)\
	X(Plot_data,data)\
	X(std::optional<std::string>,title)

STRUCT_DECLARE(Plot_setup,PLOT_SETUP)

//The reason that you might use this version is that these are parallelized.
//Gives like a 10x speedup.
std::vector<std::string> plot(std::vector<Plot_setup> const&);

#endif
