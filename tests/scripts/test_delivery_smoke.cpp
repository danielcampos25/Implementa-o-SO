#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/wait.h>

namespace
{
std::string readFile(const std::string &path)
{
    std::ifstream input(path);
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

int runCommand(const std::string &command)
{
    return std::system(command.c_str());
}

void assertContains(const std::string &text, const std::string &needle)
{
    assert(text.find(needle) != std::string::npos);
}
}

int main()
{
    const std::string outputPath = "/tmp/delivery_smoke_output.txt";
    const std::string command =
        "./bin/g2_OS tests/input/processes_delivery.txt tests/objects/files_delivery.txt tests/input/string_delivery.txt > " +
        outputPath + " 2>&1";

    const int status = runCommand(command);
    assert(status != -1);
    assert(WIFEXITED(status));
    assert(WEXITSTATUS(status) == 0);

    const std::string output = readFile(outputPath);
    std::cout << output;

    assertContains(output, "type=completion");
    assertContains(output, "Número de Faltas de Páginas por processo:");
    assertContains(output, "Sistema de arquivos =>");
    assertContains(output, "Mapa de ocupação do disco:");

    return 0;
}
