#include "Process/Process.h"
#include "ProcessScheduler/ProcessScheduler.h"

#include <cassert>
#include <iostream>
#include <optional>
#include <vector>

int createSimpleProcess(ProcessScheduler &scheduler, int priority, int cpuTime)
{
    return scheduler.createProcess(0, priority, cpuTime, 64, 0, 0, 0, 0);
}

void testProcessModel()
{
    Process process(0, 3, 1, 5, 64, 1, 0, 1, 0);

    assert(process.getPid() == 0);
    assert(process.getStartTime() == 3);
    assert(process.getPriority() == 1);
    assert(process.getProcessorTime() == 5);
    assert(process.getRemainingTime() == 5);
    assert(process.getMemoryBlocks() == 64);
    assert(process.getPrinterRequest() == 1);
    assert(process.getScannerRequest() == 0);
    assert(process.getModemRequest() == 1);
    assert(process.getSataDiskRequest() == 0);
    assert(process.isUserProcess());
    assert(!process.isRealTime());

    process.markReady();
    assert(process.getState() == ProcessState::Ready);
    process.markRunning();
    assert(process.getState() == ProcessState::Running);
    assert(process.executeFor(2) == 2);
    assert(process.getRemainingTime() == 3);
    assert(!process.isFinished());
    assert(process.executeUntilFinished() == 3);
    assert(process.isFinished());

    Process agingProcess(1, 0, 3, 1, 1, 0, 0, 0, 0);
    agingProcess.markReady();
    agingProcess.incrementWaitingCycles();
    assert(agingProcess.getWaitingCycles() == 1);
    assert(agingProcess.promotePriority());
    assert(agingProcess.getPriority() == 2);
    assert(agingProcess.getWaitingCycles() == 0);

    bool invalidPriorityThrown = false;
    try
    {
        Process invalid(2, 0, 4, 1, 1, 0, 0, 0, 0);
        (void)invalid;
    }
    catch (const std::invalid_argument &)
    {
        invalidPriorityThrown = true;
    }
    assert(invalidPriorityThrown);
}

void testPriorityAndReadyQueues()
{
    ProcessScheduler scheduler;

    const int userPid = createSimpleProcess(scheduler, 1, 2);
    const int realTimePid = createSimpleProcess(scheduler, 0, 2);
    const int userPriority2Pid = createSimpleProcess(scheduler, 2, 2);
    const int userPriority3Pid = createSimpleProcess(scheduler, 3, 2);

    assert(userPid == 0);
    assert(realTimePid == 1);
    assert(userPriority2Pid == 2);
    assert(userPriority3Pid == 3);
    assert(scheduler.processCount() == 4);
    assert(scheduler.queueSize(0) == 1);
    assert(scheduler.queueSize(1) == 1);
    assert(scheduler.queueSize(2) == 1);
    assert(scheduler.queueSize(3) == 1);

    std::optional<int> next = scheduler.selectNextProcessId();
    assert(next.has_value());
    assert(next.value() == realTimePid);

    assert(scheduler.createProcess(0, 4, 1, 1, 0, 0, 0, 0) == -1);
    assert(scheduler.processCount() == 4);
}

void testRealTimeFifoAndUserPriority()
{
    ProcessScheduler scheduler;

    const int rt1 = createSimpleProcess(scheduler, 0, 1);
    const int rt2 = createSimpleProcess(scheduler, 0, 1);
    const int p3 = createSimpleProcess(scheduler, 3, 1);
    const int p2 = createSimpleProcess(scheduler, 2, 1);
    const int p1 = createSimpleProcess(scheduler, 1, 1);

    assert(scheduler.selectNextProcessId().value() == rt1);
    scheduler.runNext();
    assert(scheduler.selectNextProcessId().value() == rt2);
    scheduler.runNext();
    assert(scheduler.selectNextProcessId().value() == p1);
    scheduler.runNext();
    assert(scheduler.selectNextProcessId().value() == p2);
    scheduler.runNext();
    assert(scheduler.selectNextProcessId().value() == p3);
    scheduler.runNext();

    assert(scheduler.getProcessState(rt1) == ProcessState::Finished);
    assert(scheduler.getProcessState(rt2) == ProcessState::Finished);
    assert(scheduler.empty());
}

void testCapacityLimit()
{
    ProcessScheduler scheduler;

    for (int i = 0; i < ProcessScheduler::MAX_PROCESSES; ++i)
    {
        assert(createSimpleProcess(scheduler, 1, 1) == i);
    }

    assert(createSimpleProcess(scheduler, 1, 1) == -1);
    assert(scheduler.processCount() == ProcessScheduler::MAX_PROCESSES);
}

