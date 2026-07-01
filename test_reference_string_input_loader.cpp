#include "ReferenceStringInput/ReferenceStringInputLoader.h"

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

namespace
{
const std::string INPUT_DIR = "tests/input/";

void assertSuccessWithReferences(const std::string &path,
                                 const std::vector<std::vector<int>> &expected)
{
    const ReferenceStringInputLoadResult result = ReferenceStringInputLoader::loadFromFile(INPUT_DIR + path);

    assert(result.success);
    assert(result.error.kind == ReferenceStringInputErrorKind::None);
    assert(result.referenceStrings == expected);
}

void assertFailureWithKind(const std::string &path, ReferenceStringInputErrorKind expectedKind)
{
    const ReferenceStringInputLoadResult result = ReferenceStringInputLoader::loadFromFile(INPUT_DIR + path);

    assert(!result.success);
    assert(result.referenceStrings.empty());
    assert(result.error.kind == expectedKind);
}

void testValidSingleLine()
{
    assertSuccessWithReferences("string_valid_single.txt",
                                {{1, 2, 3, 4, 1, 2, 5}});
}

void testValidLineWithSpaces()
{
    assertSuccessWithReferences("string_valid_spaces.txt",
                                {{1, 2, 3, 4}});
}

void testMultipleLinesPreserveOrder()
{
    assertSuccessWithReferences("string_valid_multiple.txt",
                                {{1, 2, 3}, {7, 0, 1, 2}});
}

void testBlankLinesAreIgnored()
{
    assertSuccessWithReferences("string_blank_lines.txt",
                                {{1, 2, 3}, {7, 0, 1, 2}});
}

void testEmptyFileRejected()
{
    assertFailureWithKind("string_empty.txt", ReferenceStringInputErrorKind::EmptyInput);
}

void testBlankOnlyFileRejected()
{
    assertFailureWithKind("string_blank_only.txt", ReferenceStringInputErrorKind::EmptyInput);
}

void testEmptyFieldRejected()
{
    assertFailureWithKind("string_empty_field.txt", ReferenceStringInputErrorKind::EmptyField);
}

void testOnlyCommasRejected()
{
    assertFailureWithKind("string_only_commas.txt", ReferenceStringInputErrorKind::EmptyField);
}

void testNonNumericValueRejected()
{
    assertFailureWithKind("string_non_numeric.txt", ReferenceStringInputErrorKind::InvalidNumber);
}

void testNegativeValueRejected()
{
    assertFailureWithKind("string_negative.txt", ReferenceStringInputErrorKind::NegativeNumber);
}
}

int main()
{
    testValidSingleLine();
    testValidLineWithSpaces();
    testMultipleLinesPreserveOrder();
    testBlankLinesAreIgnored();
    testEmptyFileRejected();
    testBlankOnlyFileRejected();
    testEmptyFieldRejected();
    testOnlyCommasRejected();
    testNonNumericValueRejected();
    testNegativeValueRejected();

    std::cout << "ReferenceStringInputLoader tests passed\n";
    return 0;
}
