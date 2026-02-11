#include "rand.h"

tba::Team_key rand(tba::Team_key const*);
tba::Event_key rand(tba::Event_key const*);

bool rand(bool const*){
	return rand()%2;
}

short rand(short const*){
	return rand();
}

unsigned short rand(unsigned short const*){
	return rand();
}

int rand(int const*){
	return rand();
}

unsigned rand(unsigned const*){
	return (unsigned)rand();
}

double rand(double*){
	return rand();
}

std::string rand(std::string const*){
	return "rand_string";
}

/*template<typename A,typename B>
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

template<typename K,typename V>
std::map<K,V> rand(std::map<K,V> const*){
	std::map<K,V> r;
	for(auto k:rand((std::vector<K>*)0)){
		r[k]=rand((V*)0);
	}
	return r;
}

template<typename A,typename B>
std::variant<A,B> rand(std::variant<A,B> const*){
	if(rand()%2){
		return rand((A*)0);
	}
	return rand((B*)0);
}
*/
