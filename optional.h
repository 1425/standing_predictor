#ifndef OPTIONAL_H
#define OPTIONAL_H

#include<optional>

template<typename T>
std::vector<T>& operator|=(std::vector<T> &a,std::optional<T> const& b){
	if(b){
		a|=*b;
	}
	return a;
}

template<typename T>
auto flatten(std::vector<std::optional<T>> a){
	std::vector<T> r;
	for(auto elem:a){
		r|=elem;
	}
	return r;
}

template<typename T>
bool operator==(std::optional<T> const& a,std::optional<T> const& b){
	if(a){
		if(b){
			return *a==*b;
		}
		return 0;
	}
	return !b;
}

template<typename T>
std::vector<T> nonempty(std::vector<std::optional<T>> const& a){
	std::vector<T> r;
	for(auto elem:a){
		if(elem){
			r|=*elem;
		}
	}
	return r;
}

template<typename T>
std::vector<T> operator|(std::vector<T>,std::optional<T>);


#endif
