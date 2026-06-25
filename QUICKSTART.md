# 🚀 Quick Start - Módulo de Arquivos

## 5 Minutos para Começar

### 1️⃣ Compile os Testes (1 minuto)
```bash
cd SO_implementacao.worktrees/agents-implementacao-modulo-arquivos
g++ -std=c++17 -Wall -Wextra FileSystem/FileSystem.cpp \
    FileSystem/FileSystemManager.cpp test_filesystem.cpp \
    -o test_filesystem
```

### 2️⃣ Execute os Testes (1 minuto)
```bash
./test_filesystem
```
Você verá 5 testes diferentes demonstrando todas as funcionalidades.

### 3️⃣ Execute o Exemplo Completo (1 minuto)
```bash
g++ -std=c++17 -Wall -Wextra FileSystem/FileSystem.cpp \
    FileSystem/FileSystemManager.cpp example_complete.cpp \
    -o example_complete
./example_complete
```

### 4️⃣ Leia a Documentação (2 minutos)
```bash
# Visão geral
cat FileSystem/README.md

# Tutorial
cat FileSystem/TUTORIAL.md

# Referência
cat FileSystem/API_REFERENCE.md
```

---

## 📚 Leitura Recomendada

| Tempo | Conteúdo | Arquivo |
|-------|----------|---------|
| 2 min | Visão geral | `FileSystem/README.md` |
| 5 min | Tutorial | `FileSystem/TUTORIAL.md` |
| 3 min | Como integrar | `FileSystem/INTEGRATION.md` |
| 10 min | Referência técnica | `FileSystem/API_REFERENCE.md` |

---

## 💻 Uso Básico em 30 segundos

```cpp
#include "FileSystem/FileSystem.h"
#include <iostream>

int main() {
    // Criar disco com 20 blocos
    FileSystem fs(20);
    
    // Criar arquivo
    auto r1 = fs.createFile(0, "app", 5, 1);
    std::cout << (r1.success ? "Sucesso!" : "Erro") << "\n";
    
    // Ver mapa do disco
    std::cout << fs.getDiskMapString() << "\n";
    
    // Deletar arquivo
    fs.deleteFile(0, "app", 1);
    
    return 0;
}
```

Compile:
```bash
g++ -std=c++17 FileSystem/FileSystem.cpp seu_arquivo.cpp -o seu_exe
```

---

## 📋 Arquivos Principais

| Arquivo | Propósito | Linhas |
|---------|-----------|--------|
| `FileSystem/FileSystem.h` | Interface | 62 |
| `FileSystem/FileSystem.cpp` | Implementação | 325 |
| `FileSystem/FileSystemManager.h` | Gerenciador | 38 |
| `FileSystem/FileSystemManager.cpp` | Gerenciador impl. | 270 |

---

## 🎯 O Que Você Pode Fazer

✅ Criar arquivos em disco  
✅ Deletar arquivos  
✅ Controlar permissões por processo  
✅ Visualizar mapa do disco  
✅ Carregar estado pré-existente  
✅ Usar de forma thread-safe  

---

## 🔑 Conceitos Principais

**Alocação Contígua**: Arquivos ocupam blocos sequenciais
```
Disco: [A][A][A][B][B][0][0]
```

**First-Fit**: Aloca no primeiro espaço livre
```
Busca: [A][A][A] -> encontra [0][0] em posição 5
Aloca: [A][A][A][B][B][C][C]
```

**Permissões**:
- Tempo real (priority=0): Acesso total
- Usuário (priority>1): Acesso limitado

---

## ⚡ Comandos Rápidos

### Compilar tudo
```bash
cd SO_implementacao.worktrees/agents-implementacao-modulo-arquivos
g++ -std=c++17 FileSystem/*.cpp test_filesystem.cpp -o test_fs
g++ -std=c++17 FileSystem/*.cpp example_complete.cpp -o example
```

### Executar
```bash
./test_fs
./example
```

### Integrar com seu código
```bash
g++ -std=c++17 FileSystem/FileSystem.cpp \
    FileSystem/FileSystemManager.cpp \
    seu_arquivo.cpp -o seu_exe
```

---

## 🐛 Troubleshooting

**Erro: file not found**
```bash
# Verifique se está no diretório correto
cd SO_implementacao.worktrees/agents-implementacao-modulo-arquivos

# Use caminhos relativos corretos
g++ -std=c++17 FileSystem/FileSystem.cpp ...
```

**Erro: undefined reference**
```bash
# Certifique-se de compilar os .cpp corretos
g++ FileSystem/FileSystem.cpp FileSystem/FileSystemManager.cpp seu_arquivo.cpp
```

**Warnings sobre parâmetros não usados**
```
# Não é problema - são warnings aceitáveis
# Pode suprimir com -Wno-unused-parameter se necessário
```

---

## 📞 Ajuda

1. **Entender o módulo**: `FileSystem/README.md`
2. **Aprender a usar**: `FileSystem/TUTORIAL.md`
3. **Integrar com SO**: `FileSystem/INTEGRATION.md`
4. **Referência API**: `FileSystem/API_REFERENCE.md`
5. **Índice completo**: `FileSystem/INDEX.md`

---

## ✅ Validação

Após compilar, você deve conseguir:
- ✅ Executar `test_filesystem` sem erros
- ✅ Executar `example_complete` sem erros
- ✅ Ver saída esperada nos exemplos
- ✅ Compilar seu próprio código usando as classes

Se tudo funcionar, você está pronto para usar! 🎉

---

## 🎁 O Que Vem A Seguir

1. **Explore o código**: Leia `FileSystem.cpp` para entender como funciona
2. **Integre com SO**: Siga `FileSystem/INTEGRATION.md`
3. **Customize**: Adapte conforme necessário
4. **Teste**: Crie seus próprios testes

---

## 📝 Exemplo Completo (10 linhas)

```cpp
#include "FileSystem/FileSystemManager.h"

int main() {
    FileSystemManager fsm(20);
    fsm.loadDiskConfiguration("examples/files_example1.txt");
    fsm.registerProcess(0, 0);
    fsm.registerProcess(1, 1);
    fsm.executeAllOperations();
    fsm.printResults();
    fsm.printFinalDiskMap();
    return 0;
}
```

Compile:
```bash
g++ -std=c++17 FileSystem/FileSystem.cpp \
    FileSystem/FileSystemManager.cpp seu_arquivo.cpp -o seu_exe
```

Execute:
```bash
./seu_exe
```

---

**Pronto para usar! 🚀**

Comece com: `cat FileSystem/README.md`
