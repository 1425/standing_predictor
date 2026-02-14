#CXXFLAGS=-std=c++23 -Wall -Wextra -Ofast -fdiagnostics-color=always
CXXFLAGS=-std=c++23 -Wall -Wextra -Og -g -fdiagnostics-color=always
LIBS=-lsqlite3 -lcurl -lsimdjson
CC=$(CXX)

EXE=outline meta declines

all: $(EXE)

TBA_OBJS= \
	../tba/db.o \
	../tba/data.o \
	../tba/curl.o \
	../tba/simdjson.o \
	../tba/util.o

FRC_API_OBJS= \
	../frc_api/data.o \
	../frc_api/query.o \
	../frc_api/db.o \
	../frc_api/simdjson.o \
	../frc_api/curl.o

OUTLINE_OBJS= \
	award_limits.o \
	outline.o \
	output.o \
	util.o \
	event.o \
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
	meta.o \
	util.o \
	tba.o \
	arguments.o \
	io.o \

meta: $(META_OBJS)
	$(CXX) $(CXXFLAGS) $^ $(LIBS) -lgumbo -o $@

DECLINE_OBJS= \
	declines.o \
	frc_api.o \
	rank_pts.o \
	arguments.o \
	print_r.o \
	tba.o \
	util.o \
	io.o \
	$(TBA_OBJS) \
	$(FRC_API_OBJS)

declines: $(DECLINE_OBJS)
	$(CXX) $(CXXFLAGS) $^ $(LIBS) -o $@

clean:
	rm -f $(EXE) $(OUTLINE_OBJS) $(META_OBJS) $(DECLINE_OBJS)
