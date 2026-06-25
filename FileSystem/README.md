# Módulo de Gerenciamento de Arquivos - FileSystem

## Visão Geral

O módulo de gerenciamento de arquivos implementa um sistema de arquivos que:
- Utiliza **alocação contígua** com algoritmo **first-fit**
- Controla permissões de acesso baseado em tipos de processos (tempo real vs. usuário)
- Mantém persistência de dados no disco
- Suporta operações de criar e deletar arquivos

## Estrutura de Classes

### 1. `File` (File.h)
Estrutura que representa um arquivo no disco:
```cpp
struct File {
    std::string name;      // Nome do arquivo (ex: 'A', 'B', ...)
    int startBlock;        // Primeiro bloco do arquivo
    int numBlocks;         // Quantidade de blocos ocupados
    int ownerPID;          // PID do processo que criou o arquivo
};
```

### 2. `FileSystem` (FileSystem.h/cpp)
Gerencia alocação e liberação de espaço em disco:

**Métodos principais:**
- `createFile(processPID, fileName, numBlocks, priority)` - Cria um arquivo
- `deleteFile(processPID, fileName, priority)` - Deleta um arquivo
- `loadDiskState(diskStateString)` - Carrega estado inicial do disco
- `getAvailableSpace()` - Retorna blocos livres
- `getDiskMapString()` - Retorna representação do disco

**Algoritmo de Alocação:**
- Busca sequencial no disco (first-fit)
- Encontra primeira sequência contígua de blocos livres
- Retorna erro se não há espaço suficiente

### 3. `FileSystemManager` (FileSystemManager.h/cpp)
Gerencia operações de arquivo em nível mais alto:

**Métodos principais:**
- `loadDiskConfiguration(filename)` - Carrega configuração do arquivo
- `registerProcess(pid, priority)` - Registra um processo
- `executeAllOperations()` - Executa todas as operações
- `printResults()` - Exibe resultados das operações

## Regras de Acesso

### Processos de Tempo Real (prioridade = 0)
- Podem **criar** arquivos sem restrição
- Podem **deletar** qualquer arquivo, independente de propriedade

### Processos de Usuário (prioridade > 0)
- Podem **criar** quantos arquivos desejarem
- Podem **deletar** apenas arquivos que criaram

## Formato de Entrada (files.txt)

```
<total_blocos>
<num_segmentos>
<arquivo>,<bloco_inicial>,<num_blocos>
...
<pid>,<operacao>,<nome_arquivo>[,<num_blocos_se_criar>]
...
```

**Exemplo:**
```
10
3
X,0,2
Y,3,1
Z,5,3
0,0,A,5
0,1,X
2,0,B,2
0,0,D,3
1,0,E,2
```

### Significado:
- Disco com 10 blocos
- 3 segmentos pré-existentes (X, Y, Z)
- Operações:
  - Processo 0: criar arquivo A com 5 blocos
  - Processo 0: deletar arquivo X
  - Processo 2: criar arquivo B com 2 blocos
  - Processo 0: criar arquivo D com 3 blocos
  - Processo 1: criar arquivo E com 2 blocos

## Mapa de Disco

O mapa de disco mostra a ocupação em tempo real:
- Números/letras indicam quais blocos estão ocupados
- 0 indica bloco vazio
- Sequência contígua de letras iguais = um arquivo

**Exemplo de saída:**
```
Mapa de ocupação do disco:
X X 0 Y 0 Z Z Z 0 0
```

## Exemplo de Compilação e Execução

```bash
# Compilação
g++ -std=c++17 FileSystem/FileSystem.cpp FileSystem/FileSystemManager.cpp test_filesystem.cpp -o test_filesystem

# Execução
./test_filesystem
```

## Integração com o Pseudo-SO

### Para integrar com o Scheduler:
1. Modificar `isRealTimeProcess()` para consultar prioridade real do processo
2. Passar `priority` correto nas chamadas de `createFile()` e `deleteFile()`
3. Implementar interface de fila de operações de arquivo

### Exemplo de integração:
```cpp
// No dispatcher principal
FileSystemManager fsm(20);

// Carregar configuração
fsm.loadDiskConfiguration("files.txt");

// Registrar processos conforme criados
for (int i = 0; i < numProcesses; i++) {
    fsm.registerProcess(i, processPriority[i]);
}

// Executar operações
fsm.executeAllOperations();

// Exibir resultados
fsm.printResults();
fsm.printFinalDiskMap();
```

## Limitações e Considerações

1. **Fragmentação**: Alocação contígua pode gerar fragmentação
2. **First-Fit**: Sempre aloca no primeiro espaço disponível (não é otimizado)
3. **Sem Compactação**: Disco não realoca arquivos para otimizar espaço
4. **Permissões Simplificadas**: Apenas dois níveis (tempo real e usuário)
5. **Sem Criptografia**: Arquivos não são criptografados

## Testes Inclusos

- **Test 1**: Operações básicas de criação e deleção
- **Test 2**: Testes de permissão
- **Test 3**: Testes de alocação de espaço
- **Test 4**: Verificação de first-fit
- **Test 5**: Carregamento de estado do disco

Todos os testes passam com sucesso demonstrando corretude do módulo.
