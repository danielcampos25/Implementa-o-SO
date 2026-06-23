CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic

# Arquivos de implementação comuns ao projeto
SRC = ResourceManager/ResourceManager.cpp Scheduler/scheduler.cpp

# Arquivos de teste
TEST1 = test_resource_scheduler.cpp
TEST2 = test_resource_scheduler2.cpp
TEST3 = test_resource_scheduler3.cpp

OBJ = $(SRC:.cpp=.o)

# Alvo padrão: compila todos os testes
all: test_resource_scheduler test_resource_scheduler2 test_resource_scheduler3

# Executa todos os testes sequencialmente
run: all
	./test_resource_scheduler && ./test_resource_scheduler2 && ./test_resource_scheduler3

# Compila o primeiro teste
test_resource_scheduler: $(SRC) $(TEST1)
	$(CXX) $(CXXFLAGS) $(SRC) $(TEST1) -o $@

# Compila o segundo teste
test_resource_scheduler2: $(SRC) $(TEST2)
	$(CXX) $(CXXFLAGS) $(SRC) $(TEST2) -o $@

# Compila o terceiro teste
test_resource_scheduler3: $(SRC) $(TEST3)
	$(CXX) $(CXXFLAGS) $(SRC) $(TEST3) -o $@

# Limpa os executáveis de teste
clean:
	rm -f test_resource_scheduler test_resource_scheduler2 test_resource_scheduler3 $(OBJ)
