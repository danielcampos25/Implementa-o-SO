#define ProcessType SchedulerProcessType
#include "Dispatcher.h"
#undef ProcessType

#include "../MemoryManager/MemoryManager.h"

#include <algorithm>
#include <memory>
#include <ostream>
#include <sstream>

namespace
{
ProcessType memoryTypeForPriority(int priority)
{
    return priority == Process::REAL_TIME_PRIORITY ? REAL_TIME : USER;
}

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
      schedulerEventCursor(0),
      referenceStrings(),
      referenceStringsConfigured(false),
      pidReferenceIndexes(),
      pidReferenceCursors(),
      pageFaultsByPid(),
      releasedPids(),
      memoryManager(std::make_unique<MemoryManager>()),
      simulationError(false),
      simulationErrorMessage()
{
}

Dispatcher::~Dispatcher() = default;

Dispatcher::Dispatcher(const std::vector<ProcessWorkloadEntry> &entries)
    : Dispatcher()
{
    setWorkload(entries);
}

Dispatcher::Dispatcher(const std::vector<ProcessWorkloadEntry> &entries,
                       const std::vector<std::vector<int>> &referenceStrings)
    : Dispatcher()
{
    setReferenceStrings(referenceStrings);
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
    clearMemoryAccountingState();

    for (std::size_t index = 0; index < entries.size(); ++index)
    {
        ProcessWorkloadEntry entry = entries[index];
        entry.inputOrder = static_cast<int>(index);
        workload.push_back({entry, false, false});
    }
}

void Dispatcher::setReferenceStrings(const std::vector<std::vector<int>> &newReferenceStrings)
{
    referenceStrings = newReferenceStrings;
    referenceStringsConfigured = true;
    clearMemoryAccountingState();
}

void Dispatcher::clearMemoryAccountingState()
{
    pidReferenceIndexes.clear();
    pidReferenceCursors.clear();
    pageFaultsByPid.clear();
    releasedPids.clear();
    memoryManager = std::make_unique<MemoryManager>();
    simulationError = false;
    simulationErrorMessage.clear();
}

void Dispatcher::recordEvent(DispatcherEventType type,
                             int pid,
                             int priority,
                             int remainingTime,
                             const std::string &message)
{
    recordEventAtCycle(currentCycle, type, pid, priority, remainingTime, message);
}

void Dispatcher::recordEventAtCycle(int cycle,
                                    DispatcherEventType type,
                                    int pid,
                                    int priority,
                                    int remainingTime,
                                    const std::string &message)
{
    events.push_back({cycle, type, pid, priority, remainingTime, message});
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

void Dispatcher::initializeProcessMemoryAccounting(int pid, const ProcessWorkloadEntry &entry)
{
    memoryManager->register_process_working_set_limit(pid, entry.memoryBlocks);

    if (!referenceStringsConfigured)
    {
        return;
    }

    if (entry.inputOrder < 0 || static_cast<std::size_t>(entry.inputOrder) >= referenceStrings.size())
    {
        simulationError = true;
        simulationErrorMessage = "String de referencia ausente para o processo " + std::to_string(pid);
        return;
    }

    pidReferenceIndexes[pid] = static_cast<std::size_t>(entry.inputOrder);
    pidReferenceCursors[pid] = 0;
    pageFaultsByPid[pid] = 0;

    preloadFirstReferenceIfAvailable(pid, entry);
}

void Dispatcher::preloadFirstReferenceIfAvailable(int pid, const ProcessWorkloadEntry &entry)
{
    if (!referenceStringsConfigured || entry.processorTime <= 0)
    {
        return;
    }

    const auto indexIt = pidReferenceIndexes.find(pid);
    if (indexIt == pidReferenceIndexes.end() || indexIt->second >= referenceStrings.size())
    {
        return;
    }

    const std::vector<int> &references = referenceStrings[indexIt->second];
    if (references.empty())
    {
        simulationError = true;
        simulationErrorMessage = "String de referencia vazia para o processo " + std::to_string(pid);
        return;
    }

    const ProcessType memoryType = memoryTypeForPriority(entry.priority);

    // Pré-carga: carrega a primeira página sem somar o retorno ao contador de faltas.
    memoryManager->ref_page(pid, references.front(), memoryType);
}

bool Dispatcher::consumeAllReferencesForProcess(int pid)
{
    if (!referenceStringsConfigured || pid < 0)
    {
        return true;
    }

    const Process &process = scheduler.getProcess(pid);
    if (process.getProcessorTime() <= 0)
    {
        return true;
    }

    const auto indexIt = pidReferenceIndexes.find(pid);
    const auto cursorIt = pidReferenceCursors.find(pid);
    if (indexIt == pidReferenceIndexes.end() || cursorIt == pidReferenceCursors.end())
    {
        simulationError = true;
        simulationErrorMessage = "String de referencia ausente para o processo " + std::to_string(pid);
        return false;
    }

    const std::size_t referenceIndex = indexIt->second;
    if (referenceIndex >= referenceStrings.size())
    {
        simulationError = true;
        simulationErrorMessage = "Indice de string de referencia invalido para o processo " + std::to_string(pid);
        return false;
    }

    const ProcessType memoryType = memoryTypeForPriority(process.getPriority());
    const std::vector<int> &references = referenceStrings[referenceIndex];
    std::size_t &cursor = cursorIt->second;

    while (cursor < references.size())
    {
        const int page = references[cursor++];
        pageFaultsByPid[pid] += memoryManager->ref_page(pid, page, memoryType);
    }

    return true;
}
void Dispatcher::releaseProcessMemoryIfFinished(int pid)
{
    if (pid < 0 || releasedPids.find(pid) != releasedPids.end())
    {
        return;
    }

    const Process &process = scheduler.getProcess(pid);
    if (!process.isFinished())
    {
        return;
    }

    if (!consumeAllReferencesForProcess(pid))
{
    return;
}

    memoryManager->free_process_memory(pid, memoryTypeForPriority(process.getPriority()));
    releasedPids.insert(pid);
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
        initializeProcessMemoryAccounting(pid, entry);
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
            releaseProcessMemoryIfFinished(pid);
        }
    }
}

