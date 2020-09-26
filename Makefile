
CXX=g++
CXXFLAGS=-Iinclude -std=c++17 -g -Og
LD=g++
LDFLAGS=-lGL -lX11 -lpng -lpthread -g -Og -std=c++17

sources=$(wildcard src/*.cpp)
objects=$(patsubst src/%.cpp,obj/%.o,$(sources))

paint: obj $(objects)
	$(LD) -o $@ $(objects) $(LDFLAGS)

obj:
	mkdir -p obj

obj/%.o: src/%.cpp include/*.h
	$(CXX) -c -o $@ $< $(CXXFLAGS)

clean:
	rm -rf obj

.PHONY: clean

