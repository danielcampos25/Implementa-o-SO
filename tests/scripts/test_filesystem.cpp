#include "./../../FileSystem/FileSystem.h"
#include "./../../FileSystem/FileSystemManager.h"
#include <iostream>
#include <cassert>

/*
 * Test 1: Basic file creation and deletion
 */
void testBasicOperations()
{
    std::cout << "\n===== TEST 1: Basic File Operations =====\n";
    
    FileSystem fs(10);
    
    // Test file creation
    auto result1 = fs.createFile(0, "A", 3, 1);
    std::cout << "Create A (3 blocks): " << (result1.success ? "SUCCESS" : "FAILED") << "\n";
    std::cout << "  Message: " << result1.message << "\n";
    
    auto result2 = fs.createFile(1, "B", 2, 1);
    std::cout << "Create B (2 blocks): " << (result2.success ? "SUCCESS" : "FAILED") << "\n";
    std::cout << "  Message: " << result2.message << "\n";
    
    std::cout << "Available space: " << fs.getAvailableSpace() << "/" << fs.getTotalSpace() << " blocks\n";
    std::cout << "Disk map: " << fs.getDiskMapString() << "\n";
    
    // Test deletion
    auto result3 = fs.deleteFile(0, "A", 1);
    std::cout << "Delete A: " << (result3.success ? "SUCCESS" : "FAILED") << "\n";
    std::cout << "  Message: " << result3.message << "\n";
    
    std::cout << "Disk map after delete: " << fs.getDiskMapString() << "\n";
    std::cout << "Available space: " << fs.getAvailableSpace() << "/" << fs.getTotalSpace() << " blocks\n";
}

/*
 * Test 2: Permission tests
 */
void testPermissions()
{
    std::cout << "\n===== TEST 2: Permission Tests =====\n";
    
    FileSystem fs(20);
    
    // Process 0 creates a file
    auto result1 = fs.createFile(0, "FileA", 5, 1);
    std::cout << "Process 0 creates FileA: " << (result1.success ? "SUCCESS" : "FAILED") << "\n";
    
    // Process 1 tries to delete file created by Process 0 (should fail)
    auto result2 = fs.deleteFile(1, "FileA", 1);
    std::cout << "Process 1 tries to delete FileA (non-owner): " << (result2.success ? "SUCCESS" : "FAILED") << "\n";
    std::cout << "  Message: " << result2.message << "\n";
    
    // Process 0 deletes its own file (should succeed)
    auto result3 = fs.deleteFile(0, "FileA", 1);
    std::cout << "Process 0 deletes FileA (owner): " << (result3.success ? "SUCCESS" : "FAILED") << "\n";
    
    // Real-time process (priority 0) can delete any file
    auto result4 = fs.createFile(1, "FileB", 3, 1);
    std::cout << "Process 1 creates FileB: " << (result4.success ? "SUCCESS" : "FAILED") << "\n";
    
    auto result5 = fs.deleteFile(0, "FileB", 0); // Priority 0 = real-time
    std::cout << "Real-time process deletes FileB (not owner): " << (result5.success ? "SUCCESS" : "FAILED") << "\n";
    std::cout << "  Message: " << result5.message << "\n";
}

/*
 * Test 3: Space allocation tests
 */
void testSpaceAllocation()
{
    std::cout << "\n===== TEST 3: Space Allocation Tests =====\n";
    
    FileSystem fs(10);
    
    // Fill almost all space
    auto result1 = fs.createFile(0, "Large", 8, 1);
    std::cout << "Create Large (8 blocks): " << (result1.success ? "SUCCESS" : "FAILED") << "\n";
    std::cout << "  Message: " << result1.message << "\n";
    std::cout << "Available space: " << fs.getAvailableSpace() << "/10 blocks\n";
    
    // Try to create file that won't fit
    auto result2 = fs.createFile(1, "TooBig", 5, 1);
    std::cout << "Create TooBig (5 blocks): " << (result2.success ? "SUCCESS" : "FAILED") << "\n";
    std::cout << "  Message: " << result2.message << "\n";
    
    // Create file that fits in remaining space
    auto result3 = fs.createFile(1, "Small", 2, 1);
    std::cout << "Create Small (2 blocks): " << (result3.success ? "SUCCESS" : "FAILED") << "\n";
    std::cout << "Available space: " << fs.getAvailableSpace() << "/10 blocks\n";
    
    // Try to create when disk is full
    auto result4 = fs.createFile(2, "Full", 1, 1);
    std::cout << "Create Full (1 block) when full: " << (result4.success ? "SUCCESS" : "FAILED") << "\n";
    std::cout << "  Message: " << result4.message << "\n";
}

/*
 * Test 4: First-fit allocation verification
 */
void testFirstFit()
{
    std::cout << "\n===== TEST 4: First-Fit Allocation =====\n";
    
    FileSystem fs(20);
    
    // Create fragmented layout
    fs.createFile(0, "A", 3, 1);
    fs.createFile(1, "B", 2, 1);
    fs.createFile(2, "C", 3, 1);
    
    std::cout << "Initial: " << fs.getDiskMapString() << "\n";
    
    // Delete middle file to create gap
    fs.deleteFile(1, "B", 1);
    std::cout << "After delete B: " << fs.getDiskMapString() << "\n";
    std::cout << "Available space: " << fs.getAvailableSpace() << "/20 blocks\n";
    
    // Create file that should use the first gap (first-fit)
    fs.createFile(3, "D", 2, 1);
    std::cout << "After create D (2 blocks, should fill first gap): " << fs.getDiskMapString() << "\n";
}

/*
 * Test 5: Disk state loading
 */
void testDiskStateLoading()
{
    std::cout << "\n===== TEST 5: Disk State Loading =====\n";
    
    FileSystem fs(10);
    
    // Load pre-existing state
    fs.loadDiskState("X:0:2|Y:3:1|Z:5:3");
    
    std::cout << "Loaded state: " << fs.getDiskMapString() << "\n";
    std::cout << "Available space: " << fs.getAvailableSpace() << "/10 blocks\n";
    
    // Try to create file using first-fit in remaining space
    auto result = fs.createFile(0, "NewFile", 2, 1);
    std::cout << "Create NewFile (2 blocks): " << (result.success ? "SUCCESS" : "FAILED") << "\n";
    std::cout << "  Message: " << result.message << "\n";
    std::cout << "Final state: " << fs.getDiskMapString() << "\n";
}

int main()
{
    std::cout << "===== FileSystem Module Tests =====\n";
    
    testBasicOperations();
    testPermissions();
    testSpaceAllocation();
    testFirstFit();
    testDiskStateLoading();
    
    std::cout << "\n===== All Tests Completed =====\n";
    
    return 0;
}
