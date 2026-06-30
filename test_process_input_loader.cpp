#include "Dispatcher/Dispatcher.h"
#include "ProcessInput/ProcessInputLoader.h"

#include <cassert>
#include <iostream>
#include <string>

namespace
{
const std::string INPUT_DIR = "tests/input/";

void assertSuccess(const ProcessInputLoadResult &result)
{
    assert(result.success);
    assert(result.error.kind == ProcessInputErrorKind::None);
}

void assertFailure(const ProcessInputLoadResult &result, ProcessInputErrorKind kind)
{
    assert(!result.success);
    assert(result.entries.empty());
    assert(result.error.kind == kind);
    assert(!result.error.message.empty());
}

void testLoadValidFile()
{
    const ProcessInputLoadResult result =
        ProcessInputLoader::loadFromFile(INPUT_DIR + "processes_valid.txt");

    assertSuccess(result);
    assert(result.entries.size() == 3);

    assert(result.entries[0].startTime == 0);
    assert(result.entries[0].priority == 3);
    assert(result.entries[0].processorTime == 3);
    assert(result.entries[0].memoryBlocks == 64);
    assert(result.entries[0].printerRequest == 0);
    assert(result.entries[0].scannerRequest == 0);
    assert(result.entries[0].modemRequest == 0);
    assert(result.entries[0].sataDiskRequest == 0);
    assert(result.entries[0].inputOrder == 0);

    assert(result.entries[1].startTime == 2);
    assert(result.entries[1].priority == 0);
    assert(result.entries[1].processorTime == 5);
    assert(result.entries[1].memoryBlocks == 64);
    assert(result.entries[1].printerRequest == 1);
    assert(result.entries[1].inputOrder == 1);

    assert(result.entries[2].startTime == 1);
    assert(result.entries[2].priority == 0);
    assert(result.entries[2].processorTime == 1);
    assert(result.entries[2].memoryBlocks == 32);
    assert(result.entries[2].modemRequest == 0);
    assert(result.entries[2].inputOrder == 2);
}

void testLoadCommaSeparatedFileWithSpaces()
{
    const ProcessInputLoadResult result =
        ProcessInputLoader::loadFromFile(INPUT_DIR + "processes_comma_spaces.txt");

    assertSuccess(result);
    assert(result.entries.size() == 1);

    assert(result.entries[0].startTime == 2);
    assert(result.entries[0].priority == 0);
    assert(result.entries[0].processorTime == 3);
    assert(result.entries[0].memoryBlocks == 64);
    assert(result.entries[0].printerRequest == 0);
    assert(result.entries[0].scannerRequest == 0);
    assert(result.entries[0].modemRequest == 0);
    assert(result.entries[0].sataDiskRequest == 0);
    assert(result.entries[0].inputOrder == 0);
}

void testLoadCompactCommaSeparatedFile()
{
    const ProcessInputLoadResult result =
        ProcessInputLoader::loadFromFile(INPUT_DIR + "processes_comma_compact.txt");

    assertSuccess(result);
    assert(result.entries.size() == 1);

    assert(result.entries[0].startTime == 2);
    assert(result.entries[0].priority == 0);
    assert(result.entries[0].processorTime == 3);
    assert(result.entries[0].memoryBlocks == 64);
    assert(result.entries[0].printerRequest == 0);
    assert(result.entries[0].scannerRequest == 0);
    assert(result.entries[0].modemRequest == 0);
    assert(result.entries[0].sataDiskRequest == 0);
    assert(result.entries[0].inputOrder == 0);
}

void testLoaderDoesNotCreateProcessesOrEvents()
{
    const ProcessInputLoadResult result =
        ProcessInputLoader::loadFromFile(INPUT_DIR + "processes_valid.txt");

    assertSuccess(result);

    Dispatcher untouchedDispatcher;
    assert(untouchedDispatcher.getScheduler().processCount() == 0);
    assert(untouchedDispatcher.getEvents().empty());
}

void testNonexistentFileError()
{
    const ProcessInputLoadResult result =
        ProcessInputLoader::loadFromFile(INPUT_DIR + "does_not_exist.txt");

    assertFailure(result, ProcessInputErrorKind::FileOpenError);
    assert(result.error.lineNumber == 0);
}

void testReadFailureError()
{
    const ProcessInputLoadResult result = ProcessInputLoader::loadFromFile("tests/input");

    assertFailure(result, ProcessInputErrorKind::FileReadError);
}

void testInvalidColumnCountErrors()
{
    ProcessInputLoadResult result =
        ProcessInputLoader::loadFromFile(INPUT_DIR + "processes_missing_column.txt");

    assertFailure(result, ProcessInputErrorKind::InvalidColumnCount);
    assert(result.error.lineNumber == 1);

    result = ProcessInputLoader::loadFromFile(INPUT_DIR + "processes_extra_column.txt");

    assertFailure(result, ProcessInputErrorKind::InvalidColumnCount);
    assert(result.error.lineNumber == 1);
}

void testInvalidNumberError()
{
    const ProcessInputLoadResult result =
        ProcessInputLoader::loadFromFile(INPUT_DIR + "processes_invalid_number.txt");

    assertFailure(result, ProcessInputErrorKind::InvalidNumber);
    assert(result.error.lineNumber == 1);
}

void testInvalidCommaFields()
{
    ProcessInputLoadResult result =
        ProcessInputLoader::loadFromFile(INPUT_DIR + "processes_comma_empty_field.txt");

    assertFailure(result, ProcessInputErrorKind::InvalidNumber);
    assert(result.error.lineNumber == 1);

    result = ProcessInputLoader::loadFromFile(INPUT_DIR + "processes_only_commas.txt");

    assertFailure(result, ProcessInputErrorKind::InvalidNumber);
    assert(result.error.lineNumber == 1);

    result = ProcessInputLoader::loadFromFile(INPUT_DIR + "processes_comma_empty_spaces.txt");

    assertFailure(result, ProcessInputErrorKind::InvalidNumber);
    assert(result.error.lineNumber == 1);
}

void testBlankLinesAndExtraSpaces()
{
    const ProcessInputLoadResult result =
        ProcessInputLoader::loadFromFile(INPUT_DIR + "processes_blank_lines.txt");

    assertSuccess(result);
    assert(result.entries.size() == 2);

    assert(result.entries[0].startTime == 0);
    assert(result.entries[0].priority == 1);
    assert(result.entries[0].processorTime == 2);
    assert(result.entries[0].inputOrder == 0);

    assert(result.entries[1].startTime == 3);
    assert(result.entries[1].priority == 2);
    assert(result.entries[1].scannerRequest == 1);
    assert(result.entries[1].inputOrder == 1);
}

void testPreservesOrderWithRepeatedAndUnsortedStartTimes()
{
    const ProcessInputLoadResult result =
        ProcessInputLoader::loadFromFile(INPUT_DIR + "processes_same_start_time.txt");

    assertSuccess(result);
    assert(result.entries.size() == 4);

    assert(result.entries[0].startTime == 5);
    assert(result.entries[0].priority == 3);
    assert(result.entries[0].inputOrder == 0);

    assert(result.entries[1].startTime == 2);
    assert(result.entries[1].priority == 0);
    assert(result.entries[1].inputOrder == 1);

    assert(result.entries[2].startTime == 2);
    assert(result.entries[2].priority == 1);
    assert(result.entries[2].inputOrder == 2);

    assert(result.entries[3].startTime == 7);
    assert(result.entries[3].priority == 2);
    assert(result.entries[3].inputOrder == 3);
}

void testDispatcherCompatibility()
{
    const ProcessInputLoadResult result =
        ProcessInputLoader::loadFromFile(INPUT_DIR + "processes_valid.txt");

    assertSuccess(result);

    Dispatcher dispatcher(result.entries);
    assert(dispatcher.pendingCount() == result.entries.size());
    assert(dispatcher.getScheduler().processCount() == 0);
    assert(dispatcher.getEvents().empty());
}
}

int main()
{
    std::cout << "===== Process Input Loader Tests =====\n";

    testLoadValidFile();
    testLoadCommaSeparatedFileWithSpaces();
    testLoadCompactCommaSeparatedFile();
    testLoaderDoesNotCreateProcessesOrEvents();
    testNonexistentFileError();
    testReadFailureError();
    testInvalidColumnCountErrors();
    testInvalidNumberError();
    testInvalidCommaFields();
    testBlankLinesAndExtraSpaces();
    testPreservesOrderWithRepeatedAndUnsortedStartTimes();
    testDispatcherCompatibility();

    std::cout << "All process input loader tests passed.\n";
    return 0;
}
