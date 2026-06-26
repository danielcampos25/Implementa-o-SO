CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic

# Arquivos de implementação comuns ao projeto
SRC = ResourceManager/ResourceManager.cpp Scheduler/scheduler.cpp MemoryManager/MemoryManager.cpp

# Arquivos de teste
TEST1 = tests/scripts/test_resource_scheduler.cpp
TEST2 = tests/scripts/test_resource_scheduler2.cpp
TEST3 = tests/scripts/test_resource_scheduler3.cpp
TEST4 = tests/scripts/test_memory1.cpp
TEST5 = tests/scripts/test_memory2.cpp

OBJ = $(SRC:.cpp=.o)

# Alvo padrão: compila todos os testes
all: test_resource_scheduler test_resource_scheduler2 test_resource_scheduler3 test_memory1 test_memory2

# Executa todos os testes sequencialmente
run: all
	./bin/test_resource_scheduler && ./bin/test_resource_scheduler2 && ./bin/test_resource_scheduler3 && ./bin/test_memory1 && ./bin/test_memory2

# Compila o primeiro teste
test_resource_scheduler: $(SRC) $(TEST1)
	$(CXX) $(CXXFLAGS) $(SRC) $(TEST1) -o bin/$@

# Compila o segundo teste
test_resource_scheduler2: $(SRC) $(TEST2)
	$(CXX) $(CXXFLAGS) $(SRC) $(TEST2) -o bin/$@

# Compila o terceiro teste
test_resource_scheduler3: $(SRC) $(TEST3)
	$(CXX) $(CXXFLAGS) $(SRC) $(TEST3) -o bin/$@

# Compila o teste de memória
test_memory1:
	$(CXX) $(CXXFLAGS) MemoryManager/MemoryManager.cpp $(TEST4) -o bin/$@

test_memory2:
	$(CXX) $(CXXFLAGS) MemoryManager/MemoryManager.cpp $(TEST5) -o bin/$@

# Limpa os executáveis de teste
clean:
	rm -f bin/test* $(OBJ)
