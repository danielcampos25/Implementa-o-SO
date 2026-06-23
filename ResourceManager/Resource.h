#ifndef RESOURCE_H
#define RESOURCE_H

struct ResourceRequest
{
    int pid;

    bool printer;
    bool scanner;
    bool modem;
    bool sata;
};

#endif