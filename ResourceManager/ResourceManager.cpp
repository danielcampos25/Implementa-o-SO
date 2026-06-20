#include "ResourceManager.h"

#include <iostream>

using namespace std;

ResourceManager::ResourceManager()
{
    scannerOwner = -1;
    modemOwner = -1;

    for(int i = 0; i < 2; i++)
    {
        printerOwners[i] = -1;
        sataOwners[i] = -1;
    }
}

bool ResourceManager::allocate(const ResourceRequest& request)
{
    bool scannerAvailable =
        !request.scanner ||
        scannerOwner == -1;

    bool modemAvailable =
        !request.modem ||
        modemOwner == -1;

    bool printerAvailable =
        !request.printer ||
        printerOwners[0] == -1 ||
        printerOwners[1] == -1;

    bool sataAvailable =
        !request.sata ||
        sataOwners[0] == -1 ||
        sataOwners[1] == -1;

    if(!(scannerAvailable &&
         modemAvailable &&
         printerAvailable &&
         sataAvailable))
    {
        return false;
    }

    if(request.scanner)
    {
        scannerOwner = request.pid;
    }

    if(request.modem)
    {
        modemOwner = request.pid;
    }

    if(request.printer)
    {
        if(printerOwners[0] == -1)
        {
            printerOwners[0] = request.pid;
        }
        else
        {
            printerOwners[1] = request.pid;
        }
    }

    if(request.sata)
    {
        if(sataOwners[0] == -1)
        {
            sataOwners[0] = request.pid;
        }
        else
        {
            sataOwners[1] = request.pid;
        }
    }

    return true;
}

void ResourceManager::release(int pid)
{
    if(scannerOwner == pid)
    {
        scannerOwner = -1;
    }

    if(modemOwner == pid)
    {
        modemOwner = -1;
    }

    for(int i = 0; i < 2; i++)
    {
        if(printerOwners[i] == pid)
        {
            printerOwners[i] = -1;
        }

        if(sataOwners[i] == pid)
        {
            sataOwners[i] = -1;
        }
    }
}

void ResourceManager::printStatus() const
{
    cout << "\n===== RESOURCE STATUS =====\n";

    cout << "Scanner: ";
    if(scannerOwner == -1)
        cout << "FREE\n";
    else
        cout << "PID " << scannerOwner << "\n";

    cout << "Modem: ";
    if(modemOwner == -1)
        cout << "FREE\n";
    else
        cout << "PID " << modemOwner << "\n";

    cout << "Printer 0: ";
    if(printerOwners[0] == -1)
        cout << "FREE\n";
    else
        cout << "PID " << printerOwners[0] << "\n";

    cout << "Printer 1: ";
    if(printerOwners[1] == -1)
        cout << "FREE\n";
    else
        cout << "PID " << printerOwners[1] << "\n";

    cout << "SATA 0: ";
    if(sataOwners[0] == -1)
        cout << "FREE\n";
    else
        cout << "PID " << sataOwners[0] << "\n";

    cout << "SATA 1: ";
    if(sataOwners[1] == -1)
        cout << "FREE\n";
    else
        cout << "PID " << sataOwners[1] << "\n";

    cout << "===========================\n";
}