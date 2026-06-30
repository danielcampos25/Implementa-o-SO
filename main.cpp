#include "Dispatcher/Dispatcher.h"
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
    (void)fileSystemInputPath;

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

    Dispatcher dispatcher(processLoadResult.entries);
    dispatcher.runUntilComplete();
    dispatcher.printEvents(std::cout);

    return SUCCESS_EXIT_CODE;
}
