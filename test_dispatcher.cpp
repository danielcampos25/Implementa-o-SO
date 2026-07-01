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

void testDispatcherStoresReferenceStrings()
{
    const std::vector<std::vector<int>> referenceStrings = {{1, 2, 3}, {4, 5}};
    Dispatcher dispatcher({entry(0, 1, 1), entry(1, 1, 1)}, referenceStrings);

    assert(dispatcher.getReferenceStringCount() == referenceStrings.size());
    assert(!dispatcher.hasSimulationError());
}

void testAdmittedProcessGetsInitialPageFaultTotal()
{
    Dispatcher dispatcher({entry(0, 1, 1)}, {{7}});

    dispatcher.admitEligibleProcesses();

    assert(dispatcher.admittedCount() == 1);
    assert(dispatcher.hasPageFaultTotal(0));
    assert(dispatcher.getPageFaultsForPid(0) == 0);
    assert(!dispatcher.hasSimulationError());
}

void testRejectedProcessDoesNotGetPageFaultTotal()
{
    Dispatcher dispatcher({entry(0, 4, 1), entry(0, 1, 1)}, {{9}, {10}});

    dispatcher.admitEligibleProcesses();

    assert(dispatcher.rejectedCount() == 1);
    assert(dispatcher.admittedCount() == 1);
    assert(!dispatcher.hasPageFaultTotal(-1));
    assert(dispatcher.hasPageFaultTotal(0));
    assert(dispatcher.getPageFaultsForPid(0) == 0);
}

void testLastRunPidObservation()
{
    Dispatcher dispatcher({entry(0, 1, 2)}, {{1, 2}});

    assert(dispatcher.getScheduler().getLastRunPid() == -1);
    assert(dispatcher.runNext());

    assert(dispatcher.getScheduler().getLastRunPid() == 0);
    assert(dispatcher.getScheduler().getLastConsumedTime() == ProcessScheduler::USER_QUANTUM);
    assert(dispatcher.getCurrentCycle() == 1);
}

void testUserProcessPageFaultsAreCounted()
{
    Dispatcher dispatcher({entry(0, 1, 3)}, {{1, 2, 1}});

    dispatcher.runUntilComplete();

    assert(dispatcher.isComplete());
    assert(!dispatcher.hasSimulationError());
    assert(dispatcher.hasPageFaultTotal(0));
    assert(dispatcher.getPageFaultsForPid(0) == 1);
}

void testRepeatedPageDoesNotIncreasePageFaults()
{
    Dispatcher dispatcher({entry(0, 1, 2)}, {{5, 5}});

    dispatcher.runUntilComplete();

    assert(dispatcher.isComplete());
    assert(!dispatcher.hasSimulationError());
    assert(dispatcher.getPageFaultsForPid(0) == 0);
}

void testRealTimeProcessConsumesMultipleReferencesInOneDispatch()
{
    Dispatcher dispatcher({entry(0, 0, 3)}, {{1, 2, 3}});

    dispatcher.runUntilComplete();

    assert(dispatcher.isComplete());
    assert(!dispatcher.hasSimulationError());
    assert(dispatcher.getScheduler().getLastRunPid() == 0);
    assert(dispatcher.getScheduler().getLastConsumedTime() == 3);
    assert(dispatcher.getPageFaultsForPid(0) == 2);
}

void testShortReferenceStringIsValid()
{
    Dispatcher dispatcher({entry(0, 1, 2)}, {{8}});

    dispatcher.runUntilComplete();

    assert(dispatcher.isComplete());
    assert(!dispatcher.hasSimulationError());
    assert(dispatcher.getPageFaultsForPid(0) == 0);
}

void testZeroCpuProcessDoesNotConsumeReference()
{
    Dispatcher dispatcher({entry(0, 1, 0)}, {{42}});

    dispatcher.runUntilComplete();

    assert(dispatcher.isComplete());
    assert(!dispatcher.hasSimulationError());
    assert(dispatcher.hasPageFaultTotal(0));
    assert(dispatcher.getPageFaultsForPid(0) == 0);
}

void testPageFaultTotalsAreOrderedByPid()
{
    Dispatcher dispatcher({entry(0, 1, 1), entry(0, 1, 1), entry(0, 1, 1)},
                          {{1}, {2}, {3}});

    dispatcher.runUntilComplete();

    const std::vector<std::pair<int, int>> totals = dispatcher.getPageFaultTotals();
    assert(totals.size() == 3);
    assert(totals[0].first == 0);
    assert(totals[1].first == 1);
    assert(totals[2].first == 2);
}

void testPageFaultTotalsExcludeRejectedProcesses()
{
    Dispatcher dispatcher({entry(0, 4, 1), entry(0, 1, 1)}, {{10}, {20}});

    dispatcher.runUntilComplete();

    const std::vector<std::pair<int, int>> totals = dispatcher.getPageFaultTotals();
    assert(totals.size() == 1);
    assert(totals[0].first == 0);
}

