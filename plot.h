#ifndef PLOT_H
#define PLOT_H

#include<string>
#include<vector>
#include<optional>
#include "util.h"

std::string plot(std::vector<std::pair<int,double>> const&,std::optional<std::string> title=std::nullopt);

using Plot_point=std::pair<int,double>;
using Plot_data=std::vector<Plot_point>;

#define PLOT_SETUP(X)\
	X(Plot_data,data)\
	X(std::optional<std::string>,title)

STRUCT_DECLARE(Plot_setup,PLOT_SETUP)

//The reason that you might use this version is that these are parallelized.
//Gives like a 10x speedup.
std::vector<std::string> plot(std::vector<Plot_setup> const&);

#endif
