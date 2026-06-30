#ifndef REFERENCE_STRING_INPUT_LOADER_H
#define REFERENCE_STRING_INPUT_LOADER_H

#include <string>
#include <vector>

enum class ReferenceStringInputErrorKind
{
    None,
    FileOpenError,
    FileReadError,
    EmptyInput,
    EmptyField,
    InvalidNumber,
    NegativeNumber
};

struct ReferenceStringInputError
{
    ReferenceStringInputErrorKind kind;
    int lineNumber;
    std::string message;
};

struct ReferenceStringInputLoadResult
{
    bool success;
    std::vector<std::vector<int>> referenceStrings;
    ReferenceStringInputError error;
};

class ReferenceStringInputLoader
{
public:
    static ReferenceStringInputLoadResult loadFromFile(const std::string &path);
};

#endif
