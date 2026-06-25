#include "Dispatcher.h"

#include <algorithm>
#include <ostream>
#include <sstream>

namespace
{
DispatcherEventType toDispatcherEventType(SchedulerEventType type)
{
    switch (type)
    {
    case SchedulerEventType::Dispatch:
        return DispatcherEventType::Dispatch;
    case SchedulerEventType::Quantum:
        return DispatcherEventType::Quantum;
    case SchedulerEventType::Aging:
        return DispatcherEventType::Aging;
    case SchedulerEventType::Finish:
        return DispatcherEventType::Finish;
    case SchedulerEventType::Rejected:
        return DispatcherEventType::Rejection;
    case SchedulerEventType::Created:
    case SchedulerEventType::Idle:
        return DispatcherEventType::Completion;
    }

    return DispatcherEventType::Completion;
}

bool shouldForwardSchedulerEvent(SchedulerEventType type)
{
    return type == SchedulerEventType::Dispatch ||
           type == SchedulerEventType::Quantum ||
           type == SchedulerEventType::Finish ||
           type == SchedulerEventType::Aging ||
           type == SchedulerEventType::Rejected;
}
}

Dispatcher::Dispatcher()
    : workload(),
      events(),
      scheduler(),
      currentCycle(0),
      admittedProcesses(0),
      rejectedProcesses(0),
      completionRecorded(false),
      schedulerEventCursor(0)
{
}

Dispatcher::Dispatcher(const std::vector<ProcessWorkloadEntry> &entries)
    : Dispatcher()
{
    setWorkload(entries);
}

void Dispatcher::setWorkload(const std::vector<ProcessWorkloadEntry> &entries)
{
    workload.clear();
    events.clear();
    scheduler = ProcessScheduler();
    currentCycle = 0;
    admittedProcesses = 0;
    rejectedProcesses = 0;
    completionRecorded = false;
    schedulerEventCursor = 0;

    for (std::size_t index = 0; index < entries.size(); ++index)
    {
        ProcessWorkloadEntry entry = entries[index];
        entry.inputOrder = static_cast<int>(index);
        workload.push_back({entry, false, false});
    }
}

void Dispatcher::recordEvent(DispatcherEventType type,
                             int pid,
                             int priority,
                             int remainingTime,
                             const std::string &message)
{
    events.push_back({currentCycle, type, pid, priority, remainingTime, message});
}

bool Dispatcher::hasPendingProcess() const
{
    for (const PendingProcess &process : workload)
    {
        if (!process.admitted && !process.rejected)
        {
            return true;
        }
    }

    return false;
}

int Dispatcher::nextPendingStartTime() const
{
    int nextStartTime = -1;

    for (const PendingProcess &process : workload)
    {
        if (!process.admitted && !process.rejected)
        {
            if (nextStartTime < 0 || process.entry.startTime < nextStartTime)
            {
                nextStartTime = process.entry.startTime;
            }
        }
    }

    return nextStartTime;
}

void Dispatcher::admitEligibleProcesses()
{
    for (PendingProcess &pending : workload)
    {
        if (pending.admitted || pending.rejected || pending.entry.startTime > currentCycle)
        {
            continue;
        }

        const ProcessWorkloadEntry &entry = pending.entry;
        const int pid = scheduler.createProcess(entry.startTime,
                                                entry.priority,
                                                entry.processorTime,
                                                entry.memoryBlocks,
                                                entry.printerRequest,
                                                entry.scannerRequest,
                                                entry.modemRequest,
                                                entry.sataDiskRequest);
        schedulerEventCursor = scheduler.getEvents().size();

        if (pid < 0)
        {
            pending.rejected = true;
            ++rejectedProcesses;
            recordEvent(DispatcherEventType::Rejection,
                        -1,
                        entry.priority,
                        entry.processorTime,
                        "Processo rejeitado na admissao");
            continue;
        }

        pending.admitted = true;
        ++admittedProcesses;
        const Process &process = scheduler.getProcess(pid);
        recordEvent(DispatcherEventType::Admission,
                    pid,
                    process.getPriority(),
                    process.getRemainingTime(),
                    "Processo admitido");

        if (process.isFinished())
        {
            recordEvent(DispatcherEventType::Finish,
                        pid,
                        process.getPriority(),
                        process.getRemainingTime(),
                        "Processo finalizado sem despacho de CPU");
        }
    }
}

