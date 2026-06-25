#include "FileSystem/FileSystem.h"
#include "FileSystem/FileSystemManager.h"
#include <iostream>

/*
 * Exemplo Completo: Sistema de Arquivos Integrado
 * Este exemplo demonstra todas as funcionalidades do módulo FileSystem
 */

void printSection(const std::string &title)
{
    std::cout << "\n" << std::string(50, '=') << "\n";
    std::cout << " " << title << "\n";
    std::cout << std::string(50, '=') << "\n";
}

void demonstrateBasicFileSystem()
{
    printSection("1. EXEMPLO BÁSICO - FileSystem");
    
    std::cout << "Criando sistema com 15 blocos...\n";
    FileSystem fs(15);
    
    std::cout << "\n--- Criando Arquivos ---\n";
    auto r1 = fs.createFile(0, "App1", 3, 1);
    std::cout << "Resultado: " << (r1.success ? "✓ " : "✗ ") << r1.message << "\n";
    
    auto r2 = fs.createFile(1, "App2", 4, 1);
    std::cout << "Resultado: " << (r2.success ? "✓ " : "✗ ") << r2.message << "\n";
    
    std::cout << "\nEstado atual do disco:\n";
    std::cout << "Mapa: " << fs.getDiskMapString() << "\n";
    std::cout << "Blocos livres: " << fs.getAvailableSpace() << "/" << fs.getTotalSpace() << "\n";
    
    std::cout << "\n--- Deletando Arquivo ---\n";
    auto r3 = fs.deleteFile(0, "App1", 1);
    std::cout << "Resultado: " << (r3.success ? "✓ " : "✗ ") << r3.message << "\n";
    std::cout << "Mapa pós-deleção: " << fs.getDiskMapString() << "\n";
}

void demonstratePermissions()
{
    printSection("2. CONTROLE DE PERMISSÕES");
    
    FileSystem fs(20);
    
    std::cout << "\n--- Processo Usuário (priority=1) ---\n";
    auto r1 = fs.createFile(5, "UserFile", 3, 1);
    std::cout << "P5 cria arquivo: " << (r1.success ? "✓" : "✗") << " - " << r1.message << "\n";
    
    std::cout << "\n--- Outro Usuário tenta Deletar ---\n";
    auto r2 = fs.deleteFile(10, "UserFile", 1);
    std::cout << "P10 deleta arquivo de P5: " << (r2.success ? "✓" : "✗") << " - " << r2.message << "\n";
    
    std::cout << "\n--- Tempo Real (priority=0) Deleta ---\n";
    auto r3 = fs.deleteFile(0, "UserFile", 0);
    std::cout << "P0 (RT) deleta arquivo de P5: " << (r3.success ? "✓" : "✗") << " - " << r3.message << "\n";
}

void demonstrateFirstFit()
{
    printSection("3. ALGORITMO FIRST-FIT");
    
    FileSystem fs(20);
    
    std::cout << "\nFragmentando o disco...\n\n";
    
    // Criar três arquivos
    fs.createFile(0, "A", 2, 1);
    std::cout << "Criar A (2 blocos): " << fs.getDiskMapString() << "\n";
    
    fs.createFile(1, "B", 3, 1);
    std::cout << "Criar B (3 blocos): " << fs.getDiskMapString() << "\n";
    
    fs.createFile(2, "C", 2, 1);
    std::cout << "Criar C (2 blocos): " << fs.getDiskMapString() << "\n";
    
    // Deletar arquivo do meio
    std::cout << "\nDeletando B (arquivo do meio)...\n";
    fs.deleteFile(1, "B", 1);
    std::cout << "Pós-deleção:        " << fs.getDiskMapString() << "\n";
    
    // Criar novo arquivo (deve usar first-fit)
    std::cout << "\nCriando D (2 blocos com first-fit)...\n";
    fs.createFile(3, "D", 2, 1);
    std::cout << "Resultado:          " << fs.getDiskMapString() << "\n";
    std::cout << "✓ D foi alocado no primeiro espaço livre (blocos 2-3)\n";
}

void demonstrateErrorHandling()
{
    printSection("4. TRATAMENTO DE ERROS");
    
    FileSystem fs(10);
    
    std::cout << "\n--- Erro 1: Arquivo Duplicado ---\n";
    fs.createFile(0, "Test", 3, 1);
    auto r1 = fs.createFile(0, "Test", 2, 1);
    std::cout << "Resultado: " << r1.message << "\n";
    
    std::cout << "\n--- Erro 2: Sem Espaço ---\n";
    fs.createFile(1, "Big", 7, 1);
    auto r2 = fs.createFile(2, "Overflow", 5, 1);
    std::cout << "Resultado: " << r2.message << "\n";
    
    std::cout << "\n--- Erro 3: Arquivo não Existe ---\n";
    auto r3 = fs.deleteFile(0, "Phantom", 1);
    std::cout << "Resultado: " << r3.message << "\n";
    
    std::cout << "\n--- Erro 4: Blocos Inválidos ---\n";
    auto r4 = fs.createFile(0, "Invalid", -5, 1);
    std::cout << "Resultado: " << r4.message << "\n";
}

void demonstrateLoadState()
{
    printSection("5. CARREGAMENTO DE ESTADO");
    
    FileSystem fs(15);
    
    std::cout << "\nCarregando disco pré-existente...\n";
    std::cout << "Estado: SYSTEM:0:3|DATA:4:2|LOG:7:2\n\n";
    
    fs.loadDiskState("SYSTEM:0:3|DATA:4:2|LOG:7:2");
    
    std::cout << "Disco carregado: " << fs.getDiskMapString() << "\n";
    std::cout << "Blocos livres: " << fs.getAvailableSpace() << "/15\n";
    
    std::cout << "\nAdicionando novo arquivo...\n";
    fs.createFile(0, "TEMP", 3, 1);
    std::cout << "Disco final: " << fs.getDiskMapString() << "\n";
}

