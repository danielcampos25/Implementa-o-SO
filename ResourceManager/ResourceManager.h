#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include "Resource.h"

class ResourceManager
{
private:

    int scannerOwner;

    int modemOwner;

    int printerOwners[2];

    int sataOwners[2];

public:

    ResourceManager();

    bool allocate(const ResourceRequest& request);

    void release(int pid);

    void printStatus() const;
};

#endif