#include "Dispatcher/Dispatcher.h"
#include "ProcessInput/ProcessInputLoader.h"

#include <iostream>

namespace
{
constexpr int SUCCESS_EXIT_CODE = 0;
constexpr int ERROR_EXIT_CODE = 1;

void printUsage(const char *programName)
{
    std::cerr << "Uso: " << programName << " <processes.txt>\n";
}

void printLoadError(const ProcessInputError &error)
{
    std::cerr << "Erro ao carregar arquivo de processos: " << error.message;

    if (error.lineNumber > 0)
    {
        std::cerr << " (linha " << error.lineNumber << ')';
    }

    std::cerr << '\n';
}
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printUsage(argv[0]);
        return ERROR_EXIT_CODE;
    }

    const ProcessInputLoadResult loadResult = ProcessInputLoader::loadFromFile(argv[1]);
    if (!loadResult.success)
    {
        printLoadError(loadResult.error);
        return ERROR_EXIT_CODE;
    }

    Dispatcher dispatcher(loadResult.entries);
    dispatcher.runUntilComplete();
    dispatcher.printEvents(std::cout);

    return SUCCESS_EXIT_CODE;
}
