#include "rand.h"
#include "../tba/data.h"

tba::Team_key rand(tba::Team_key const*);
tba::Event_key rand(tba::Event_key const*);

auto options(tba::Award_type const*){
	return std::array{
		#define X(A,B) tba::Award_type::A,
		TBA_AWARD_TYPES(X)
		#undef X
	};
}

tba::Award_type rand(tba::Award_type const* x){
	return choose(options(x));
}

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

size_t rand(size_t const*){
	return rand();
}

double rand(double const*){
	return rand();
}

std::string rand(std::string const*){
	return "rand_string";
}

std::chrono::year_month_day rand(std::chrono::year_month_day const*){
	std::chrono::year_month_day r{};
	//obviously could make this more random.
	return r;
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
