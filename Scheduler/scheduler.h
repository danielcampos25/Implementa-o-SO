#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <queue>
#include "../ResourceManager/ResourceManager.h"

struct blockedBy;

struct BlockedProcess
{
    int pid;
    ResourceRequest request;
    blockedBy reason;
};

class Scheduler
{
private:
    std::queue<BlockedProcess> blockedIO;
    ResourceManager *resourceManager = nullptr;

public:
    Scheduler();
    void setResourceManager(ResourceManager *rm);

    void blockProcess(const ResourceRequest &request, const blockedBy &reason);
    void unblockProcess(int pid);

    void checkBlockedProcesses();

    std::queue<BlockedProcess> &getBlockedQueue();
    bool isBlocked(int pid) const;
};

#endif