void Dispatcher::forwardSchedulerEvents()
{
    const std::vector<SchedulerEvent> &schedulerEvents = scheduler.getEvents();

    while (schedulerEventCursor < schedulerEvents.size())
    {
        const SchedulerEvent &event = schedulerEvents[schedulerEventCursor++];

        if (!shouldForwardSchedulerEvent(event.type))
        {
            continue;
        }

        recordEvent(toDispatcherEventType(event.type),
                    event.pid,
                    event.priority,
                    event.remainingTime,
                    event.message);
    }
}

void Dispatcher::recordCompletionIfNeeded()
{
    if (!completionRecorded && !hasPendingProcess() && !scheduler.hasReadyProcess())
    {
        completionRecorded = true;
        recordEvent(DispatcherEventType::Completion,
                    -1,
                    -1,
                    -1,
                    "Simulacao concluida");
    }
}

bool Dispatcher::runNext()
{
    admitEligibleProcesses();

    if (scheduler.hasReadyProcess())
    {
        const bool ran = scheduler.runNext();
        if (!ran)
        {
            forwardSchedulerEvents();
            return false;
        }

        currentCycle += scheduler.getLastConsumedTime();
        forwardSchedulerEvents();
        admitEligibleProcesses();
        recordCompletionIfNeeded();
        return true;
    }

    if (hasPendingProcess())
    {
        const int nextStartTime = nextPendingStartTime();
        if (nextStartTime > currentCycle)
        {
            currentCycle = nextStartTime;
            recordEvent(DispatcherEventType::IdleAdvance,
                        -1,
                        -1,
                        -1,
                        "Sem processo pronto; avancando ate a proxima chegada");
        }

        admitEligibleProcesses();
        recordCompletionIfNeeded();
        return scheduler.hasReadyProcess() || hasPendingProcess();
    }

    recordCompletionIfNeeded();
    return false;
}

void Dispatcher::runUntilComplete()
{
    while (runNext())
    {
    }
}

int Dispatcher::getCurrentCycle() const
{
    return currentCycle;
}

std::size_t Dispatcher::pendingCount() const
{
    std::size_t count = 0;

    for (const PendingProcess &process : workload)
    {
        if (!process.admitted && !process.rejected)
        {
            ++count;
        }
    }

    return count;
}

int Dispatcher::admittedCount() const
{
    return admittedProcesses;
}

int Dispatcher::rejectedCount() const
{
    return rejectedProcesses;
}

bool Dispatcher::isComplete() const
{
    return !hasPendingProcess() && !scheduler.hasReadyProcess();
}

const std::vector<DispatcherEvent> &Dispatcher::getEvents() const
{
    return events;
}

const ProcessScheduler &Dispatcher::getScheduler() const
{
    return scheduler;
}

std::string Dispatcher::eventTypeName(DispatcherEventType type)
{
    switch (type)
    {
    case DispatcherEventType::Admission:
        return "admission";
    case DispatcherEventType::Rejection:
        return "rejection";
    case DispatcherEventType::IdleAdvance:
        return "idle";
    case DispatcherEventType::Dispatch:
        return "dispatch";
    case DispatcherEventType::Quantum:
        return "quantum";
    case DispatcherEventType::Aging:
        return "aging";
    case DispatcherEventType::Finish:
        return "finish";
    case DispatcherEventType::Completion:
        return "completion";
    }

    return "unknown";
}

std::string Dispatcher::formatEvent(const DispatcherEvent &event)
{
    std::ostringstream output;
    output << "cycle=" << event.cycle
           << " type=" << eventTypeName(event.type)
           << " pid=" << event.pid
           << " priority=" << event.priority
           << " remaining=" << event.remainingTime
           << " message=" << event.message;
    return output.str();
}

void Dispatcher::printEvents(std::ostream &output) const
{
    for (const DispatcherEvent &event : events)
    {
        output << formatEvent(event) << '\n';
    }
}
