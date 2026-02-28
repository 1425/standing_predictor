#ifndef CAT_H
#define CAT_H

#include<string>
#include<variant>
#include "util.h"

STRUCT_DECLARE(Now,EMPTY)
STRUCT_DECLARE(Static,EMPTY)
using Cat_result=std::variant<Now,Static>;

std::optional<Cat_result> cat(std::string const&);

#endif
