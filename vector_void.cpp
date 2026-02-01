#include "vector_void.h"

vector_void::vector_void(size_t a):size_(a){}

size_t vector_void::size()const{
	return size_;
}

std::set<int> to_set(vector_void const&){
	return std::set<int>();
}

std::ostream& operator<<(std::ostream& o,vector_void){
	return o<<"vector_void";
}

