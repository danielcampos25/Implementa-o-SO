#include "Dispatcher/Dispatcher.h"
#include "Process/Process.h"

#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

ProcessWorkloadEntry entry(int startTime,
                           int priority,
                           int processorTime,
                           int memoryBlocks = 64,
                           int printerRequest = 0,
                           int scannerRequest = 0,
                           int modemRequest = 0,
                           int sataDiskRequest = 0)
{
    return {startTime,
            priority,
            processorTime,
            memoryBlocks,
            printerRequest,
            scannerRequest,
            modemRequest,
            sataDiskRequest,
            0};
}

int countEvents(const Dispatcher &dispatcher, DispatcherEventType type)
{
    int count = 0;
    for (const DispatcherEvent &event : dispatcher.getEvents())
    {
        if (event.type == type)
        {
            ++count;
        }
    }
    return count;
}

std::vector<DispatcherEvent> eventsOfType(const Dispatcher &dispatcher, DispatcherEventType type)
{
    std::vector<DispatcherEvent> matches;
    for (const DispatcherEvent &event : dispatcher.getEvents())
    {
        if (event.type == type)
        {
            matches.push_back(event);
        }
    }
    return matches;
}

void testInitialAdmissionAndPendingProcesses()
{
    Dispatcher dispatcher({entry(0, 1, 1), entry(2, 1, 1), entry(5, 1, 1)});

    dispatcher.admitEligibleProcesses();

    assert(dispatcher.getCurrentCycle() == 0);
    assert(dispatcher.admittedCount() == 1);
    assert(dispatcher.pendingCount() == 2);
    assert(dispatcher.rejectedCount() == 0);
    assert(dispatcher.getScheduler().queueSize(Process::USER_PRIORITY_HIGH) == 1);
    assert(countEvents(dispatcher, DispatcherEventType::Admission) == 1);
}

void testSameStartTimeOrdering()
{
    Dispatcher dispatcher({entry(0, 1, 1), entry(0, 1, 1), entry(0, 1, 1)});

    dispatcher.admitEligibleProcesses();
    const std::vector<DispatcherEvent> admissions = eventsOfType(dispatcher, DispatcherEventType::Admission);

    assert(admissions.size() == 3);
    assert(admissions[0].pid == 0);
    assert(admissions[1].pid == 1);
    assert(admissions[2].pid == 2);
    assert(dispatcher.pendingCount() == 0);
}

void testNoDuplicateAdmission()
{
    Dispatcher dispatcher({entry(0, 1, 1), entry(0, 1, 1)});

    dispatcher.admitEligibleProcesses();
    dispatcher.admitEligibleProcesses();

    assert(dispatcher.admittedCount() == 2);
    assert(countEvents(dispatcher, DispatcherEventType::Admission) == 2);
    assert(dispatcher.getScheduler().processCount() == 2);
}

void testInvalidEntriesRejectedAtAdmission()
{
    Dispatcher dispatcher({entry(0, 4, 1), entry(0, 1, 1)});

    dispatcher.admitEligibleProcesses();

    assert(dispatcher.rejectedCount() == 1);
    assert(dispatcher.admittedCount() == 1);
    assert(dispatcher.pendingCount() == 0);
    assert(countEvents(dispatcher, DispatcherEventType::Rejection) == 1);
}

void testDispatcherCapacityLimit()
{
    std::vector<ProcessWorkloadEntry> workload;
    for (int i = 0; i < ProcessScheduler::MAX_PROCESSES + 1; ++i)
    {
        workload.push_back(entry(0, 1, 1));
    }

    Dispatcher dispatcher(workload);
    dispatcher.admitEligibleProcesses();

    assert(dispatcher.admittedCount() == ProcessScheduler::MAX_PROCESSES);
    assert(dispatcher.rejectedCount() == 1);
    assert(dispatcher.pendingCount() == 0);
}

