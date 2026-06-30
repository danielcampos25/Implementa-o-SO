#include "FileSystem.h"
#include <iostream>
#include <sstream>
#include <algorithm>

/*
 * Construtor: inicializa o sistema de arquivos com um número total de blocos.
 * O disco inicia vazio (todos os blocos marcados como 0).
 */
FileSystem::FileSystem(int totalBlocks)
    : totalBlocks(totalBlocks)
{
    diskMap.assign(totalBlocks, 0);
}

/*
 * Verifica se um PID corresponde a um processo de tempo real.
 * Por convenção, assumimos que processos de tempo real têm prioridade 0.
 * Este método pode ser adaptado conforme a integração com o ProcessScheduler.
 */
bool FileSystem::isRealTimeProcess(int processPID)
{
    // TODO: Integrar com o ProcessScheduler para obter a prioridade real do processo
    // Por enquanto, retorna false para todos (será ajustado na integração)
    return false;
}

/*
 * Encontra espaço contíguo de numBlocks blocos livres usando first-fit.
 * Retorna o índice do primeiro bloco encontrado, ou -1 se não houver espaço.
 */
int FileSystem::findContiguousSpace(int numBlocks)
{
    if (numBlocks <= 0 || numBlocks > totalBlocks)
        return -1;
    
    int consecutiveBlocks = 0;
    int startBlock = -1;
    
    for (int i = 0; i < totalBlocks; ++i)
    {
        if (diskMap[i] == 0) // Bloco livre
        {
            if (consecutiveBlocks == 0)
                startBlock = i;
            ++consecutiveBlocks;
            
            if (consecutiveBlocks == numBlocks)
                return startBlock;
        }
        else
        {
            consecutiveBlocks = 0;
            startBlock = -1;
        }
    }
    
    return -1;
}

/*
 * Encontra um arquivo pelo nome na lista de arquivos.
 * Retorna o índice do arquivo na lista, ou -1 se não encontrado.
 */
int FileSystem::findFileByName(const std::string &name)
{
    for (size_t i = 0; i < files.size(); ++i)
    {
        if (files[i].name == name)
            return i;
    }
    return -1;
}

/*
 * Aloca um arquivo no disco, marcando os blocos como ocupados.
 * Retorna uma string com os blocos alocados ou mensagem de erro.
 */
std::string FileSystem::allocateFileOnDisk(const std::string &fileName, int numBlocks, int ownerPID)
{
    int startBlock = findContiguousSpace(numBlocks);
    
    if (startBlock == -1)
        return "";
    
    // Marcar blocos como ocupados
    int fileIndex = files.size();
    for (int i = startBlock; i < startBlock + numBlocks; ++i)
    {
        diskMap[i] = fileIndex + 1; // +1 para evitar confundir com blocos livres (0)
    }
    
    // Criar entrada de arquivo
    File newFile(fileName, startBlock, numBlocks, ownerPID);
    files.push_back(newFile);
    
    // Montar string com blocos alocados
    std::stringstream ss;
    for (int i = startBlock; i < startBlock + numBlocks; ++i)
    {
        if (i > startBlock) ss << ", ";
        ss << i;
    }
    
    return ss.str();
}

/*
 * Cria um arquivo no disco.
 * Restrições:
 * - Processos de tempo real podem criar arquivos sem restrição
 * - Processos de usuário podem criar quantos arquivos desejarem
 */
FileOperationResult FileSystem::createFile(int processPID, const std::string &fileName, int numBlocks, int priority)
{
    std::lock_guard<std::recursive_mutex> lock(mtx);
    
    // Validações básicas
    if (numBlocks <= 0)
        return FileOperationResult(false, "Número de blocos deve ser maior que zero.");
    
    if (numBlocks > getAvailableSpace())
        return FileOperationResult(false, "Falta de espaço disponível no disco.");
    
    // Verificar se arquivo com mesmo nome já existe
    if (findFileByName(fileName) != -1)
        return FileOperationResult(false, "Arquivo '" + fileName + "' já existe.");
    
    // Tentar alocar espaço contíguo
    std::string blocksStr = allocateFileOnDisk(fileName, numBlocks, processPID);
    
    if (blocksStr.empty())
        return FileOperationResult(false, "Falha ao alocar espaço contíguo no disco.");
    
    return FileOperationResult(true, "Arquivo '" + fileName + "' criado com sucesso nos blocos [" + blocksStr + "].");
}

/*
 * Deleta um arquivo do disco.
 * Restrições:
 * - Processos de tempo real podem deletar qualquer arquivo
 * - Processos de usuário podem deletar apenas arquivos que criaram
 */
