#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include "File.h"
#include <vector>
#include <string>
#include <mutex>

/*
 * Estrutura que representa o resultado de uma operação de arquivo
 */
struct FileOperationResult
{
    bool success;          // Se a operação teve sucesso
    std::string message;   // Mensagem descritiva do resultado
    std::vector<int> allocatedBlocks; // Blocos alocados (para CREATE)
    
    FileOperationResult(bool s = false, const std::string &msg = "")
        : success(s), message(msg) {}
};

/*
 * FileSystem gerencia a alocação de espaço em disco usando alocação contígua
 * e algoritmo first-fit. Cada processo pode criar/deletar arquivos com
 * restrições de permissão baseadas em prioridade e propriedade.
 */
class FileSystem
{
private:
    int totalBlocks;                    // Total de blocos no disco
    std::vector<int> diskMap;           // Mapa do disco: 0=livre, caso contrário=índice do arquivo
    std::vector<File> files;            // Lista de arquivos no disco
    mutable std::recursive_mutex mtx;   // Proteção de acesso concorrente
    
    // Operações auxiliares
    int findContiguousSpace(int numBlocks);
    int findFileByName(const std::string &name);
    bool isRealTimeProcess(int processPID);
    std::string allocateFileOnDisk(const std::string &fileName, int numBlocks, int ownerPID);
    
public:
    FileSystem(int totalBlocks = 20);
    
    // Operações principais
    FileOperationResult createFile(int processPID, const std::string &fileName, int numBlocks, int priority = 1);
    FileOperationResult deleteFile(int processPID, const std::string &fileName, int priority = 1);
    
    // Operações de carregamento
    bool loadDiskState(const std::string &diskStateString);
    
    // Operações de consulta
    int getAvailableSpace() const;
    int getTotalSpace() const;
    
    // Operações de exibição
    void printDiskMap() const;
    void printFileList() const;
    std::string getDiskMapString() const;
};

#endif
