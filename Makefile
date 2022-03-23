CXXFLAGS=-std=c++20 -Ofast
LIBS=-lsqlite3 -lcurl
all: outline

outline: outline.o util.o ../tba/db.o ../tba/data.o ../tba/curl.o ../tba/rapidjson.o ../tba/util.o
	$(CXX) $(CXXFLAGS) $^ $(LIBS) -o $@

clean:
	rm -f outline *.o
