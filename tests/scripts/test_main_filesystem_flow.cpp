#include <cassert>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

namespace
{
std::string readFile(const std::string &path)
{
    std::ifstream input(path);
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

int runProgram(const std::string &arguments, const std::string &outputPath)
{
    const std::string command = "./bin/g2_OS " + arguments + " > " + outputPath + " 2>&1";
    return std::system(command.c_str());
}

void assertContains(const std::string &text, const std::string &needle)
{
    assert(text.find(needle) != std::string::npos);
}

void assertNotContains(const std::string &text, const std::string &needle)
{
    assert(text.find(needle) == std::string::npos);
}

void assertAppearsBefore(const std::string &text, const std::string &first, const std::string &second)
{
    const std::size_t firstPosition = text.find(first);
    const std::size_t secondPosition = text.find(second);

    assert(firstPosition != std::string::npos);
    assert(secondPosition != std::string::npos);
    assert(firstPosition < secondPosition);
}

void testValidFileSystemInputIsProcessed()
{
    const std::string outputPath = "/tmp/g2_os_main_flow_valid.txt";
    const int exitCode = runProgram("tests/input/processes_valid.txt tests/objects/files_example1.txt tests/input/string_valid_for_processes.txt",
                                    outputPath);
    const std::string output = readFile(outputPath);

    assert(exitCode == 0);
    assertContains(output, "Sistema de arquivos =>");
    assertContains(output, "Operação 1 =>");
    assertContains(output, "Arquivo 'B' criado com sucesso");
    assertContains(output, "Mapa de ocupação do disco:");
}

void testInvalidFileSystemInputFailsClearly()
{
    const std::string outputPath = "/tmp/g2_os_main_flow_invalid_files.txt";
    const int exitCode = runProgram("tests/input/processes_valid.txt tests/objects/files_invalid_main_flow.txt tests/input/string_valid_for_processes.txt",
                                    outputPath);
    const std::string output = readFile(outputPath);

    assert(exitCode != 0);
    assertContains(output, "Erro ao carregar arquivo do sistema de arquivos");
    assertNotContains(output, "Sistema de arquivos =>");
    assertNotContains(output, "Mapa de ocupação do disco:");
}

void testFileSystemIsSkippedWhenDispatcherReportsSimulationError()
{
    const std::string outputPath = "/tmp/g2_os_main_flow_simulation_error.txt";
    const int exitCode = runProgram("tests/input/processes_valid.txt tests/objects/files_example1.txt tests/input/string_insufficient_main_flow.txt",
                                    outputPath);
    const std::string output = readFile(outputPath);

    assert(exitCode == 0);
    assertContains(output, "type=dispatch");
    assertNotContains(output, "type=completion");
    assertNotContains(output, "Número de Faltas de Páginas por processo:");
    assertNotContains(output, "Sistema de arquivos =>");
    assertNotContains(output, "Mapa de ocupação do disco:");
}

void testOutputOrder()
{
    const std::string outputPath = "/tmp/g2_os_main_flow_order.txt";
    const int exitCode = runProgram("tests/input/processes_valid.txt tests/objects/files_example1.txt tests/input/string_valid_for_processes.txt",
                                    outputPath);
    const std::string output = readFile(outputPath);

    assert(exitCode == 0);
    assertAppearsBefore(output, "type=completion", "Número de Faltas de Páginas por processo:");
    assertAppearsBefore(output, "Número de Faltas de Páginas por processo:", "Sistema de arquivos =>");
    assertAppearsBefore(output, "Sistema de arquivos =>", "Mapa de ocupação do disco:");
}

void testLoadedProcessPrioritiesAreUsed()
{
    const std::string outputPath = "/tmp/g2_os_main_flow_permissions.txt";
    const int exitCode = runProgram("tests/input/processes_filesystem_permissions.txt tests/objects/files_permissions_main_flow.txt tests/input/string_filesystem_permissions.txt",
                                    outputPath);
    const std::string output = readFile(outputPath);

    assert(exitCode == 0);
    assertContains(output, "Arquivo 'A' deletado com sucesso.");
    assertContains(output, "Processo 2 não tem permissão para deletar arquivo 'B'.");
    assertContains(output, "Processo 9 não existe.");
}
}

int main()
{
    testValidFileSystemInputIsProcessed();
    testInvalidFileSystemInputFailsClearly();
    testFileSystemIsSkippedWhenDispatcherReportsSimulationError();
    testOutputOrder();
    testLoadedProcessPrioritiesAreUsed();

    return 0;
}
