#ifndef RESOURCE_H
#define RESOURCE_H
using namespace std;

struct ResourceRequest
{
    int pid;

    bool printer;
    bool scanner;
    bool modem;
    bool sata;
};

#endif