void testIdleAdvanceToFirstFutureStart()
{
    Dispatcher dispatcher({entry(4, 1, 1)});

    assert(dispatcher.runNext());

    assert(dispatcher.getCurrentCycle() == 4);
    assert(dispatcher.admittedCount() == 1);
    assert(dispatcher.pendingCount() == 0);
    assert(countEvents(dispatcher, DispatcherEventType::IdleAdvance) == 1);
}

void testGapHandlingAndCompletion()
{
    Dispatcher dispatcher({entry(0, 1, 1), entry(3, 1, 1)});

    dispatcher.runUntilComplete();

    assert(dispatcher.isComplete());
    assert(dispatcher.getCurrentCycle() == 4);
    assert(dispatcher.admittedCount() == 2);
    assert(countEvents(dispatcher, DispatcherEventType::IdleAdvance) == 1);
    assert(countEvents(dispatcher, DispatcherEventType::Completion) == 1);
}

void testAdmissionAfterLongRealTimeExecution()
{
    Dispatcher dispatcher({entry(0, 0, 5), entry(2, 1, 1), entry(4, 1, 1)});

    assert(dispatcher.runNext());

    assert(dispatcher.getCurrentCycle() == 5);
    assert(dispatcher.admittedCount() == 3);
    assert(dispatcher.pendingCount() == 0);
    assert(dispatcher.getScheduler().queueSize(Process::USER_PRIORITY_HIGH) == 2);
}

void testEmptyWorkloadCompletes()
{
    Dispatcher dispatcher;

    dispatcher.runUntilComplete();

    assert(dispatcher.isComplete());
    assert(dispatcher.pendingCount() == 0);
    assert(countEvents(dispatcher, DispatcherEventType::Completion) == 1);
}

void testZeroCpuProcessFinishesAtAdmission()
{
    Dispatcher dispatcher({entry(0, 1, 0)});

    dispatcher.runUntilComplete();

    assert(dispatcher.isComplete());
    assert(dispatcher.admittedCount() == 1);
    assert(countEvents(dispatcher, DispatcherEventType::Admission) == 1);
    assert(countEvents(dispatcher, DispatcherEventType::Finish) == 1);
    assert(dispatcher.getScheduler().empty());
}

void testObservableEventsAndFormatting()
{
    Dispatcher dispatcher({entry(0, 1, 2), entry(2, 4, 1), entry(3, 1, 1)});

    dispatcher.runUntilComplete();

    assert(countEvents(dispatcher, DispatcherEventType::Admission) >= 2);
    assert(countEvents(dispatcher, DispatcherEventType::Rejection) == 1);
    assert(countEvents(dispatcher, DispatcherEventType::IdleAdvance) >= 1);
    assert(countEvents(dispatcher, DispatcherEventType::Dispatch) >= 1);
    assert(countEvents(dispatcher, DispatcherEventType::Quantum) >= 1);
    assert(countEvents(dispatcher, DispatcherEventType::Finish) >= 1);
    assert(countEvents(dispatcher, DispatcherEventType::Completion) == 1);

    std::ostringstream output;
    dispatcher.printEvents(output);
    const std::string text = output.str();

    assert(text.find("cycle=") != std::string::npos);
    assert(text.find("pid=") != std::string::npos);
    assert(text.find("priority=") != std::string::npos);
    assert(text.find("remaining=") != std::string::npos);
    assert(text.find("type=admission") != std::string::npos);
    assert(text.find("type=dispatch") != std::string::npos);
    assert(text.find("type=quantum") != std::string::npos);
    assert(text.find("type=finish") != std::string::npos);
}

int main()
{
    std::cout << "===== Dispatcher Integration Tests =====\n";

    testInitialAdmissionAndPendingProcesses();
    testSameStartTimeOrdering();
    testNoDuplicateAdmission();
    testInvalidEntriesRejectedAtAdmission();
    testDispatcherCapacityLimit();
    testIdleAdvanceToFirstFutureStart();
    testGapHandlingAndCompletion();
    testAdmissionAfterLongRealTimeExecution();
    testEmptyWorkloadCompletes();
    testZeroCpuProcessFinishesAtAdmission();
    testObservableEventsAndFormatting();

    std::cout << "All dispatcher tests passed.\n";
    return 0;
}
