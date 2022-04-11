CXXFLAGS=-std=c++20 -g
LIBS=-lsqlite3 -lcurl
all: outline meta

TBA_OBJS=../tba/db.o ../tba/data.o ../tba/curl.o ../tba/rapidjson.o ../tba/util.o

OBJS= \
	outline.o util.o $(TBA_OBJS)
	#../frc_api/db.o ../frc_api/curl.o

outline: $(OBJS)
	$(CXX) $(CXXFLAGS) $^ $(LIBS) -o $@

meta: $(TBA_OBJS) meta.o util.o
	$(CXX) $(CXXFLAGS) $^ $(LIBS) -lgumbo -o $@

clean:
	rm -f outline meta $(OBJS) meta.o

