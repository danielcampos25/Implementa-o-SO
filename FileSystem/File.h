#ifndef FILE_H
#define FILE_H

#include <string>
#include <vector>

/*
 * Estrutura que representa um arquivo no disco.
 * Cada arquivo tem um nome, um bloco inicial, quantidade de blocos,
 * e o PID do processo que o criou.
 */
struct File
{
    std::string name;      // Nome do arquivo (ex: 'A', 'B', ...)
    int startBlock;        // Primeiro bloco do arquivo
    int numBlocks;         // Quantidade de blocos ocupados
    int ownerPID;          // PID do processo que criou o arquivo
    
    File(const std::string &n, int start, int blocks, int owner)
        : name(n), startBlock(start), numBlocks(blocks), ownerPID(owner) {}
    
    File() : name(""), startBlock(-1), numBlocks(0), ownerPID(-1) {}
};

/*
 * Enum para códigos de operação de arquivo
 */
enum FileOperationType
{
    CREATE = 0,
    DELETE = 1
};

/*
 * Estrutura que representa uma operação a ser realizada no sistema de arquivos
 */
struct FileOperation
{
    int processPID;        // ID do processo que faz a operação
    FileOperationType op;  // Tipo de operação (CREATE ou DELETE)
    std::string fileName;  // Nome do arquivo
    int numBlocks;         // Número de blocos (apenas para CREATE)
    
    FileOperation() : processPID(-1), op(CREATE), fileName(""), numBlocks(0) {}
};

#endif
