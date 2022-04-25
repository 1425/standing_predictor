CXXFLAGS=-std=c++20 -g -Wall -Wextra
LIBS=-lsqlite3 -lcurl
all: outline meta declines

TBA_OBJS=../tba/db.o ../tba/data.o ../tba/curl.o ../tba/rapidjson.o ../tba/util.o

OBJS= \
	outline.o util.o $(TBA_OBJS)
	#../frc_api/db.o ../frc_api/curl.o

outline: $(OBJS)
	$(CXX) $(CXXFLAGS) $^ $(LIBS) -o $@

meta: $(TBA_OBJS) meta.o util.o
	$(CXX) $(CXXFLAGS) $^ $(LIBS) -lgumbo -o $@

FRC_API_OBJS= \
	../frc_api/data.o \
	../frc_api/query.o \
	../frc_api/db.o \
	../frc_api/rapidjson.o \
	../frc_api/curl.o

declines: declines.o frc_api.o util.o $(TBA_OBJS) $(FRC_API_OBJS)
	$(CXX) $(CXXFLAGS) $^ $(LIBS) -o $@

clean:
	rm -f outline meta $(OBJS) $(FRC_API_OBJS) meta.o declines declines.o
