#include "ProcessInputLoader.h"

#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace
{
constexpr int EXPECTED_COLUMN_COUNT = 8;

ProcessInputLoadResult successResult(const std::vector<ProcessWorkloadEntry> &entries)
{
    return {true, entries, {ProcessInputErrorKind::None, 0, ""}};
}

ProcessInputLoadResult errorResult(ProcessInputErrorKind kind,
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

std::vector<std::string> splitCommaColumns(const std::string &line)
{
    std::vector<std::string> columns;
    std::size_t start = 0;

    while (start <= line.size())
    {
        const std::size_t delimiter = line.find(',', start);
        if (delimiter == std::string::npos)
        {
            columns.push_back(trimSpaces(line.substr(start)));
            break;
        }

        columns.push_back(trimSpaces(line.substr(start, delimiter - start)));
        start = delimiter + 1;
    }

    return columns;
}

std::vector<std::string> splitColumns(const std::string &line)
{
    if (line.find(',') != std::string::npos)
    {
        return splitCommaColumns(line);
    }

    std::istringstream stream(line);
    std::vector<std::string> columns;
    std::string column;

    while (stream >> column)
    {
        columns.push_back(column);
    }

    return columns;
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

ProcessInputLoadResult ProcessInputLoader::loadFromFile(const std::string &path)
{
    std::ifstream input(path);
    if (!input.is_open())
    {
        return errorResult(ProcessInputErrorKind::FileOpenError,
                           0,
                           "Nao foi possivel abrir o arquivo de processos: " + path);
    }

    std::vector<ProcessWorkloadEntry> entries;
    std::string line;
    int lineNumber = 0;

    while (std::getline(input, line))
    {
        ++lineNumber;
        const std::vector<std::string> columns = splitColumns(line);

        if (columns.empty())
        {
            continue;
        }

        if (columns.size() != EXPECTED_COLUMN_COUNT)
        {
            return errorResult(ProcessInputErrorKind::InvalidColumnCount,
                               lineNumber,
                               "Linha com quantidade invalida de colunas");
        }

        int values[EXPECTED_COLUMN_COUNT] = {};
        for (int index = 0; index < EXPECTED_COLUMN_COUNT; ++index)
        {
            if (!parseInteger(columns[static_cast<std::size_t>(index)], values[index]))
            {
                return errorResult(ProcessInputErrorKind::InvalidNumber,
                                   lineNumber,
                                   "Campo numerico invalido");
            }
        }

        ProcessWorkloadEntry entry = {values[0],
                                      values[1],
                                      values[2],
                                      values[3],
                                      values[4],
                                      values[5],
                                      values[6],
                                      values[7],
                                      static_cast<int>(entries.size())};
        entries.push_back(entry);
    }

    if (input.bad())
    {
        return errorResult(ProcessInputErrorKind::FileReadError,
                           lineNumber > 0 ? lineNumber : 0,
                           "Falha durante a leitura do arquivo de processos");
    }

    return successResult(entries);
}