void testCpuSimulation()
{
    ProcessScheduler scheduler;

    const int rt = createSimpleProcess(scheduler, 0, 3);
    scheduler.runNext();
    assert(scheduler.getProcessRemainingTime(rt) == 0);
    assert(scheduler.getProcessState(rt) == ProcessState::Finished);
    assert(scheduler.queueSize(0) == 0);

    const int user = createSimpleProcess(scheduler, 1, 3);
    scheduler.runNext();
    assert(scheduler.getProcessRemainingTime(user) == 2);
    assert(scheduler.getProcessState(user) == ProcessState::Ready);
    assert(scheduler.queueSize(1) == 1);

    scheduler.runUntilComplete();
    assert(scheduler.getProcessState(user) == ProcessState::Finished);
    assert(scheduler.empty());

    const int zero = scheduler.createProcess(0, 1, 0, 64, 0, 0, 0, 0);
    assert(zero >= 0);
    assert(scheduler.getProcessState(zero) == ProcessState::Finished);
    assert(scheduler.getProcessRemainingTime(zero) == 0);
    assert(scheduler.empty());
}

void testRunUntilComplete()
{
    ProcessScheduler scheduler;

    std::vector<int> pids;
    pids.push_back(createSimpleProcess(scheduler, 3, 2));
    pids.push_back(createSimpleProcess(scheduler, 2, 2));
    pids.push_back(createSimpleProcess(scheduler, 1, 2));
    pids.push_back(createSimpleProcess(scheduler, 0, 2));

    scheduler.runUntilComplete();

    for (int pid : pids)
    {
        assert(scheduler.getProcessState(pid) == ProcessState::Finished);
        assert(scheduler.getProcessRemainingTime(pid) == 0);
    }
    assert(scheduler.empty());
}

void testAging()
{
    ProcessScheduler scheduler;

    const int p3 = createSimpleProcess(scheduler, 3, 1);
    const int p2 = createSimpleProcess(scheduler, 2, 1);
    const int p1 = createSimpleProcess(scheduler, 1, 10);

    assert(scheduler.getProcessPriority(p3) == 3);
    assert(scheduler.getProcessPriority(p2) == 2);
    assert(scheduler.getProcessPriority(p1) == 1);

    for (int i = 0; i < ProcessScheduler::DEFAULT_AGING_THRESHOLD; ++i)
    {
        scheduler.runNext();
    }

    assert(scheduler.getProcessPriority(p3) == 2);
    assert(scheduler.getProcessWaitingCycles(p3) == 0);
    assert(scheduler.getProcessPriority(p2) == 1);
    assert(scheduler.getProcessWaitingCycles(p2) == 0);
    assert(scheduler.getProcessPriority(p1) == 1);

    scheduler.runUntilComplete();
    assert(scheduler.empty());

    ProcessScheduler realTimeScheduler;
    const int rt = createSimpleProcess(realTimeScheduler, 0, 2);
    realTimeScheduler.runUntilComplete();
    assert(realTimeScheduler.getProcessPriority(rt) == 0);
}

void testEvents()
{
    ProcessScheduler scheduler;

    createSimpleProcess(scheduler, 0, 1);
    createSimpleProcess(scheduler, 1, 2);
    scheduler.runUntilComplete();

    bool sawCreate = false;
    bool sawDispatch = false;
    bool sawFinish = false;
    bool sawQuantum = false;

    for (const SchedulerEvent &event : scheduler.getEvents())
    {
        sawCreate = sawCreate || event.type == SchedulerEventType::Created;
        sawDispatch = sawDispatch || event.type == SchedulerEventType::Dispatch;
        sawFinish = sawFinish || event.type == SchedulerEventType::Finish;
        sawQuantum = sawQuantum || event.type == SchedulerEventType::Quantum;
    }

    assert(sawCreate);
    assert(sawDispatch);
    assert(sawFinish);
    assert(sawQuantum);
}

int main()
{
    std::cout << "===== ProcessScheduler Module Tests =====\n";

    testProcessModel();
    testPriorityAndReadyQueues();
    testRealTimeFifoAndUserPriority();
    testCapacityLimit();
    testCpuSimulation();
    testRunUntilComplete();
    testAging();
    testEvents();

    std::cout << "All process scheduler tests passed.\n";
    return 0;
}
