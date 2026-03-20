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

template<typename T>
T rand(T const*);

std::chrono::year rand(std::chrono::year const*){
	return std::chrono::year(rand()%3000);
}

std::chrono::month rand(std::chrono::month const*){
	return std::chrono::month(1+rand()%12);
}

std::chrono::day rand(std::chrono::day const*){
	return std::chrono::day(1+rand()%28);
}

std::chrono::year_month_day rand(std::chrono::year_month_day const*){
	std::chrono::year_month_day r{
		rand((std::chrono::year*)0),
		rand((std::chrono::month*)0),
		rand((std::chrono::day*)0)
	};
	return r;
}