void Dispatcher::forwardSchedulerEvents(int cycleOffset)
{
    const std::vector<SchedulerEvent> &schedulerEvents = scheduler.getEvents();

    while (schedulerEventCursor < schedulerEvents.size())
    {
        const SchedulerEvent &event = schedulerEvents[schedulerEventCursor++];

        if (!shouldForwardSchedulerEvent(event.type))
        {
            continue;
        }

        recordEventAtCycle(event.cycle + cycleOffset,
                           toDispatcherEventType(event.type),
                           event.pid,
                           event.priority,
                           event.remainingTime,
                           event.message);
    }
}

void Dispatcher::recordCompletionIfNeeded()
{
    if (!simulationError && !completionRecorded && !hasPendingProcess() && !scheduler.hasReadyProcess())
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
    if (simulationError)
    {
        return false;
    }

    admitEligibleProcesses();

    if (simulationError)
    {
        return false;
    }

    if (scheduler.hasReadyProcess())
    {
        const int dispatcherCycleBeforeRun = currentCycle;
        const int schedulerCycleBeforeRun = scheduler.getCurrentCycle();
        const bool ran = scheduler.runNext();
        const int cycleOffset = dispatcherCycleBeforeRun - schedulerCycleBeforeRun;
        if (!ran)
        {
            forwardSchedulerEvents(cycleOffset);
            return false;
        }

        const int lastRunPid = scheduler.getLastRunPid();
        currentCycle += scheduler.getLastConsumedTime();
        forwardSchedulerEvents(cycleOffset);

        releaseProcessMemoryIfFinished(lastRunPid);
        if (simulationError)
        {
            return false;
        }

        admitEligibleProcesses();
        recordCompletionIfNeeded();
        return true;
    }

    if (hasPendingProcess())
    {
        const int nextStartTime = nextPendingStartTime();
        if (nextStartTime > currentCycle)
        {
            const int idleCycle = currentCycle;
            currentCycle = nextStartTime;
            recordEventAtCycle(idleCycle,
                               DispatcherEventType::IdleAdvance,
                               -1,
                               -1,
                               -1,
                               "Sem processo pronto; avancando ate o ciclo " + std::to_string(nextStartTime));
        }

        admitEligibleProcesses();
        recordCompletionIfNeeded();
        return !simulationError && (scheduler.hasReadyProcess() || hasPendingProcess());
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

std::size_t Dispatcher::getReferenceStringCount() const
{
    return referenceStrings.size();
}

bool Dispatcher::hasPageFaultTotal(int pid) const
{
    return pageFaultsByPid.find(pid) != pageFaultsByPid.end();
}

int Dispatcher::getPageFaultsForPid(int pid) const
{
    const auto it = pageFaultsByPid.find(pid);
    if (it == pageFaultsByPid.end())
    {
        return 0;
    }

    return it->second;
}

std::vector<std::pair<int, int>> Dispatcher::getPageFaultTotals() const
{
    std::vector<std::pair<int, int>> totals;
    totals.reserve(pageFaultsByPid.size());

    for (const auto &entry : pageFaultsByPid)
    {
        totals.push_back(entry);
    }

    std::sort(totals.begin(), totals.end());
    return totals;
}

bool Dispatcher::hasSimulationError() const
{
    return simulationError;
}

const std::string &Dispatcher::getSimulationErrorMessage() const
{
    return simulationErrorMessage;
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

void Dispatcher::printPageFaultSummary(std::ostream &output) const
{
    output << "Número de Faltas de Páginas por processo:\n";

    for (const auto &total : getPageFaultTotals())
    {
        output << 'P' << total.first << " = " << total.second << " faltas de páginas\n";
    }
}

void Dispatcher::printMemoryTables() {
    memoryManager->show_memory_table(REAL_TIME);
    memoryManager->show_memory_table(USER);
}