#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include "Resource.h"
#include <array>
#include <mutex>

class Scheduler;

/*
 * blockedBy descreve quais recursos impedem a alocação de um processo.
 * Cada campo booleano indica se aquele recurso está ocupando e bloqueando
 * o processo que faz a requisição.
 */
struct blockedBy
{
    bool scanner;
    bool modem;
    bool printer;
    bool sata;
    int pid; // PID do processo que está sendo analisado (para debug)
};

/*
 * ResourceManager gerencia a alocação de dispositivos de E/S.
 * Ele sabe quais processos possuem scanner, modem, impressoras e SATA.
 * Além disso, notifica o Scheduler quando recursos são liberados.
 */
class ResourceManager
{
private:
    int scannerOwner;                 // PID do processo com scanner (ou -1 se livre)
    int modemOwner;                   // PID do processo com modem (ou -1 se livre)
    std::array<int, 2> printerOwners; // PIDs dos donos das impressoras
    std::array<int, 2> sataOwners;    // PIDs dos donos das portas SATA
    Scheduler *scheduler;             // Scheduler associado
    mutable std::recursive_mutex mtx; // Protege acesso concorrente

public:
    ResourceManager(Scheduler *scheduler = nullptr);

    void setScheduler(Scheduler *scheduler);

    blockedBy canAllocate(const ResourceRequest &request);

    bool allocate(const ResourceRequest &req, bool canBlock = true);

    void release(int pid);

    void printStatus() const;
};

#endif