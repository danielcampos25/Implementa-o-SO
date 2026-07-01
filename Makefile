CXX = g++
CXXFLAGS = -std=c++17 #-Wall -Wextra -pedantic

# -----------------------------------
# -  Geral
# -----------------------------------

BIN = bin
EXE = $(BIN)/g2_OS
RUN_PROCESS_INPUT = tests/input/processes_valid.txt
RUN_FILES_INPUT = tests/objects/files_example1.txt
RUN_STRING_INPUT = tests/input/string_valid_for_processes.txt
SMOKE_PROCESS_INPUT = tests/input/processes_delivery.txt
SMOKE_FILES_INPUT = tests/objects/files_delivery.txt
SMOKE_STRING_INPUT = tests/input/string_delivery.txt

RESOURCE_SRC = ResourceManager/ResourceManager.cpp
MEMORY_SRC = MemoryManager/MemoryManager.cpp
FILESYSTEM_SRC = FileSystem/FileSystem.cpp FileSystem/FileSystemManager.cpp
PROCESS_SRC = Process/Process.cpp ProcessScheduler/ProcessScheduler.cpp $(RESOURCE_SRC)
DISPATCHER_SRC = Dispatcher/Dispatcher.cpp $(PROCESS_SRC) $(MEMORY_SRC)
PROCESS_INPUT_SRC = ProcessInput/ProcessInputLoader.cpp $(DISPATCHER_SRC)
REFERENCE_STRING_INPUT_SRC = ReferenceStringInput/ReferenceStringInputLoader.cpp
MAIN_SRC = main.cpp $(PROCESS_INPUT_SRC) $(REFERENCE_STRING_INPUT_SRC) $(FILESYSTEM_SRC)

TEST_RESOURCE_1 = test_resource_scheduler.cpp
TEST_RESOURCE_2 = test_resource_scheduler2.cpp
TEST_RESOURCE_3 = test_resource_scheduler3.cpp
TEST_PROCESS = test_process_scheduler.cpp
TEST_DISPATCHER = test_dispatcher.cpp
TEST_PROCESS_INPUT = test_process_input_loader.cpp
TEST_REFERENCE_STRING_INPUT = test_reference_string_input_loader.cpp
TEST_MAIN_FILESYSTEM_FLOW = tests/scripts/test_main_filesystem_flow.cpp
TEST_DELIVERY_SMOKE = tests/scripts/test_delivery_smoke.cpp
TEST_FS = tests/scripts/test_filesystem.cpp
TEST_MEM1 = tests/scripts/test_memory1.cpp
TEST_MEM2 = tests/scripts/test_memory2.cpp

OBJ = $(RESOURCE_SRC:.cpp=.o) $(MEMORY_SRC:.cpp=.o) $(FILESYSTEM_SRC:.cpp=.o) $(PROCESS_SRC:.cpp=.o)

.PHONY: all compile run smoke clean all_tests test_scheduler_all test_memory_all

all: compile test_process_scheduler test_dispatcher test_process_input_loader test_reference_string_input_loader test_main_filesystem_flow

compile: $(MAIN_SRC)
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) $(MAIN_SRC) -o $(EXE)

run: compile
	./$(EXE) $(RUN_PROCESS_INPUT) $(RUN_FILES_INPUT) $(RUN_STRING_INPUT)

smoke: compile $(TEST_DELIVERY_SMOKE)
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) $(TEST_DELIVERY_SMOKE) -o $(BIN)/$(notdir $(basename $(TEST_DELIVERY_SMOKE)))
	./$(BIN)/$(notdir $(basename $(TEST_DELIVERY_SMOKE)))

dispatcher: $(MAIN_SRC)
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) $(MAIN_SRC) -o $(BIN)/$@

# -----------------------------------
# -  Testes
# -----------------------------------

all_tests: test_scheduler_all test_memory_all test_filesystem test_process_scheduler test_dispatcher test_process_input_loader test_reference_string_input_loader

test_scheduler_all: test_resource_scheduler test_resource_scheduler2 test_resource_scheduler3

test_resource_scheduler: $(PROCESS_SRC) $(TEST_RESOURCE_1)
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) $(PROCESS_SRC) $(TEST_RESOURCE_1) -o $(BIN)/$@
	./$(BIN)/$@

test_resource_scheduler2: $(PROCESS_SRC) $(TEST_RESOURCE_2)
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) $(PROCESS_SRC) $(TEST_RESOURCE_2) -o $(BIN)/$@
	./$(BIN)/$@

test_resource_scheduler3: $(PROCESS_SRC) $(TEST_RESOURCE_3)
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) $(PROCESS_SRC) $(TEST_RESOURCE_3) -o $(BIN)/$@
	./$(BIN)/$@

test_process_scheduler: $(PROCESS_SRC) $(TEST_PROCESS)
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) $(PROCESS_SRC) $(TEST_PROCESS) -o $(BIN)/$@
	./$(BIN)/$@

test_dispatcher: $(DISPATCHER_SRC) $(TEST_DISPATCHER)
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) $(DISPATCHER_SRC) $(TEST_DISPATCHER) -o $(BIN)/$@
	./$(BIN)/$@

test_process_input_loader: $(PROCESS_INPUT_SRC) $(TEST_PROCESS_INPUT)
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) $(PROCESS_INPUT_SRC) $(TEST_PROCESS_INPUT) -o $(BIN)/$@
	./$(BIN)/$@

test_reference_string_input_loader: $(REFERENCE_STRING_INPUT_SRC) $(TEST_REFERENCE_STRING_INPUT)
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) $(REFERENCE_STRING_INPUT_SRC) $(TEST_REFERENCE_STRING_INPUT) -o $(BIN)/$@
	./$(BIN)/$@

test_main_filesystem_flow: compile $(TEST_MAIN_FILESYSTEM_FLOW)
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) $(TEST_MAIN_FILESYSTEM_FLOW) -o $(BIN)/$@
	./$(BIN)/$@

test_memory_all: test_memory1 test_memory2 test_memory3

test_memory1: $(MEMORY_SRC) $(TEST_MEM1)
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) $(MEMORY_SRC) $(TEST_MEM1) -o $(BIN)/$@
	./$(BIN)/$@

test_memory2: $(MEMORY_SRC) $(TEST_MEM2)
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) $(MEMORY_SRC) $(TEST_MEM2) -o $(BIN)/$@
	./$(BIN)/$@

test_filesystem: $(FILESYSTEM_SRC) $(TEST_FS)
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) $(FILESYSTEM_SRC) $(TEST_FS) -o $(BIN)/$@
	./$(BIN)/$@

clean:
	rm -f $(BIN)/g2_OS $(BIN)/dispatcher $(BIN)/test* $(OBJ)
