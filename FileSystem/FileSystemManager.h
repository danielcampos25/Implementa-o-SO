#ifndef FILE_SYSTEM_MANAGER_H
#define FILE_SYSTEM_MANAGER_H

#include "FileSystem.h"
#include <vector>
#include <string>
#include <memory>

/*
 * Estrutura para armazenar informações sobre um processo
 */
struct ProcessInfo
{
    int pid;
    int priority;
    
    ProcessInfo(int p, int pr) : pid(p), priority(pr) {}
};

/*
 * FileSystemManager é responsável por:
 * 1. Carregar arquivo de operações de disco
 * 2. Gerenciar permissões e validações
 * 3. Executar operações respeitando as regras do pseudo-SO
 * 4. Gerar relatórios de execução
 */
class FileSystemManager
{
private:
    std::unique_ptr<FileSystem> fileSystem;
    std::vector<ProcessInfo> processes;
    std::vector<FileOperation> operations;
    std::vector<FileOperationResult> results;
    
    bool parseDisksFile(const std::string &filename);
    bool parseFileOperations(const std::string &content, int startLine, int endLine);
    
public:
    FileSystemManager(int totalBlocks = 20);
    
    bool loadDiskConfiguration(const std::string &filename);
    bool registerProcess(int pid, int priority);
    bool executeAllOperations();
    
    FileOperationResult executeOperation(const FileOperation &op);
    
    void printResults() const;
    void printFinalDiskMap() const;
    
    const std::vector<FileOperationResult> &getResults() const;
    const FileSystem *getFileSystem() const;
};

#endif
