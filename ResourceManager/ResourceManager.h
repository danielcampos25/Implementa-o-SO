#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include "Resource.h"

class Scheduler;

struct blockedBy
{
    bool scanner;
    bool modem;
    bool printer;
    bool sata;
};

class ResourceManager
{
private:
    int scannerOwner;
    int modemOwner;
    int printerOwners[2];
    int sataOwners[2];
    Scheduler* scheduler;

public:

    ResourceManager(Scheduler* scheduler = nullptr);

    void setScheduler(Scheduler* scheduler);

    blockedBy canAllocate(const ResourceRequest& request);

    bool allocate(const ResourceRequest& request);

    void release(int pid);

    void printStatus() const;
};

#endif