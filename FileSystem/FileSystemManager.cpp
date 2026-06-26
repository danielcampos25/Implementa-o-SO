#include "FileSystemManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

/*
 * Construtor: inicializa o sistema de arquivos com um número total de blocos
 */
FileSystemManager::FileSystemManager(int totalBlocks)
    : fileSystem(std::make_unique<FileSystem>(totalBlocks))
{
}

/*
 * Registra um processo no sistema para que suas operações de arquivo possam ser validadas
 */
bool FileSystemManager::registerProcess(int pid, int priority)
{
    // Verificar se processo já foi registrado
    for (const auto &proc : processes)
    {
        if (proc.pid == pid)
            return true;
    }
    
    processes.push_back(ProcessInfo(pid, priority));
    return true;
}

/*
 * Carrega a configuração do disco a partir do arquivo files.txt
 * Formato esperado:
 * Linha 1: Quantidade total de blocos
 * Linha 2: Quantidade de segmentos ocupados (n)
 * Linhas 3 a n+2: arquivo, startBlock, numBlocks
 * Linhas n+3 em diante: operações de arquivo
 */
bool FileSystemManager::loadDiskConfiguration(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Erro ao abrir arquivo: " << filename << "\n";
        return false;
    }
    
    std::string line;
    std::vector<std::string> lines;
    
    // Ler todas as linhas
    while (std::getline(file, line))
    {
        // Remover espaços em branco desnecessários
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        
        if (!line.empty())
            lines.push_back(line);
    }
    file.close();
    
    if (lines.size() < 2)
    {
        std::cerr << "Arquivo de configuração inválido (menos de 2 linhas)\n";
        return false;
    }
    
    try
    {
        int totalBlocks = std::stoi(lines[0]);
        int numSegments = std::stoi(lines[1]);
        
        // Recriar FileSystem com o total de blocos correto
        fileSystem = std::make_unique<FileSystem>(totalBlocks);
        
        // Validar estrutura
        if (lines.size() < 2 + numSegments)
        {
            std::cerr << "Número insuficiente de linhas para segmentos\n";
            return false;
        }
        
        // Carregar estado inicial do disco
        std::stringstream diskStateStream;
        for (int i = 0; i < numSegments; ++i)
        {
            std::stringstream ss(lines[2 + i]);
            std::string name, startStr, numStr;
            
            if (!std::getline(ss, name, ',') ||
                !std::getline(ss, startStr, ',') ||
                !std::getline(ss, numStr, ','))
            {
                std::cerr << "Linha de segmento inválida: " << lines[2 + i] << "\n";
                return false;
            }
            
            // Remover espaços
            name.erase(0, name.find_first_not_of(" \t"));
            name.erase(name.find_last_not_of(" \t") + 1);
            startStr.erase(0, startStr.find_first_not_of(" \t"));
            startStr.erase(startStr.find_last_not_of(" \t") + 1);
            numStr.erase(0, numStr.find_first_not_of(" \t"));
            numStr.erase(numStr.find_last_not_of(" \t") + 1);
            
            // Criar arquivo inicial
            int startBlock = std::stoi(startStr);
            int numBlocks = std::stoi(numStr);
            
            // Usar loadDiskState de forma incremental
            File initialFile(name, startBlock, numBlocks, -1);
            
            if (diskStateStream.str().length() > 0)
                diskStateStream << "|";
            diskStateStream << name << ":" << startBlock << ":" << numBlocks;
        }
        
        if (!fileSystem->loadDiskState(diskStateStream.str()))
        {
            std::cerr << "Erro ao carregar estado do disco\n";
            return false;
        }
        
        // Carregar operações de arquivo
        int operationsStartLine = 2 + numSegments;
        if (lines.size() > operationsStartLine)
        {
            if (!parseFileOperations(filename, operationsStartLine, lines.size()))
            {
                std::cerr << "Erro ao carregar operações de arquivo\n";
                return false;
            }
        }
        
        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exceção ao carregar configuração: " << e.what() << "\n";
        return false;
    }
}

/*
 * Faz parsing das operações de arquivo do arquivo de configuração
 */
