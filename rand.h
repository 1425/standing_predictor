#ifndef RAND_H
#define RAND_H

int rand(int*){
	return rand();
}

double rand(double*){
	return rand();
}

std::string rand(std::string const*){
	return "rand_string";
}

template<typename A,typename B>
auto rand(std::pair<A,B> const*){
	return std::make_pair(rand((A*)0),rand((B*)0));
}

template<typename T>
std::vector<T> rand(std::vector<T> const*){
	return mapf(
		[](auto x){
			(void)x;
			return rand((T*)0);
		},
		range(rand()%5)
	);
}

template<typename T>
std::optional<T> rand(std::optional<T> const*){
	if(rand()%2){
		return rand((T*)0);
	}
	return std::nullopt;
}

#endif
