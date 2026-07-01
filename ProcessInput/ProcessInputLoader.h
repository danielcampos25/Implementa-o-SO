#ifndef PROCESS_INPUT_LOADER_H
#define PROCESS_INPUT_LOADER_H

#include "../Dispatcher/Dispatcher.h"

#include <string>
#include <vector>

enum class ProcessInputErrorKind
{
    None,
    FileOpenError,
    FileReadError,
    InvalidColumnCount,
    InvalidNumber
};

struct ProcessInputError
{
    ProcessInputErrorKind kind;
    int lineNumber;
    std::string message;
};

struct ProcessInputLoadResult
{
    bool success;
    std::vector<ProcessWorkloadEntry> entries;
    ProcessInputError error;
};

class ProcessInputLoader
{
public:
    static ProcessInputLoadResult loadFromFile(const std::string &path);
};

#endif
