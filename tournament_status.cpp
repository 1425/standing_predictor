#include "tournament_status.h"

PRINT_STRUCT(Tournament_status_picking_in_progress,EMPTY)
PRINT_STRUCT(Tournament_status_picking_complete,EMPTY)
PRINT_STRUCT(Tournament_status_eliminations_in_progress,EMPTY)
PRINT_STRUCT(Tournament_status_eliminations_complete,EMPTY)
PRINT_STRUCT(Tournament_status_awards_in_progress,EMPTY)
PRINT_STRUCT(Tournament_status_complete,EMPTY)

ELEMENTWISE_RAND(Tournament_status_picking_in_progress,EMPTY)
ELEMENTWISE_RAND(Tournament_status_picking_complete,EMPTY)
ELEMENTWISE_RAND(Tournament_status_eliminations_in_progress,EMPTY)
ELEMENTWISE_RAND(Tournament_status_eliminations_complete,EMPTY)
ELEMENTWISE_RAND(Tournament_status_awards_in_progress,EMPTY)
ELEMENTWISE_RAND(Tournament_status_complete,EMPTY)

std::string text(Tournament_status_picking_in_progress){
	return "Alliance selection";
}

std::string text(Tournament_status_picking_complete){
	return "Alliance selection complete";
}

std::string text(Tournament_status_eliminations_in_progress){
	return "Playoffs";
}

std::string text(Tournament_status_eliminations_complete){
	return "Playoffs complete";
}

std::string text(Tournament_status_awards_in_progress){
	return "Awards";
}

std::string text(Tournament_status_complete){
	return "Complete";
}

std::string text(Qual_status_future){
	return "Future";
}

std::string text(Qual_status_complete){
	return "Quals complete";
}

std::string text(auto a){
	return as_string(a);
}

std::string show(Tournament_status const& a){
	return std::visit([](auto x){ return text(x); },a);
}
