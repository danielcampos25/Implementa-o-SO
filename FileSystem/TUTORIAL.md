# Tutorial de Uso - Módulo de Arquivos

## 1. Estrutura Básica

O módulo de arquivos é composto por:
- `File.h` - Estruturas de dados
- `FileSystem.h/cpp` - Core do sistema de arquivos
- `FileSystemManager.h/cpp` - Gerenciador de alto nível
- `test_filesystem.cpp` - Testes unitários

## 2. Como Usar

### Uso Básico

```cpp
#include "FileSystem/FileSystem.h"

// Criar sistema com 20 blocos
FileSystem fs(20);

// Criar arquivo
auto result = fs.createFile(
    0,           // PID do processo
    "MyFile",    // Nome do arquivo
    5,           // Número de blocos
    1            // Prioridade (0=tempo real, >0=usuário)
);

if (result.success) {
    std::cout << result.message << "\n";
} else {
    std::cout << "Erro: " << result.message << "\n";
}

// Deletar arquivo
fs.deleteFile(0, "MyFile", 1);

// Exibir mapa do disco
std::cout << fs.getDiskMapString() << "\n";
```

### Uso com FileSystemManager

```cpp
#include "FileSystem/FileSystemManager.h"

// Criar gerenciador com 20 blocos
FileSystemManager fsm(20);

// Registrar processos
fsm.registerProcess(0, 0);  // Tempo real
fsm.registerProcess(1, 1);  // Usuário

// Carregar configuração de arquivo
fsm.loadDiskConfiguration("files.txt");

// Executar todas as operações
fsm.executeAllOperations();

// Exibir resultados
fsm.printResults();
fsm.printFinalDiskMap();
```

## 3. Formato do Arquivo de Configuração

### Estrutura
```
<total_blocos>
<num_segmentos_iniciais>
[arquivo1,bloco_inicial1,num_blocos1]
[arquivo2,bloco_inicial2,num_blocos2]
...
<pid>,<operacao>,<nome_arquivo>[,<num_blocos>]
...
```

### Campos:
- `total_blocos`: Número total de blocos no disco (ex: 20)
- `num_segmentos_iniciais`: Quantidade de arquivos pré-existentes (ex: 3)
- `arquivo`: Nome do arquivo (letra única, ex: A, B, C)
- `bloco_inicial`: Primeiro bloco do arquivo (ex: 0, 5, 10)
- `num_blocos`: Quantidade de blocos do arquivo (ex: 2, 3, 4)
- `pid`: ID do processo (ex: 0, 1, 2)
- `operacao`: 0=criar, 1=deletar
- `nome_arquivo`: Nome do arquivo a operar (ex: FileA, FileB)
- `num_blocos`: (apenas para criar) blocos a alocar

### Exemplo Prático

```
20
2
SYSTEM,0,4
DATA,5,3
0,0,LOG,5
1,0,TEMP,2
0,1,SYSTEM
1,1,LOG
0,0,BACKUP,4
```

**Interpretação:**
1. Disco: 20 blocos
2. 2 arquivos pré-existentes:
   - SYSTEM: blocos 0-3 (4 blocos)
   - DATA: blocos 5-7 (3 blocos)
3. Operações:
   - Processo 0 cria LOG (5 blocos)
   - Processo 1 cria TEMP (2 blocos)
   - Processo 0 deleta SYSTEM
   - Processo 1 deleta LOG
   - Processo 0 cria BACKUP (4 blocos)

## 4. Regras de Acesso

### Permissões por Tipo de Processo

| Ação | Tempo Real (pri=0) | Usuário (pri>0) |
|------|:------------------:|:---------------:|
| Criar arquivo | ✓ | ✓ |
| Deletar qualquer arquivo | ✓ | ✗ |
| Deletar arquivo próprio | ✓ | ✓ |

### Erros Comuns

1. **Arquivo não existe**
   - Tentar deletar arquivo inexistente
   - Resultado: "Arquivo 'X' não existe."

2. **Sem permissão**
   - Usuário tenta deletar arquivo de outro
   - Resultado: "Processo X não tem permissão..."

3. **Sem espaço**
   - Disco cheio ou fragmentado
   - Resultado: "Falta de espaço disponível no disco."

4. **Arquivo duplicado**
   - Tentar criar arquivo com nome existente
   - Resultado: "Arquivo 'X' já existe."

## 5. Compreendendo o Mapa de Disco

O mapa de disco mostra a ocupação em tempo real:

```
A A A B B 0 0 C C C 0 D D D D
```

**Legenda:**
- `A, B, C, D`: Letras indicam nomes de arquivos
- `0`: Bloco vazio/livre
- Sequências contíguas: Partes do mesmo arquivo

**Exemplo de leitura:**
- Blocos 0-2: Arquivo A (3 blocos)
- Blocos 3-4: Arquivo B (2 blocos)
- Blocos 5-6: Vazios
- Blocos 7-9: Arquivo C (3 blocos)
- Bloco 10: Vazio
- Blocos 11-14: Arquivo D (4 blocos)

## 6. Algoritmo First-Fit Explicado

O sistema usa **first-fit** para alocação:

### Passo a passo

```
Disco inicial: [0][0][0][0][0][0][0][0][0][0]

Criar A (3 blocos):
-> Varre do início: encontra blocos 0-2 livres
-> Aloca A nos blocos 0-2
Disco: [A][A][A][0][0][0][0][0][0][0]

Criar B (2 blocos):
-> Varre do início: blocos 0-2 ocupados
-> Encontra blocos 3-4 livres
-> Aloca B nos blocos 3-4
Disco: [A][A][A][B][B][0][0][0][0][0]

Deletar A:
-> Libera blocos 0-2
Disco: [0][0][0][B][B][0][0][0][0][0]

Criar C (2 blocos):
-> Varre do início: encontra blocos 0-1 livres (first-fit!)
-> Aloca C nos blocos 0-1
Disco: [C][C][0][B][B][0][0][0][0][0]
```

**Características:**
- Sempre toma o primeiro espaço disponível
- Não otimiza para fragmentação
- Simples de implementar e entender

## 7. Compilando Exemplos Personalizados

### Criar seu próprio teste

```cpp
#include "FileSystem/FileSystem.h"
#include <iostream>

int main() {
    FileSystem fs(25);
    
    // Criar seus testes aqui
    auto result = fs.createFile(0, "TEST", 5, 1);
    std::cout << result.message << "\n";
    
    std::cout << "Mapa: " << fs.getDiskMapString() << "\n";
    
    return 0;
}
```

### Compilar

```bash
g++ -std=c++17 FileSystem/FileSystem.cpp meu_teste.cpp -o meu_teste
./meu_teste
```

## 8. Troubleshooting

### Compilação com undefined reference

**Problema**: Linker não encontra implementações

**Solução**: Incluir arquivos .cpp na compilação:
```bash
g++ -std=c++17 FileSystem/FileSystem.cpp FileSystem/FileSystemManager.cpp seu_arquivo.cpp -o seu_exe
```

### Warning sobre unused parameter

**Problema**: Warnings de parâmetros não usados

**Solução**: Esperado. Use `-Wno-unused-parameter` para suprimir se necessário.

### Caracteres estranhos na saída

**Problema**: Encoding de caracteres acentuados

**Solução**: Esperado em Windows. Funciona corretamente em Linux.

## 9. Próximas Melhorias

Sugestões para versões futuras:
1. Compactação de disco
2. Alocação dinâmica
3. Suporte a permissões granulares
4. Quotas de espaço por processo
5. Histórico de operações/log
