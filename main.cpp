#include "Dispatcher/Dispatcher.h"
#include "FileSystem/FileSystemManager.h"
#include "ProcessInput/ProcessInputLoader.h"
#include "ReferenceStringInput/ReferenceStringInputLoader.h"

#include <iostream>

namespace
{
constexpr int SUCCESS_EXIT_CODE = 0;
constexpr int ERROR_EXIT_CODE = 1;

void printUsage()
{
    std::cerr << "Uso: ./dispatcher <processes.txt> <files.txt> <string.txt>\n";
}

void printProcessLoadError(const ProcessInputError &error)
{
    std::cerr << "Erro ao carregar arquivo de processos: " << error.message;

    if (error.lineNumber > 0)
    {
        std::cerr << " (linha " << error.lineNumber << ')';
    }

    std::cerr << '\n';
}

void printReferenceStringLoadError(const ReferenceStringInputError &error)
{
    std::cerr << "Erro ao carregar arquivo de strings de referencia: " << error.message;

    if (error.lineNumber > 0)
    {
        std::cerr << " (linha " << error.lineNumber << ')';
    }

    std::cerr << '\n';
}

void printReferenceStringCountMismatch(std::size_t processCount, std::size_t referenceStringCount)
{
    std::cerr << "Erro: quantidade de strings de referencia ("
              << referenceStringCount
              << ") diferente da quantidade de processos ("
              << processCount
              << ")\n";
}

void printFileSystemLoadError(const char *path)
{
    std::cerr << "Erro ao carregar arquivo do sistema de arquivos: " << path << '\n';
}
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printUsage();
        return ERROR_EXIT_CODE;
    }

    const char *processInputPath = argv[1];
    const char *fileSystemInputPath = argv[2];
    const char *referenceStringInputPath = argv[3];

    const ProcessInputLoadResult processLoadResult = ProcessInputLoader::loadFromFile(processInputPath);
    if (!processLoadResult.success)
    {
        printProcessLoadError(processLoadResult.error);
        return ERROR_EXIT_CODE;
    }

    const ReferenceStringInputLoadResult referenceStringLoadResult =
        ReferenceStringInputLoader::loadFromFile(referenceStringInputPath);
    if (!referenceStringLoadResult.success)
    {
        printReferenceStringLoadError(referenceStringLoadResult.error);
        return ERROR_EXIT_CODE;
    }

    if (referenceStringLoadResult.referenceStrings.size() != processLoadResult.entries.size())
    {
        printReferenceStringCountMismatch(processLoadResult.entries.size(),
                                          referenceStringLoadResult.referenceStrings.size());
        return ERROR_EXIT_CODE;
    }

    FileSystemManager fileSystemManager;
    if (!fileSystemManager.loadDiskConfiguration(fileSystemInputPath))
    {
        printFileSystemLoadError(fileSystemInputPath);
        return ERROR_EXIT_CODE;
    }

    for (const ProcessWorkloadEntry &entry : processLoadResult.entries)
    {
        fileSystemManager.registerProcess(entry.inputOrder, entry.priority);
    }

    Dispatcher dispatcher(processLoadResult.entries, referenceStringLoadResult.referenceStrings);
    dispatcher.runUntilComplete();
    dispatcher.printEvents(std::cout);
    if (!dispatcher.hasSimulationError())
    {
        dispatcher.printPageFaultSummary(std::cout);
        dispatcher.printMemoryTables();
        fileSystemManager.executeAllOperations();
        fileSystemManager.printResults();
        fileSystemManager.printFinalDiskMap();
    }

    return SUCCESS_EXIT_CODE;
}
