#CXXFLAGS=-std=c++23 -Wall -Wextra -Ofast -fdiagnostics-color=always -flto=auto -mtune=native -march=native
#CXXFLAGS=-std=c++23 -Wall -Wextra -pedantic -Og -g -fdiagnostics-color=always -fsanitize=undefined,address -fno-omit-frame-pointer
#CXXFLAGS=-std=c++26 -Wall -Wextra -pedantic -Og -g -fdiagnostics-color=always -freflection
CXXFLAGS=-std=c++23 -Wall -Wextra -pedantic -Og -g -fdiagnostics-color=always
#CXXFLAGS=-std=c++23 -Os
LIBS=-lsqlite3 -lcurl -lsimdjson -ltbb
CC=$(CXX)

EXE=outline meta declines diff

all: $(EXE)

TBA_OBJS= \
	../tba/db.o \
	../tba/data.o \
	../tba/curl.o \
	../tba/simdjson.o \
	../tba/util.o\
	../tba/event_key.o\
	../tba/year.o\
	../tba/district_key.o\
	../tba/team_key.o\
	../tba/match_key.o\
	../tba/match.o\

FRC_API_OBJS= \
	../frc_api/data.o \
	../frc_api/query.o \
	../frc_api/db.o \
	../frc_api/simdjson.o \
	../frc_api/curl.o

OUTLINE_OBJS= \
	query.o \
	fly.o \
	history.o \
	index.o \
	tournament_status.o\
	event_categories.o\
	avatar.o \
	annotated_complex.o\
	cat.o \
	data_range.o\
	toggle.o\
	event_partial.o\
	ranking_match_status.o\
	vector.o \
	winners.o\
	probability.o\
	lock2.o\
	event_limits.o\
	district_championship_assignment.o\
	int_limited.o \
	venue.o\
	address.o \
	playoff_limits.o \
	award_limits.o \
	outline.o \
	output.o \
	util.o \
	slots.o \
	arguments.o \
	tba.o \
	status.o \
	event_status.o \
	cmp_reason.o \
	run.o \
	ca.o \
	zipcodes.o \
	skill.o \
	skill_opr.o \
	dates.o \
	print_r.o \
	vector_void.o \
	io.o \
	timezone.o \
	names.o \
	interval.o \
	plot.o \
	subprocess.o \
	lock.o \
	rand.o \
	rank_pts.o \
	pick_points.o\
	rank_limits.o\
	rp.o \
	$(TBA_OBJS)\
	$(FRC_API_OBJS)

outline: $(OUTLINE_OBJS)
	$(CXX) $(CXXFLAGS) $^ $(LIBS) -o $@

META_OBJS= \
	$(TBA_OBJS) \
	probability.o\
	cat.o \
	meta.o \
	util.o \
	tba.o \
	arguments.o \
	io.o \
	dates.o\
	names.o\
	print_r.o\
	$(FRC_API_OBJS) \
	zipcodes.o\
	slots.o\
	timezone.o\
	address.o\
	interval.o\
	vector.o\
	event_status.o \
	status.o \
	cmp_reason.o \
	query.o \
	rand.o \
	history.o \
	run.o \
	event_partial.o \
	skill_opr.o \
	skill.o \
	tournament_status.o\
	rank_limits.o\
	rp.o\
	rank_pts.o\
	event_categories.o\
	ranking_match_status.o\
	pick_points.o\
	output.o\
	plot.o\
	ca.o\
	subprocess.o\
	avatar.o\
	event_limits.o\
	award_limits.o\
	playoff_limits.o\
	lock2.o\
	district_championship_assignment.o\
	annotated_complex.o\

meta: $(META_OBJS)
	$(CXX) $(CXXFLAGS) $^ $(LIBS) -lgumbo -o $@

DECLINE_OBJS= \
	cat.o \
	declines.o \
	frc_api.o \
	rank_pts.o \
	arguments.o \
	tba.o \
	util.o \
	io.o \
	$(TBA_OBJS) \
	$(FRC_API_OBJS)\
	dates.o\
	names.o\
	$(FRC_API_OBJS) \
	slots.o\
	timezone.o\
	address.o\
	print_r.o\
	interval.o\
	vector.o\
	query.o\
	rand.o\
	history.o \
	run.o \
	event_partial.o \
	skill_opr.o \
	skill.o \
	tournament_status.o\
	rank_limits.o\
	rp.o\
	rank_pts.o\
	event_categories.o\
	ranking_match_status.o\
	pick_points.o\
	output.o\
	plot.o\
	ca.o\
	subprocess.o\
	avatar.o\
	event_limits.o\
	award_limits.o\
	playoff_limits.o\
	lock2.o\
	district_championship_assignment.o\
	annotated_complex.o\
	probability.o\
	cmp_reason.o\
	zipcodes.o\
	status.o\
	event_status.o\

declines: $(DECLINE_OBJS)
	$(CXX) $(CXXFLAGS) $^ $(LIBS) -o $@

DIFF_OBJS=\
	$(TBA_OBJS)\
	io.o\
	diff.o\
	vector.o\
	util.o\

diff: $(DIFF_OBJS)
	$(CXX) $(CXXFLAGS) $^ $(LIBS) -o $@

clean:
	rm -f $(EXE) $(OUTLINE_OBJS) $(META_OBJS) $(DECLINE_OBJS) $(DIFF_OBJS)
