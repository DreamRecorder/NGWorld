INTERNAL_SOURCES = $(wildcard internal/*.cpp)
INTERNAL_OBJECTS = $(patsubst internal/%.cpp, obj/internal/%.o, $(INTERNAL_SOURCES))

SERVER_SOURCES = $(wildcard server/*.cpp)
SERVER_OBJECTS = $(patsubst server/%.cpp, obj/server/%.o, $(SERVER_SOURCES))

CLIENT_SOURCES = $(wildcard client/*.cpp)
CLIENT_OBJECTS = $(patsubst client/%.cpp, obj/client/%.o, $(CLIENT_SOURCES))

TESTBENCH_SOURCES = $(wildcard testbench/*.cpp)
TESTBENCH_OBJECTS = $(patsubst testbench/%.cpp, obj/testbench/%.o, $(TESTBENCH_SOURCES))

# put internal headers into search path
CXXFLAGS += -I internal/

ifeq ($(DEBUG), 1)
	CXXFLAGS += -g -O0
else
	CXXFLAGS += -O2
endif

ifeq ($(NOWARNING), 1)
	CXXFLAGS += -w
else
	CXXFLAGS += -Werror
endif

ifeq ($(shell sh -c 'uname -s'), Darwin)
	CXX = clang++
endif

all:
	@echo You must specify which target to build:
	@echo \* TestBench
	@echo \* Server
	@echo \* Client

TestBench: bin/testbench

bin/testbench: bin/ obj/testbench Internal $(TESTBENCH_OBJECTS)
	$(CXX) -o bin/testbench $(LDFLAGS) $(INTERNAL_OBJECTS) $(TESTBENCH_OBJECTS)

Internal: obj/internal $(INTERNAL_OBJECTS)

Server: bin/server

bin/server: bin/ obj/server Internal $(SERVER_OBJECTS)
	$(CXX) -o bin/server $(LDFLAGS) $(INTERNAL_OBJECTS) $(SERVER_OBJECTS)

Client: bin/client

bin/client: bin obj/client Internal $(CLIENT_OBJECTS)
	$(CXX) -o bin/client $(LDFLAGS) $(INTERNAL_OBJECTS) $(CLIENT_OBJECTS)

obj/:
	mkdir -p obj

bin/:
	mkdir -p bin

obj/internal:
	mkdir -p obj/internal

obj/server:
	mkdir -p obj/server

obj/client:
	mkdir -p obj/client

obj/testbench:
	mkdir -p obj/testbench

obj/internal/%.o: internal/%.cpp
	$(CXX) -I include -std=c++11 $(CXXFLAGS) -c $< -o $@ 

obj/server/%.o: server/%.cpp
	$(CXX) -I include -std=c++11 $(CXXFLAGS) -c $< -o $@ 

obj/client/%.o: client/%.cpp
	$(CXX) -I include -std=c++11 $(CXXFLAGS) -c $< -o $@ 

obj/testbench/%.o: testbench/%.cpp
	$(CXX) -I include -std=c++11 $(CXXFLAGS) -c $< -o $@ 

clean:
	rm -rf obj/* bin/*
