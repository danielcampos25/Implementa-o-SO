# Guia de Integração - Módulo de Arquivos com Pseudo-SO

## 1. Visão Geral da Integração

O módulo de arquivos deve integrar-se com:
- **Scheduler**: Para conhecer prioridade dos processos
- **ResourceManager**: Para recursos de I/O durante operações
- **Dispatcher**: Para coordenar execução de operações

## 2. Arquitetura de Integração

```
┌─────────────────┐
│  Dispatcher     │
└────────┬────────┘
         │
    ┌────┴──────────────────────┐
    │                           │
┌───▼─────────────┐    ┌───────▼──────────┐
│  Scheduler      │    │ FileSystemMgr    │
└───┬─────────────┘    └───────┬──────────┘
    │                          │
┌───▼──────────────┐    ┌──────▼───────────┐
│ ResourceManager  │    │  FileSystem      │
└──────────────────┘    └──────────────────┘
```

## 3. Estrutura de Dados Integrada

### Classe ProcessInfo Estendida

```cpp
struct ProcessInfo {
    int pid;
    int priority;           // 0=tempo real, >0=usuário
    int state;             // READY, RUNNING, BLOCKED, TERMINATED
    int totalCPUTime;
    int executedTime;
    // ... outros campos
};
```

### Fila de Operações de Arquivo

```cpp
struct FileOperationQueue {
    std::queue<FileOperation> pending;
    std::vector<FileOperationResult> completed;
};
```

## 4. Modificações no FileSystem

### Atualizar isRealTimeProcess()

**Antes:**
```cpp
bool FileSystem::isRealTimeProcess(int processPID) {
    return false;  // TODO: Integrar com Scheduler
}
```

**Depois:**
```cpp
bool FileSystem::isRealTimeProcess(int processPID) {
    // Consultar Scheduler para prioridade real do processo
    if (scheduler == nullptr)
        return false;
    
    int priority = scheduler->getProcessPriority(processPID);
    return priority == 0;  // 0 = tempo real
}
```

### Adicionar Referência ao Scheduler

```cpp
class FileSystem {
private:
    Scheduler *scheduler;
    // ...

public:
    void setScheduler(Scheduler *s) { scheduler = s; }
    // ...
};
```

## 5. Modificações no FileSystemManager

### Integrar com Scheduler

```cpp
class FileSystemManager {
private:
    Scheduler *scheduler;
    ResourceManager *resourceManager;
    
public:
    void setScheduler(Scheduler *s) { 
        scheduler = s;
        fileSystem->setScheduler(s);
    }
    
    void setResourceManager(ResourceManager *rm) {
        resourceManager = rm;
    }
    
    // Executar operação respeitando disponibilidade de recursos
    FileOperationResult executeOperation(const FileOperation &op) {
        // Verificar se processo pode acessar I/O
        if (!resourceManager->canAccessDisk(op.processPID)) {
            return FileOperationResult(false, 
                "Processo aguardando disponibilidade de I/O");
        }
        
        // Obter prioridade do scheduler
        int priority = scheduler->getProcessPriority(op.processPID);
        
        if (op.op == CREATE) {
            return fileSystem->createFile(op.processPID, 
                                        op.fileName, 
                                        op.numBlocks, 
                                        priority);
        }
        // ... resto da implementação
    }
};
```

## 6. No Dispatcher Principal

### Inicialização

```cpp
#include "FileSystem/FileSystemManager.h"
#include "ResourceManager/ResourceManager.h"
#include "Scheduler/scheduler.h"

int main(int argc, char *argv[]) {
    // Arquivos de entrada
    std::string processFile = argv[1];
    std::string filesFile = argv[2];
    std::string stringsFile = argv[3];
    
    // Criar componentes
    Scheduler scheduler;
    ResourceManager resourceManager(&scheduler);
    FileSystemManager fileSystemMgr(20);
    
    // Integrar componentes
    fileSystemMgr.setScheduler(&scheduler);
    fileSystemMgr.setResourceManager(&resourceManager);
    
    // Carregar configuração
    fileSystemMgr.loadDiskConfiguration(filesFile);
    
    // ... resto da implementação do dispatcher
    
    return 0;
}
```

### Durante Criação de Processos

```cpp
// Quando criar cada processo
for (int i = 0; i < numProcesses; i++) {
    int pid = scheduler.createProcess(/* ... */);
    int priority = /* ler do arquivo */;
    
    // Registrar no FileSystemManager
    fileSystemMgr.registerProcess(pid, priority);
}
```

### Durante Execução de Processos

```cpp
// Na loop de escalonamento
while (processosNaoFinalizados) {
    Process *current = scheduler.scheduleNext();
    
    if (current) {
        // Executar instruções do processo
        current->execute();
        
        // Se o processo tiver operação de arquivo pendente
        if (current->hasFileOperation()) {
            FileOperation op = current->getFileOperation();
            FileOperationResult result = fileSystemMgr.executeOperation(op);
            
            if (!result.success) {
                // Bloquear processo ou registrar erro
                scheduler.blockProcess(current->pid, 
                    "Esperando I/O de arquivo");
            }
        }
        
        // Quantum expirou?
        if (current->hasQuantumExpired() && current->priority > 0) {
            scheduler.preemptProcess(current->pid);
        }
    }
}
```

### Finalização

```cpp
// Após executar todos os processos
std::cout << "\nSistema de arquivos =>\n";
fileSystemMgr.printResults();
fileSystemMgr.printFinalDiskMap();
```

## 7. Fluxo de Operação Integrado

