CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic

SRC = ResourceManager/ResourceManager.cpp Scheduler/scheduler.cpp
TEST1 = test_resource_scheduler.cpp
TEST2 = test_resource_scheduler2.cpp
TEST3 = test_resource_scheduler3.cpp
OBJ = $(SRC:.cpp=.o)

all: test_resource_scheduler test_resource_scheduler2 test_resource_scheduler3

run: all
	./test_resource_scheduler && ./test_resource_scheduler2 && ./test_resource_scheduler3

test_resource_scheduler: $(SRC) $(TEST1)
	$(CXX) $(CXXFLAGS) $(SRC) $(TEST1) -o $@

test_resource_scheduler2: $(SRC) $(TEST2)
	$(CXX) $(CXXFLAGS) $(SRC) $(TEST2) -o $@

test_resource_scheduler3: $(SRC) $(TEST3)
	$(CXX) $(CXXFLAGS) $(SRC) $(TEST3) -o $@

clean:
	rm -f test_resource_scheduler test_resource_scheduler2 test_resource_scheduler3 $(OBJ)