FileOperationResult FileSystem::deleteFile(int processPID, const std::string &fileName, int priority)
{
    std::lock_guard<std::recursive_mutex> lock(mtx);
    
    // Encontrar arquivo
    int fileIndex = findFileByName(fileName);
    if (fileIndex == -1)
        return FileOperationResult(false, "Arquivo '" + fileName + "' não existe.");
    
    File &fileToDelete = files[fileIndex];
    
    // Verificar permissões
    bool isRealTime = (priority == 0); // Assumindo que prioridade 0 é tempo real
    if (!isRealTime && fileToDelete.ownerPID != processPID)
        return FileOperationResult(false, "Processo " + std::to_string(processPID) + 
                                   " não tem permissão para deletar arquivo '" + fileName + "'.");
    
    // Liberar blocos no disco
    for (int i = fileToDelete.startBlock; i < fileToDelete.startBlock + fileToDelete.numBlocks; ++i)
    {
        diskMap[i] = 0;
    }
    
    // Remover arquivo da lista
    files.erase(files.begin() + fileIndex);
    
    // Atualizar índices no diskMap
    for (int i = 0; i < totalBlocks; ++i)
    {
        if (diskMap[i] > 0)
        {
            if (diskMap[i] > fileIndex + 1)
                diskMap[i]--;
        }
    }
    
    return FileOperationResult(true, "Arquivo '" + fileName + "' deletado com sucesso.");
}

/*
 * Carrega o estado inicial do disco a partir de uma string de descrição.
 * Formato esperado: "arquivo1:startBlock1:numBlocks1|arquivo2:startBlock2:numBlocks2|..."
 * Exemplo: "X:0:2|Y:3:1|Z:5:3"
 */
bool FileSystem::loadDiskState(const std::string &diskStateString)
{
    std::lock_guard<std::recursive_mutex> lock(mtx);
    
    // Limpar estado anterior
    files.clear();
    diskMap.assign(totalBlocks, 0);
    
    if (diskStateString.empty())
        return true;
    
    // Fazer parsing da string
    std::stringstream ss(diskStateString);
    std::string fileSpec;
    
    while (std::getline(ss, fileSpec, '|'))
    {
        std::stringstream fileStream(fileSpec);
        std::string name, startStr, numStr;
        
        if (!std::getline(fileStream, name, ':') ||
            !std::getline(fileStream, startStr, ':') ||
            !std::getline(fileStream, numStr, ':'))
            continue;
        
        try
        {
            int startBlock = std::stoi(startStr);
            int numBlocks = std::stoi(numStr);
            
            // Validar
            if (startBlock < 0 || numBlocks <= 0 || startBlock + numBlocks > totalBlocks)
                continue;
            
            // Marcar blocos no mapa
            int fileIndex = files.size();
            for (int i = startBlock; i < startBlock + numBlocks; ++i)
            {
                diskMap[i] = fileIndex + 1;
            }
            
            // Adicionar arquivo (ownerPID = -1 para arquivos pré-existentes)
            File newFile(name, startBlock, numBlocks, -1);
            files.push_back(newFile);
        }
        catch (...)
        {
            continue;
        }
    }
    
    return true;
}

/*
 * Retorna o número de blocos disponíveis (livres) no disco.
 */
int FileSystem::getAvailableSpace() const
{
    std::lock_guard<std::recursive_mutex> lock(mtx);
    
    int count = 0;
    for (int block : diskMap)
    {
        if (block == 0)
            count++;
    }
    return count;
}

/*
 * Retorna o número total de blocos do disco.
 */
int FileSystem::getTotalSpace() const
{
    return totalBlocks;
}

/*
 * Imprime o mapa do disco mostrando quais blocos estão ocupados e por qual arquivo.
 */
void FileSystem::printDiskMap() const
{
    std::lock_guard<std::recursive_mutex> lock(mtx);
    
    std::cout << "Mapa de ocupação do disco:\n";
    for (int i = 0; i < totalBlocks; ++i)
    {
        if (diskMap[i] == 0)
        {
            std::cout << "0";
        }
        else
        {
            int fileIndex = diskMap[i] - 1;
            if (fileIndex >= 0 && fileIndex < (int)files.size())
            {
                std::cout << files[fileIndex].name;
            }
            else
            {
                std::cout << "?";
            }
        }
        
        if ((i + 1) % 10 == 0)
            std::cout << "\n";
        else
            std::cout << " ";
    }
    if (totalBlocks % 10 != 0)
        std::cout << "\n";
}

/*
 * Imprime a lista de arquivos com suas informações (nome, startBlock, numBlocks, ownerPID).
 */
void FileSystem::printFileList() const
{
    std::lock_guard<std::recursive_mutex> lock(mtx);
    
    std::cout << "\nArquivos no disco:\n";
    for (const File &file : files)
    {
        std::cout << "  " << file.name << ": blocos [" << file.startBlock 
                  << " a " << (file.startBlock + file.numBlocks - 1) 
                  << "], owner PID: " << file.ownerPID << "\n";
    }
    if (files.empty())
        std::cout << "  (nenhum arquivo)\n";
}

/*
 * Retorna uma string representando o mapa do disco.
 * Usado para exibição compacta.
 */
std::string FileSystem::getDiskMapString() const
{
    std::lock_guard<std::recursive_mutex> lock(mtx);
    
    std::stringstream ss;
    for (int i = 0; i < totalBlocks; ++i)
    {
        if (diskMap[i] == 0)
        {
            ss << "0";
        }
        else
        {
            int fileIndex = diskMap[i] - 1;
            if (fileIndex >= 0 && fileIndex < (int)files.size())
            {
                ss << files[fileIndex].name;
            }
            else
            {
                ss << "?";
            }
        }
        
        if (i < totalBlocks - 1)
            ss << " ";
    }
    
    return ss.str();
}
