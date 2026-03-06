#ifndef TOURNAMENT_STATUS_H
#define TOURNAMENT_STATUS_H

#include "util.h"
#include "rank_limits.h"

STRUCT_DECLARE(Tournament_status_picking_in_progress,EMPTY)
STRUCT_DECLARE(Tournament_status_picking_complete,EMPTY)
STRUCT_DECLARE(Tournament_status_eliminations_in_progress,EMPTY)
STRUCT_DECLARE(Tournament_status_eliminations_complete,EMPTY)
STRUCT_DECLARE(Tournament_status_awards_in_progress,EMPTY)
STRUCT_DECLARE(Tournament_status_complete,EMPTY)

using Tournament_status=std::variant<
	Qual_status_future,
	Qual_status_in_progress,
	Qual_status_complete,
	Tournament_status_picking_in_progress,
	Tournament_status_picking_complete,
	Tournament_status_eliminations_in_progress,
	Tournament_status_eliminations_complete,
	Tournament_status_awards_in_progress,
	Tournament_status_complete
>;

bool in_progress(Tournament_status);

#endif
