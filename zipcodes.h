#ifndef ZIPCODES_H
#define ZIPCODES_H

#include<vector>
#include<string>

//This is a listing of all of the zip codes in California.

std::vector<std::tuple<std::string,std::string,std::string>> const& zipcode_data();

#endif
