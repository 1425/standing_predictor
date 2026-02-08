#ifndef QUEUE_H
#define QUEUE_H

#include<queue>

template<typename T>
T pop(std::queue<T> &a){
	auto r=a.front();
	a.pop();
	return r;
}

template<typename T>
std::queue<T>& operator|=(std::queue<T>& a,T const& b){
	a.push(b);
	return a;
}

template<typename T>
std::queue<T>& operator|=(std::queue<T>& a,std::vector<T> const& b){
	for(auto const& x:b){
		a|=x;
	}
	return a;
}

#endif
