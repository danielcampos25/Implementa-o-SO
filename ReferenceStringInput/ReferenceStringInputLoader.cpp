#include "ReferenceStringInputLoader.h"

#include <cerrno>
#include <cstdlib>
#include <fstream>
#include <limits>
#include <string>
#include <vector>

namespace
{
ReferenceStringInputLoadResult successResult(const std::vector<std::vector<int>> &referenceStrings)
{
    return {true, referenceStrings, {ReferenceStringInputErrorKind::None, 0, ""}};
}

ReferenceStringInputLoadResult errorResult(ReferenceStringInputErrorKind kind,
                                           int lineNumber,
                                           const std::string &message)
{
    return {false, {}, {kind, lineNumber, message}};
}

std::string trimSpaces(const std::string &text)
{
    const std::size_t first = text.find_first_not_of(" \t\r\n\f\v");
    if (first == std::string::npos)
    {
        return "";
    }

    const std::size_t last = text.find_last_not_of(" \t\r\n\f\v");
    return text.substr(first, last - first + 1);
}

std::vector<std::string> splitCommaFields(const std::string &line)
{
    std::vector<std::string> fields;
    std::size_t start = 0;

    while (start <= line.size())
    {
        const std::size_t delimiter = line.find(',', start);
        if (delimiter == std::string::npos)
        {
            fields.push_back(trimSpaces(line.substr(start)));
            break;
        }

        fields.push_back(trimSpaces(line.substr(start, delimiter - start)));
        start = delimiter + 1;
    }

    return fields;
}

bool parseInteger(const std::string &text, int &value)
{
    char *end = nullptr;
    errno = 0;
    const long parsed = std::strtol(text.c_str(), &end, 10);

    if (errno != 0 ||
        end == text.c_str() ||
        *end != '\0' ||
        parsed < std::numeric_limits<int>::min() ||
        parsed > std::numeric_limits<int>::max())
    {
        return false;
    }

    value = static_cast<int>(parsed);
    return true;
}
}

ReferenceStringInputLoadResult ReferenceStringInputLoader::loadFromFile(const std::string &path)
{
    std::ifstream input(path);
    if (!input.is_open())
    {
        return errorResult(ReferenceStringInputErrorKind::FileOpenError,
                           0,
                           "Nao foi possivel abrir o arquivo de strings de referencia: " + path);
    }

    std::vector<std::vector<int>> referenceStrings;
    std::string line;
    int lineNumber = 0;

    while (std::getline(input, line))
    {
        ++lineNumber;
        const std::string trimmedLine = trimSpaces(line);

        if (trimmedLine.empty())
        {
            continue;
        }

        const std::vector<std::string> fields = splitCommaFields(trimmedLine);
        std::vector<int> references;
        references.reserve(fields.size());

        for (const std::string &field : fields)
        {
            if (field.empty())
            {
                return errorResult(ReferenceStringInputErrorKind::EmptyField,
                                   lineNumber,
                                   "Campo vazio na string de referencia");
            }

            int value = 0;
            if (!parseInteger(field, value))
            {
                return errorResult(ReferenceStringInputErrorKind::InvalidNumber,
                                   lineNumber,
                                   "Referencia de pagina invalida");
            }

            if (value < 0)
            {
                return errorResult(ReferenceStringInputErrorKind::NegativeNumber,
                                   lineNumber,
                                   "Referencia de pagina negativa");
            }

            references.push_back(value);
        }

        if (references.empty())
        {
            return errorResult(ReferenceStringInputErrorKind::EmptyField,
                               lineNumber,
                               "Linha sem referencias de pagina");
        }

        referenceStrings.push_back(references);
    }

    if (input.bad())
    {
        return errorResult(ReferenceStringInputErrorKind::FileReadError,
                           lineNumber > 0 ? lineNumber : 0,
                           "Falha durante a leitura do arquivo de strings de referencia");
    }

    if (referenceStrings.empty())
    {
        return errorResult(ReferenceStringInputErrorKind::EmptyInput,
                           0,
                           "Arquivo de strings de referencia vazio");
    }

    return successResult(referenceStrings);
}