### Operação: Criar Arquivo

```
Processo executa: criar "arquivo.txt" com 5 blocos
    ↓
FileSystemManager consulta prioridade via Scheduler
    ↓
FileSystem verifica se é tempo real
    ↓
FileSystem verifica se arquivo já existe
    ↓
FileSystem aloca espaço contíguo (first-fit)
    ↓
Se sucesso: arquivo criado e mapa atualizado
Se falha: retorna motivo do erro
    ↓
Resultado registrado para exibição posterior
```

### Operação: Deletar Arquivo

```
Processo executa: deletar "arquivo.txt"
    ↓
FileSystemManager consulta prioridade via Scheduler
    ↓
FileSystem verifica permissões:
  - É tempo real? Pode deletar qualquer um
  - É usuário? Pode deletar apenas seus arquivos
    ↓
Se permissão ok: libera blocos do disco
Se permissão negada: retorna erro de permissão
    ↓
Resultado registrado para exibição posterior
```

## 8. Tratamento de Erros Integrado

### Quando Recurso I/O Indisponível

```cpp
// No FileSystemManager
FileOperationResult result = fileSystem->createFile(...);

if (!result.success && result.message.find("I/O") != std::string::npos) {
    // Bloquear processo no scheduler
    scheduler->blockProcess(op.processPID, "Aguardando I/O");
    
    // Requerer recurso de disco
    ResourceRequest req;
    req.sata = 1;  // Precisa de acesso ao disco
    resourceManager->allocate(req);
}
```

## 9. Sincronização Entre Componentes

### Usar Locks Recursivos

```cpp
class FileSystem {
private:
    mutable std::recursive_mutex mtx;  // Permite locks aninhados
    
    // Quando chamar from multiple threads/contexts
    FileOperationResult createFile(...) {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        // ... implementação
    }
};
```

### Coordenação de Estado

```cpp
// Garantir consistência entre Scheduler e FileSystem
void synchronizeState() {
    // Atualizar prioridades
    std::vector<int> pids = scheduler->getAllPIDs();
    for (int pid : pids) {
        int priority = scheduler->getProcessPriority(pid);
        fileSystemMgr.updateProcessPriority(pid, priority);
    }
}
```

## 10. Testes de Integração

### Teste 1: Criar e Deletar com Permissões Reais

```cpp
void testIntegration1() {
    Scheduler scheduler;
    FileSystemManager fsm(20);
    fsm.setScheduler(&scheduler);
    
    // Criar dois processos com prioridades diferentes
    scheduler.addProcess(0, 0);  // Tempo real
    scheduler.addProcess(1, 1);  // Usuário
    
    fsm.registerProcess(0, 0);
    fsm.registerProcess(1, 1);
    
    // Tempo real cria arquivo
    auto result1 = fsm.executeOperation(
        FileOperation{0, CREATE, "RTFile", 5});
    assert(result1.success);
    
    // Usuário tenta deletar arquivo de tempo real (deve falhar)
    auto result2 = fsm.executeOperation(
        FileOperation{1, DELETE, "RTFile", 0});
    assert(!result2.success);
    
    // Tempo real deleta seu próprio arquivo (deve suceder)
    auto result3 = fsm.executeOperation(
        FileOperation{0, DELETE, "RTFile", 0});
    assert(result3.success);
}
```

## 11. Checklist de Integração

- [ ] Adicionar `#include "FileSystem/FileSystemManager.h"` ao dispatcher
- [ ] Criar instância de `FileSystemManager` na main
- [ ] Chamar `setScheduler()` para integração de prioridades
- [ ] Chamar `setResourceManager()` se necessário
- [ ] Registrar processos com `registerProcess()` conforme criados
- [ ] Carregar configuração com `loadDiskConfiguration()`
- [ ] Executar operações via `executeOperation()` conforme necessário
- [ ] Exibir resultados com `printResults()` e `printFinalDiskMap()`
- [ ] Testar permissões (tempo real vs. usuário)
- [ ] Testar alocação e fragmentação
- [ ] Compilar com todos os `.cpp` necessários

## 12. Compilação Final

```bash
g++ -std=c++17 -Wall -Wextra -pedantic \
    ResourceManager/ResourceManager.cpp \
    Scheduler/scheduler.cpp \
    FileSystem/FileSystem.cpp \
    FileSystem/FileSystemManager.cpp \
    dispatcher.cpp \
    -o dispatcher
```

## 13. Exemplo de Saída Completa

```
dispatcher =>
 PID: 0
 frames: 4
 priority: 0
 time: 3
 printers: 0
 scanners: 0
 modems: 0
 drives: 0
process 0 =>
P0 STARTED
P0 instruction 1
P0 instruction 2
P0 instruction 3
P0 return SIGINT

dispatcher =>
PID: 1
 frames: 8
 priority: 0
 time: 2
 printers: 0
 scanners: 0
 modems: 0
 drives: 0
process 1 =>
P1 STARTED
P1 instruction 1
P1 instruction 2
P1 return SIGINT

Sistema de arquivos =>
Operação 1 => Falha
O processo 0 não pode criar o arquivo A (falta de espaço).
Operação 2 => Sucesso
O processo 0 deletou o arquivo X.
Operação 3 => Falha
O processo 2 não existe.
Operação 4 => Sucesso
O processo 0 criou o arquivo D (blocos 0, 1 e 2).
Operação 5 => Falha
O processo 1 não pode deletar o arquivo E porque ele não existe.

Mapa de ocupação do disco:
D D D Y Z Z Z
```