bool FileSystemManager::parseFileOperations(const std::string &filename, int startLine, int endLine)
{
    std::ifstream file(filename);
    if (!file.is_open())
        return false;
    
    std::string line;
    int currentLine = 0;
    
    while (std::getline(file, line) && currentLine < endLine)
    {
        // Remover espaços
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        
        if (line.empty() || currentLine < startLine)
        {
            currentLine++;
            continue;
        }
        
        // Parser: pid, operation, filename, [numBlocks]
        std::stringstream ss(line);
        std::string pidStr, opStr, fileName, numBlocksStr;
        
        if (!std::getline(ss, pidStr, ',') ||
            !std::getline(ss, opStr, ',') ||
            !std::getline(ss, fileName, ','))
        {
            currentLine++;
            continue;
        }
        
        // Remover espaços
        pidStr.erase(0, pidStr.find_first_not_of(" \t"));
        pidStr.erase(pidStr.find_last_not_of(" \t") + 1);
        opStr.erase(0, opStr.find_first_not_of(" \t"));
        opStr.erase(opStr.find_last_not_of(" \t") + 1);
        fileName.erase(0, fileName.find_first_not_of(" \t"));
        fileName.erase(fileName.find_last_not_of(" \t") + 1);
        
        try
        {
            FileOperation op;
            op.processPID = std::stoi(pidStr);
            op.op = static_cast<FileOperationType>(std::stoi(opStr));
            op.fileName = fileName;
            
            // Para CREATE, ler número de blocos
            if (op.op == CREATE)
            {
                if (std::getline(ss, numBlocksStr, ','))
                {
                    numBlocksStr.erase(0, numBlocksStr.find_first_not_of(" \t"));
                    numBlocksStr.erase(numBlocksStr.find_last_not_of(" \t") + 1);
                    op.numBlocks = std::stoi(numBlocksStr);
                }
            }
            
            operations.push_back(op);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Erro ao fazer parsing de operação: " << line << "\n";
        }
        
        currentLine++;
    }
    
    file.close();
    return true;
}

/*
 * Executa uma operação de arquivo individual
 */
FileOperationResult FileSystemManager::executeOperation(const FileOperation &op)
{
    // Encontrar prioridade do processo
    int priority = 1; // Default: usuário
    for (const auto &proc : processes)
    {
        if (proc.pid == op.processPID)
        {
            priority = proc.priority;
            break;
        }
    }
    
    if (op.op == CREATE)
    {
        return fileSystem->createFile(op.processPID, op.fileName, op.numBlocks, priority);
    }
    else if (op.op == DELETE)
    {
        return fileSystem->deleteFile(op.processPID, op.fileName, priority);
    }
    
    return FileOperationResult(false, "Operação desconhecida.");
}

/*
 * Executa todas as operações de arquivo registradas
 */
bool FileSystemManager::executeAllOperations()
{
    results.clear();
    
    for (const auto &op : operations)
    {
        FileOperationResult result = executeOperation(op);
        results.push_back(result);
    }
    
    return true;
}

/*
 * Imprime os resultados de todas as operações executadas
 */
void FileSystemManager::printResults() const
{
    std::cout << "\nSistema de arquivos =>\n";
    
    for (size_t i = 0; i < results.size(); ++i)
    {
        std::cout << "Operação " << (i + 1) << " => " 
                  << (results[i].success ? "Sucesso" : "Falha") << "\n";
        std::cout << results[i].message << "\n";
    }
}

/*
 * Imprime o mapa final de ocupação do disco
 */
void FileSystemManager::printFinalDiskMap() const
{
    std::cout << "\nMapa de ocupação do disco:\n";
    std::cout << fileSystem->getDiskMapString() << "\n";
}

/*
 * Retorna o vetor de resultados de operações
 */
const std::vector<FileOperationResult> &FileSystemManager::getResults() const
{
    return results;
}

/*
 * Retorna ponteiro constante para o FileSystem
 */
const FileSystem *FileSystemManager::getFileSystem() const
{
    return fileSystem.get();
}
