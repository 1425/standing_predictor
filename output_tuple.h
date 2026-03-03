#ifndef OUTPUT_TUPLE_H
#define OUTPUT_TUPLE_H

using A_Point_3=std::array<Point,3>;

#define OUTPUT_TUPLE(X)\
	X(tba::Team_key,team)\
	X(Dcmp_home,dcmp_home)\
	X(Pr,dcmp_make)\
	X(A_Point_3,dcmp_interesting)\
	X(Pr,cmp_make)\
	X(A_Point_3,cmp_interesting)

STRUCT_DECLARE(Output_tuple,OUTPUT_TUPLE)
Output_tuple rand(Output_tuple const*);

#endif
