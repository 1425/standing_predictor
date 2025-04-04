CXXFLAGS=-std=c++20 -Wall -Wextra -Ofast
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
	$(TBA_OBJS)

outline: $(OUTLINE_OBJS)
	$(CXX) $(CXXFLAGS) $^ $(LIBS) -o $@

META_OBJS= \
	$(TBA_OBJS) \
	meta.o \
	util.o \
	tba.o \
	arguments.o \

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
	$(TBA_OBJS) \
	$(FRC_API_OBJS)

declines: $(DECLINE_OBJS)
	$(CXX) $(CXXFLAGS) $^ $(LIBS) -o $@

clean:
	rm -f $(EXE) $(OUTLINE_OBJS) $(META_OBJS) $(DECLINE_OBJS)