void demonstrateFileSystemManager()
{
    printSection("6. GERENCIADOR DE ALTO NÍVEL");
    
    std::cout << "\nCriando gerenciador com 20 blocos...\n";
    FileSystemManager fsm(20);
    
    std::cout << "\nRegistrando processos...\n";
    fsm.registerProcess(0, 0);  // Tempo real
    fsm.registerProcess(1, 1);  // Usuário
    fsm.registerProcess(2, 1);  // Usuário
    
    std::cout << "- P0: Tempo real (priority=0)\n";
    std::cout << "- P1: Usuário (priority=1)\n";
    std::cout << "- P2: Usuário (priority=1)\n";
    
    std::cout << "\nExecutando operações...\n";
    std::vector<FileOperation> ops;
    
    // Operação 1: Tempo real cria KERNEL
    FileOperation op1;
    op1.processPID = 0;
    op1.op = CREATE;
    op1.fileName = "KERNEL";
    op1.numBlocks = 4;
    ops.push_back(op1);
    
    // Operação 2: Usuário cria DATA
    FileOperation op2;
    op2.processPID = 1;
    op2.op = CREATE;
    op2.fileName = "DATA";
    op2.numBlocks = 3;
    ops.push_back(op2);
    
    // Operação 3: Usuário cria CACHE
    FileOperation op3;
    op3.processPID = 2;
    op3.op = CREATE;
    op3.fileName = "CACHE";
    op3.numBlocks = 2;
    ops.push_back(op3);
    
    // Operação 4: P1 deleta DATA (seu arquivo)
    FileOperation op4;
    op4.processPID = 1;
    op4.op = DELETE;
    op4.fileName = "DATA";
    op4.numBlocks = 0;
    ops.push_back(op4);
    
    // Operação 5: P2 tenta deletar KERNEL (não é dono)
    FileOperation op5;
    op5.processPID = 2;
    op5.op = DELETE;
    op5.fileName = "KERNEL";
    op5.numBlocks = 0;
    ops.push_back(op5);
    
    for (size_t i = 0; i < ops.size(); i++) {
        auto result = fsm.executeOperation(ops[i]);
        std::string op_name = (ops[i].op == 0) ? "CREATE" : "DELETE";
        std::cout << "\nOperação " << (i+1) << ": P" << ops[i].processPID 
                  << " " << op_name << " \"" << ops[i].fileName << "\"\n";
        std::cout << "  Resultado: " << (result.success ? "✓ Sucesso" : "✗ Falha") << "\n";
        std::cout << "  Msg: " << result.message << "\n";
    }
    
    std::cout << "\n--- Mapa Final ---\n";
    std::cout << fsm.getFileSystem()->getDiskMapString() << "\n";
}

void demonstrateSpaceManagement()
{
    printSection("7. GERENCIAMENTO DE ESPAÇO");
    
    FileSystem fs(20);
    
    std::cout << "\nStatus inicial:\n";
    std::cout << "Total: " << fs.getTotalSpace() << ", Livre: " << fs.getAvailableSpace() << "\n";
    
    std::cout << "\nCriando arquivos progressivamente...\n";
    
    int files = 0;
    for (int i = 0; i < 5; i++) {
        std::string name = "F" + std::to_string(i);
        auto r = fs.createFile(0, name, 3, 1);
        if (r.success) {
            files++;
            std::cout << "✓ " << name << " (3 blocos): ";
            std::cout << fs.getAvailableSpace() << " blocos livres\n";
        } else {
            std::cout << "✗ " << name << ": " << r.message << "\n";
        }
    }
    
    std::cout << "\nResumo:\n";
    std::cout << "Arquivos criados: " << files << "\n";
    std::cout << "Espaço usado: " << (fs.getTotalSpace() - fs.getAvailableSpace()) << "\n";
    std::cout << "Espaço livre: " << fs.getAvailableSpace() << "\n";
    std::cout << "Taxa de ocupação: " 
              << ((100.0 * (fs.getTotalSpace() - fs.getAvailableSpace())) / fs.getTotalSpace())
              << "%\n";
}

int main()
{
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════╗\n";
    std::cout << "║     EXEMPLO COMPLETO - MÓDULO DE ARQUIVOS         ║\n";
    std::cout << "║   Demonstração de Todas as Funcionalidades        ║\n";
    std::cout << "╚════════════════════════════════════════════════════╝\n";
    
    try {
        demonstrateBasicFileSystem();
        demonstratePermissions();
        demonstrateFirstFit();
        demonstrateErrorHandling();
        demonstrateLoadState();
        demonstrateFileSystemManager();
        demonstrateSpaceManagement();
        
        printSection("✓ TODOS OS EXEMPLOS COMPLETADOS COM SUCESSO");
        
        std::cout << "\nResumo das Funcionalidades Testadas:\n";
        std::cout << "✓ Criação de arquivos\n";
        std::cout << "✓ Deleção de arquivos\n";
        std::cout << "✓ Controle de permissões\n";
        std::cout << "✓ Alocação first-fit\n";
        std::cout << "✓ Tratamento de erros\n";
        std::cout << "✓ Carregamento de estado\n";
        std::cout << "✓ Gerenciador de alto nível\n";
        std::cout << "✓ Gerenciamento de espaço\n";
        
        std::cout << "\n";
    }
    catch (const std::exception &e) {
        std::cerr << "Erro: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
