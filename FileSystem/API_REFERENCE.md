# Referência de API - Módulo de Arquivos

## File.h - Estruturas de Dados

### struct File
Representa um arquivo armazenado no disco.

```cpp
struct File {
    std::string name;         // Nome do arquivo (ex: "A", "B", ...)
    int startBlock;          // Índice do primeiro bloco
    int numBlocks;           // Quantidade total de blocos
    int ownerPID;            // PID do processo que criou (-1 se pré-existente)
    
    // Construtores
    File(const std::string &n, int start, int blocks, int owner);
    File();
};
```

### enum FileOperationType
Tipos de operações suportadas.

```cpp
enum FileOperationType {
    CREATE = 0,     // Criar novo arquivo
    DELETE = 1      // Deletar arquivo existente
};
```

### struct FileOperation
Descreve uma operação a ser realizada.

```cpp
struct FileOperation {
    int processPID;           // ID do processo solicitante
    FileOperationType op;     // Tipo de operação
    std::string fileName;     // Nome do arquivo
    int numBlocks;           // Blocos a alocar (só para CREATE)
    
    FileOperation();
};
```

### struct FileOperationResult
Resultado de uma operação.

```cpp
struct FileOperationResult {
    bool success;                      // Operação bem-sucedida?
    std::string message;               // Descrição detalhada
    std::vector<int> allocatedBlocks;  // Blocos usados (para CREATE)
    
    FileOperationResult(bool s = false, const std::string &msg = "");
};
```

---

## FileSystem.h - Sistema de Arquivos

### Construtor

```cpp
FileSystem::FileSystem(int totalBlocks = 20);
```
**Parâmetros:**
- `totalBlocks`: Número total de blocos no disco (padrão: 20)

**Exemplo:**
```cpp
FileSystem fs(30);  // Disco com 30 blocos
```

---

### createFile()

```cpp
FileOperationResult createFile(
    int processPID,
    const std::string &fileName,
    int numBlocks,
    int priority = 1
);
```

**Parâmetros:**
- `processPID`: ID do processo que cria o arquivo
- `fileName`: Nome do arquivo (ex: "A", "dados.txt")
- `numBlocks`: Quantidade de blocos a alocar
- `priority`: Prioridade do processo (0=tempo real, >0=usuário)

**Retorno:**
- `success`: true se criado com sucesso
- `message`: Descrição da operação
- `allocatedBlocks`: Índices dos blocos alocados

**Erros possíveis:**
- "Número de blocos deve ser maior que zero."
- "Arquivo 'X' já existe."
- "Falta de espaço disponível no disco."
- "Falha ao alocar espaço contíguo no disco."

**Exemplo:**
```cpp
auto result = fs.createFile(0, "File1", 5, 1);
if (result.success) {
    std::cout << result.message << "\n";  
    // Arquivo 'File1' criado com sucesso nos blocos [0, 1, 2, 3, 4].
}
```

---

### deleteFile()

```cpp
FileOperationResult deleteFile(
    int processPID,
    const std::string &fileName,
    int priority = 1
);
```

**Parâmetros:**
- `processPID`: ID do processo que deleta
- `fileName`: Nome do arquivo a deletar
- `priority`: Prioridade do processo (0=tempo real, >0=usuário)

**Retorno:**
- Similar a `createFile()`

**Erros possíveis:**
- "Arquivo 'X' não existe."
- "Processo X não tem permissão para deletar arquivo 'X'."

**Regras de Permissão:**
- Tempo real (priority=0): pode deletar qualquer arquivo
- Usuário (priority>0): pode deletar apenas seus arquivos

**Exemplo:**
```cpp
auto result = fs.deleteFile(0, "File1", 1);
if (result.success) {
    std::cout << "Arquivo deletado\n";
}
```

---

### loadDiskState()

```cpp
bool loadDiskState(const std::string &diskStateString);
```

**Parâmetros:**
- `diskStateString`: String descrevendo arquivos iniciais
- Formato: `"arquivo1:bloco1:blocos1|arquivo2:bloco2:blocos2|..."`

**Retorno:**
- true se carregado com sucesso, false caso contrário

**Exemplo:**
```cpp
fs.loadDiskState("X:0:2|Y:3:1|Z:5:3");
// Carrega: X (blocos 0-1), Y (bloco 3), Z (blocos 5-7)
```

---

### getAvailableSpace()

```cpp
int getAvailableSpace() const;
```

**Retorno:**
- Número de blocos livres no disco

**Exemplo:**
```cpp
int free = fs.getAvailableSpace();
std::cout << "Blocos livres: " << free << "\n";
```

---

### getTotalSpace()

```cpp
int getTotalSpace() const;
```

**Retorno:**
- Número total de blocos do disco

**Exemplo:**
```cpp
int total = fs.getTotalSpace();
std::cout << "Total de blocos: " << total << "\n";
```

---

### printDiskMap()

```cpp
void printDiskMap() const;
```

**Saída:**
- Exibe mapa do disco no console (uma linha por 10 blocos)

**Exemplo:**
```cpp
fs.printDiskMap();
// Saída:
// Mapa de ocupação do disco:
// A A A B B 0 0 0 0 0
```

---

### getDiskMapString()

```cpp
std::string getDiskMapString() const;
```

**Retorno:**
- String representando o mapa do disco

**Exemplo:**
```cpp
std::string map = fs.getDiskMapString();
std::cout << map << "\n";  // "A A A B B 0 0 0 0 0"
```

---

### printFileList()

```cpp
void printFileList() const;
```

**Saída:**
- Lista todos os arquivos com informações (nome, blocos, owner)

**Exemplo:**
```cpp
fs.printFileList();
// Saída:
// Arquivos no disco:
//   A: blocos [0 a 2], owner PID: 0
//   B: blocos [3 a 4], owner PID: 1
```

