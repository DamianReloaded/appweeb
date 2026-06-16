PROJECT := appweeb

CXX := g++

CXXFLAGS := -std=c++23 -Wall -Wextra -O2

LDFLAGS := -lws2_32

SOURCES := \
	main.cpp \
	socket.cpp \
	webserver.cpp

OBJECTS := $(SOURCES:.cpp=.o)

all: $(PROJECT).exe

$(PROJECT).exe: $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS)
	rm -f $(PROJECT).exe

rebuild: clean all

.PHONY: all clean rebuild