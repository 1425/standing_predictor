#include "vector_void.h"

vector_void::vector_void(size_t a):size_(a){}

size_t vector_void::size()const{
	return size_;
}

std::set<int> to_set(vector_void const&){
	return std::set<int>();
}

std::ostream& operator<<(std::ostream& o,vector_void a){
	return o<<"vector_void("<<a.size()<<")";
}

vector_void sorted(vector_void a){
	return a;
}

vector_void take(size_t n,vector_void a){
	return vector_void( (n>a.size())?0:(a.size()-n) );
}