---

## FileSystemManager.h - Gerenciador de Alto Nível

### Construtor

```cpp
FileSystemManager::FileSystemManager(int totalBlocks = 20);
```

**Parâmetros:**
- `totalBlocks`: Blocos do disco (padrão: 20)

---

### loadDiskConfiguration()

```cpp
bool loadDiskConfiguration(const std::string &filename);
```

**Parâmetros:**
- `filename`: Caminho do arquivo de configuração (files.txt)

**Retorno:**
- true se carregado com sucesso

**Formato esperado do arquivo:**
```
10
3
X,0,2
Y,3,1
Z,5,3
0,0,A,5
0,1,X
```

**Erros:**
- Se arquivo não existir
- Se formato for inválido
- Se não houver espaço suficiente

**Exemplo:**
```cpp
FileSystemManager fsm(20);
if (fsm.loadDiskConfiguration("files.txt")) {
    std::cout << "Configuração carregada\n";
}
```

---

### registerProcess()

```cpp
bool registerProcess(int pid, int priority);
```

**Parâmetros:**
- `pid`: ID do processo
- `priority`: Prioridade (0=tempo real, >0=usuário)

**Retorno:**
- true se registrado com sucesso

**Exemplo:**
```cpp
fsm.registerProcess(0, 0);  // Tempo real
fsm.registerProcess(1, 1);  // Usuário
```

---

### executeOperation()

```cpp
FileOperationResult executeOperation(const FileOperation &op);
```

**Parâmetros:**
- `op`: Operação a executar

**Retorno:**
- Resultado da operação

**Exemplo:**
```cpp
FileOperation op;
op.processPID = 0;
op.op = CREATE;
op.fileName = "newfile";
op.numBlocks = 5;

auto result = fsm.executeOperation(op);
```

---

### executeAllOperations()

```cpp
bool executeAllOperations();
```

**Retorno:**
- true se todas as operações foram processadas

**Efeito:**
- Executa todas as operações carregadas do arquivo

**Exemplo:**
```cpp
fsm.executeAllOperations();
```

---

### printResults()

```cpp
void printResults() const;
```

**Saída:**
- Exibe resultado de cada operação executada

**Exemplo:**
```cpp
fsm.printResults();
// Saída:
// Sistema de arquivos =>
// Operação 1 => Sucesso
// Arquivo 'A' criado com sucesso nos blocos [8, 9, 10, 11, 12].
// Operação 2 => Falha
// Arquivo 'X' deletado com sucesso.
```

---

### printFinalDiskMap()

```cpp
void printFinalDiskMap() const;
```

**Saída:**
- Exibe mapa final do disco após todas operações

**Exemplo:**
```cpp
fsm.printFinalDiskMap();
// Saída:
// Mapa de ocupação do disco:
// A A A B B 0 C C C
```

---

### getResults()

```cpp
const std::vector<FileOperationResult> &getResults() const;
```

**Retorno:**
- Vetor com resultados de todas as operações

**Exemplo:**
```cpp
const auto &results = fsm.getResults();
for (const auto &result : results) {
    std::cout << result.message << "\n";
}
```

---

### getFileSystem()

```cpp
const FileSystem *getFileSystem() const;
```

**Retorno:**
- Ponteiro const para o FileSystem interno

**Exemplo:**
```cpp
const FileSystem *fs = fsm.getFileSystem();
std::cout << fs->getAvailableSpace() << "\n";
```

---

## Padrões de Uso Comuns

### Padrão 1: Uso Básico

```cpp
#include "FileSystem/FileSystem.h"

FileSystem fs(20);
auto r1 = fs.createFile(0, "A", 3, 1);
auto r2 = fs.createFile(1, "B", 2, 1);
auto r3 = fs.deleteFile(0, "A", 1);
```

### Padrão 2: Uso com Gerenciador

```cpp
#include "FileSystem/FileSystemManager.h"

FileSystemManager fsm(20);
fsm.loadDiskConfiguration("files.txt");
fsm.registerProcess(0, 0);
fsm.registerProcess(1, 1);
fsm.executeAllOperations();
fsm.printResults();
fsm.printFinalDiskMap();
```

### Padrão 3: Operações Individuais com Manager

```cpp
FileSystemManager fsm(20);
fsm.registerProcess(0, 0);

FileOperation op;
op.processPID = 0;
op.op = CREATE;
op.fileName = "myfile";
op.numBlocks = 5;

auto result = fsm.executeOperation(op);
if (result.success) {
    std::cout << "Sucesso: " << result.message << "\n";
}
```

### Padrão 4: Consultas e Verificações

```cpp
if (fs.getAvailableSpace() >= numBlocksNeeded) {
    auto result = fs.createFile(pid, name, numBlocksNeeded, priority);
    if (result.success) {
        std::cout << "Criado em: ";
        for (int block : result.allocatedBlocks) {
            std::cout << block << " ";
        }
        std::cout << "\n";
    }
}
```

---

## Constantes e Convenções

| Símbolo | Significado |
|---------|------------|
| `0` (blocos) | Bloco livre |
| `priority=0` | Processo de tempo real |
| `priority>0` | Processo de usuário |
| Letra (A-Z) | Nome de arquivo |
| `ownerPID=-1` | Arquivo pré-existente |

---

## Notas Importantes

1. **Thread-Safety**: Todas as operações são protegidas por mutex recursivo
2. **First-Fit**: Sempre aloca no primeiro espaço disponível
3. **Alocação Contígua**: Arquivos ocupam blocos contíguos
4. **Sem Fragmentação**: Disco pode ficar fragmentado com deletions
5. **Sem Compactação**: Sistema não realoca arquivos automaticamente
