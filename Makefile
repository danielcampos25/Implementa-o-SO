CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic

# Arquivos de implementação comuns ao projeto
RESOURCE_SRC = ResourceManager/ResourceManager.cpp Scheduler/scheduler.cpp
FILESYSTEM_SRC = FileSystem/FileSystem.cpp FileSystem/FileSystemManager.cpp
PROCESS_SRC = Process/Process.cpp ProcessScheduler/ProcessScheduler.cpp
DISPATCHER_SRC = Dispatcher/Dispatcher.cpp $(PROCESS_SRC)

# Arquivos de teste
TEST1 = test_resource_scheduler.cpp
TEST2 = test_resource_scheduler2.cpp
TEST3 = test_resource_scheduler3.cpp
TEST_FS = test_filesystem.cpp
TEST_PROCESS = test_process_scheduler.cpp
TEST_DISPATCHER = test_dispatcher.cpp

OBJ = $(RESOURCE_SRC:.cpp=.o) $(FILESYSTEM_SRC:.cpp=.o) $(PROCESS_SRC:.cpp=.o)

# Alvo padrão: compila todos os testes
all: test_resource_scheduler test_resource_scheduler2 test_resource_scheduler3 test_filesystem test_process_scheduler test_dispatcher

# Executa todos os testes sequencialmente
run: all
	./test_resource_scheduler && ./test_resource_scheduler2 && ./test_resource_scheduler3 && ./test_filesystem && ./test_process_scheduler && ./test_dispatcher

# Compila o primeiro teste
test_resource_scheduler: $(RESOURCE_SRC) $(TEST1)
	$(CXX) $(CXXFLAGS) $(RESOURCE_SRC) $(TEST1) -o $@

# Compila o segundo teste
test_resource_scheduler2: $(RESOURCE_SRC) $(TEST2)
	$(CXX) $(CXXFLAGS) $(RESOURCE_SRC) $(TEST2) -o $@

# Compila o terceiro teste
test_resource_scheduler3: $(RESOURCE_SRC) $(TEST3)
	$(CXX) $(CXXFLAGS) $(RESOURCE_SRC) $(TEST3) -o $@

# Compila o teste do FileSystem
test_filesystem: $(FILESYSTEM_SRC) $(TEST_FS)
	$(CXX) $(CXXFLAGS) $(FILESYSTEM_SRC) $(TEST_FS) -o $@

# Compila o teste do ProcessScheduler
test_process_scheduler: $(PROCESS_SRC) $(TEST_PROCESS)
	$(CXX) $(CXXFLAGS) $(PROCESS_SRC) $(TEST_PROCESS) -o $@

test_dispatcher: $(DISPATCHER_SRC) $(TEST_DISPATCHER)
	$(CXX) $(CXXFLAGS) $(DISPATCHER_SRC) $(TEST_DISPATCHER) -o $@

# Limpa os executáveis de teste
clean:
	rm -f test_resource_scheduler test_resource_scheduler2 test_resource_scheduler3 test_filesystem test_process_scheduler test_dispatcher $(OBJ)