void testPageFaultSummaryFormatting()
{
    Dispatcher dispatcher({entry(0, 1, 2)}, {{5, 5}});

    dispatcher.runUntilComplete();

    std::ostringstream output;
    dispatcher.printPageFaultSummary(output);
    const std::string text = output.str();

    assert(text.find("Número de Faltas de Páginas por processo:") != std::string::npos);
    assert(text.find("P0 = 0 faltas de páginas") != std::string::npos);
}

void testPageFaultSummaryAppearsAfterEvents()
{
    Dispatcher dispatcher({entry(0, 1, 1)}, {{7}});

    dispatcher.runUntilComplete();

    std::ostringstream output;
    dispatcher.printEvents(output);
    dispatcher.printPageFaultSummary(output);
    const std::string text = output.str();

    const std::size_t eventPosition = text.find("type=completion");
    const std::size_t summaryPosition = text.find("Número de Faltas de Páginas por processo:");
    assert(eventPosition != std::string::npos);
    assert(summaryPosition != std::string::npos);
    assert(eventPosition < summaryPosition);
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

    const std::vector<DispatcherEvent> idleEvents = eventsOfType(dispatcher, DispatcherEventType::IdleAdvance);
    assert(idleEvents[0].cycle == 0);
    assert(idleEvents[0].message.find("ciclo 4") != std::string::npos);
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

void testEventCycleSemantics()
{
    Dispatcher userDispatcher({entry(0, 3, 1)});
    userDispatcher.runUntilComplete();

    const std::vector<DispatcherEvent> userAdmissions = eventsOfType(userDispatcher, DispatcherEventType::Admission);
    const std::vector<DispatcherEvent> userDispatches = eventsOfType(userDispatcher, DispatcherEventType::Dispatch);
    const std::vector<DispatcherEvent> userQuantums = eventsOfType(userDispatcher, DispatcherEventType::Quantum);
    const std::vector<DispatcherEvent> userFinishes = eventsOfType(userDispatcher, DispatcherEventType::Finish);
    const std::vector<DispatcherEvent> userCompletions = eventsOfType(userDispatcher, DispatcherEventType::Completion);

    assert(userAdmissions[0].cycle == 0);
    assert(userDispatches[0].cycle == 0);
    assert(userQuantums[0].cycle == 1);
    assert(userFinishes[0].cycle == 1);
    assert(userCompletions[0].cycle == 1);

    Dispatcher realTimeDispatcher({entry(1, 0, 2)});
    realTimeDispatcher.runUntilComplete();

    const std::vector<DispatcherEvent> realTimeIdleEvents = eventsOfType(realTimeDispatcher, DispatcherEventType::IdleAdvance);
    const std::vector<DispatcherEvent> realTimeAdmissions = eventsOfType(realTimeDispatcher, DispatcherEventType::Admission);
    const std::vector<DispatcherEvent> realTimeDispatches = eventsOfType(realTimeDispatcher, DispatcherEventType::Dispatch);
    const std::vector<DispatcherEvent> realTimeFinishes = eventsOfType(realTimeDispatcher, DispatcherEventType::Finish);
    const std::vector<DispatcherEvent> realTimeCompletions = eventsOfType(realTimeDispatcher, DispatcherEventType::Completion);

    assert(realTimeIdleEvents[0].cycle == 0);
    assert(realTimeAdmissions[0].cycle == 1);
    assert(realTimeDispatches[0].cycle == 1);
    assert(realTimeFinishes[0].cycle == 3);
    assert(realTimeCompletions[0].cycle == 3);

    Dispatcher invalidDispatcher({entry(2, 4, 1)});
    invalidDispatcher.runUntilComplete();

    const std::vector<DispatcherEvent> invalidIdleEvents = eventsOfType(invalidDispatcher, DispatcherEventType::IdleAdvance);
    const std::vector<DispatcherEvent> rejections = eventsOfType(invalidDispatcher, DispatcherEventType::Rejection);
    const std::vector<DispatcherEvent> invalidCompletions = eventsOfType(invalidDispatcher, DispatcherEventType::Completion);

    assert(invalidIdleEvents[0].cycle == 0);
    assert(rejections[0].cycle == 2);
    assert(invalidCompletions[0].cycle == 2);
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

    testDispatcherStoresReferenceStrings();
    testAdmittedProcessGetsInitialPageFaultTotal();
    testRejectedProcessDoesNotGetPageFaultTotal();
    testLastRunPidObservation();
    testUserProcessPageFaultsAreCounted();
    testRepeatedPageDoesNotIncreasePageFaults();
    testRealTimeProcessConsumesMultipleReferencesInOneDispatch();
    testShortReferenceStringIsValid();
    testZeroCpuProcessDoesNotConsumeReference();
    testPageFaultTotalsAreOrderedByPid();
    testPageFaultTotalsExcludeRejectedProcesses();
    testPageFaultSummaryFormatting();
    testPageFaultSummaryAppearsAfterEvents();
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
    testEventCycleSemantics();
    testObservableEventsAndFormatting();

    std::cout << "All dispatcher tests passed.\n";
    return 0;
}
