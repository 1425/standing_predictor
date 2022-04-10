CXXFLAGS=-std=c++20 -Ofast
LIBS=-lsqlite3 -lcurl
all: outline

OBJS= \
	outline.o util.o \
	../tba/db.o ../tba/data.o ../tba/curl.o ../tba/rapidjson.o ../tba/util.o \
	#../frc_api/db.o ../frc_api/curl.o

outline: $(OBJS)
	$(CXX) $(CXXFLAGS) $^ $(LIBS) -o $@

clean:
	rm -f outline $(OBJS)
