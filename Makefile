CXX = g++
CXXFLAGS = -std=c++17 #-Wall -Wextra -pedantic

# -----------------------------------
# -  Geral
# -----------------------------------

# Arquivos de implementação comuns ao projeto
SRC = $(RESOURCE_SRC) $(MEMORY_SRC) $(FILESYSTEM_SRC)
OBJ = $(SRC:.cpp=.o)
BIN = bin
EXE = $(BIN)/g2_OS

# Compilar p projeto
compile: $(SRC)
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(EXE)


# Rodar o projeto (ainda não faz nada, não tem main)
run: $(EXE)
	./$(EXE)


# Compilar e rodar
all: compile run



# -----------------------------------
# -  Testes
# -----------------------------------


# Limpa os executáveis de teste
clean:
	rm -f $(BIN)/test* $(OBJ)


# Compila e executa todos os testes
all_tests: $(SRC) test_scheduler_all test_memory_all test_filesystem


# -- Testes do ResourceManager

RESOURCE_SRC = ResourceManager/ResourceManager.cpp Scheduler/scheduler.cpp

TEST_RSRC1 = tests/scripts/test_resource_scheduler.cpp
TEST_RSRC2 = tests/scripts/test_resource_scheduler2.cpp
TEST_RSRC3 = tests/scripts/test_resource_scheduler3.cpp

test_scheduler_all: test_resource_scheduler test_resource_scheduler2 test_resource_scheduler3

test_resource_scheduler: $(RESOURCE_SRC) $(TEST_RSRC1)
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) $(RESOURCE_SRC) $(TEST_RSRC1) -o $(BIN)/$@
	./$(BIN)/$@

test_resource_scheduler2: $(RESOURCE_SRC) $(TEST_RSRC2)
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) $(RESOURCE_SRC) $(TEST_RSRC2) -o $(BIN)/$@
	./$(BIN)/$@

test_resource_scheduler3: $(RESOURCE_SRC) $(TEST_RSRC3)
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) $(RESOURCE_SRC) $(TEST_RSRC3) -o $(BIN)/$@
	./$(BIN)/$@


# -- Testes do MemoryManager

MEMORY_SRC = MemoryManager/MemoryManager.cpp

TEST_MEM1 = tests/scripts/test_memory1.cpp
TEST_MEM2 = tests/scripts/test_memory2.cpp

test_memory_all: test_memory1 test_memory2

test_memory1:
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) $(MEMORY_SRC) $(TEST_MEM1) -o $(BIN)/$@
	./$(BIN)/$@

test_memory2:
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) $(MEMORY_SRC) $(TEST_MEM2) -o $(BIN)/$@
	./$(BIN)/$@


# -- Testes do FileSystem

FILESYSTEM_SRC = FileSystem/FileSystem.cpp FileSystem/FileSystemManager.cpp

TEST_FS = tests/scripts/test_filesystem.cpp

test_filesystem: $(FILESYSTEM_SRC) $(TEST_FS)
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) $(FILESYSTEM_SRC) $(TEST_FS) -o $(BIN)/$@
	./$(BIN)/$@
