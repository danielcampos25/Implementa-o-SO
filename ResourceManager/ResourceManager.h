#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include "Resource.h"
#include <array>
#include <mutex>

class Scheduler;

struct blockedBy
{
    bool scanner;
    bool modem;
    bool printer;
    bool sata;
    int pid; // PID do processo que está bloqueando (para debug)
};

class ResourceManager
{
private:
    int scannerOwner;
    int modemOwner;
    std::array<int, 2> printerOwners;
    std::array<int, 2> sataOwners;
    Scheduler *scheduler;
    mutable std::recursive_mutex mtx;

public:
    ResourceManager(Scheduler *scheduler = nullptr);

    void setScheduler(Scheduler *scheduler);

    blockedBy canAllocate(const ResourceRequest &request);

    bool allocate(const ResourceRequest &req, bool canBlock = true);

    void release(int pid);

    void printStatus() const;
};

#endif