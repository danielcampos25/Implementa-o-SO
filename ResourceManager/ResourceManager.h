#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include "Resource.h"

#include <array>
#include <mutex>

class ProcessScheduler;

class ResourceManager
{
private:
    int scannerOwner;
    int modemOwner;
    std::array<int, 2> printerOwners;
    std::array<int, 2> sataOwners;

    ProcessScheduler *scheduler;

    mutable std::recursive_mutex mtx;

public:
    ResourceManager(ProcessScheduler *scheduler = nullptr);

    void setScheduler(ProcessScheduler *scheduler);

    blockedBy canAllocate(const ResourceRequest &request);

    bool allocate(const ResourceRequest &req, bool canBlock = true);

    void release(int pid);

    void printStatus() const;
};

#endif