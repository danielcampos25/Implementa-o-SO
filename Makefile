CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic

# Arquivos de implementação comuns ao projeto
SRC = ResourceManager/ResourceManager.cpp Scheduler/scheduler.cpp MemoryManager/MemoryManager.cpp

# Arquivos de teste
TEST1 = tests/scripts/test_resource_scheduler.cpp
TEST2 = tests/scripts/test_resource_scheduler2.cpp
TEST3 = tests/scripts/test_resource_scheduler3.cpp
TEST4 = tests/scripts/test_memory.cpp

OBJ = $(SRC:.cpp=.o)

# Alvo padrão: compila todos os testes
all: test_resource_scheduler test_resource_scheduler2 test_resource_scheduler3 test_memory

# Executa todos os testes sequencialmente
run: all
	./test_resource_scheduler && ./test_resource_scheduler2 && ./test_resource_scheduler3 && ./test_memory

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
test_memory:
	$(CXX) $(CXXFLAGS) MemoryManager/MemoryManager.cpp $(TEST4) -o bin/$@

# Limpa os executáveis de teste
clean:
	rm -f test_resource_scheduler test_resource_scheduler2 test_resource_scheduler3 $(OBJ)
