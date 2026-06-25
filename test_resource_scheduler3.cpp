#include "ResourceManager/ResourceManager.h"
#include "Scheduler/scheduler.h"
#include <iostream>

/*
 * Teste 3: verifica desbloqueios parciais em cadeia.
 *
 * Objetivos:
 * - P1, P2 e P3 alocam recursos distintos.
 * - P4 e P5 ficam bloqueados por recursos já ocupados.
 * - Quando P1 libera recursos, apenas um bloqueado deve ser atendido.
 * - Quando P2 libera recursos, o último bloqueado deve ser atendido.
 */
int main()
{
    Scheduler scheduler;
    ResourceManager rm(&scheduler);
    scheduler.setResourceManager(&rm);
    rm.setScheduler(&scheduler);

    ResourceRequest p1{1, true, true, false, false};  // printer + scanner
    ResourceRequest p2{2, false, false, true, false}; // modem only
    ResourceRequest p3{3, false, false, false, true}; // sata only
    ResourceRequest p4{4, true, false, true, false};  // printer + modem
    ResourceRequest p5{5, false, true, false, false}; // scanner only

    std::cout << "=== TESTE 3: bloqueios em fila e desbloqueio parcial ===\n";
    std::cout << "P1, P2, P3 alocam recursos iniciais\n";
    bool allocated1 = rm.allocate(p1);
    bool allocated2 = rm.allocate(p2);
    bool allocated3 = rm.allocate(p3);
    std::cout << "P1: " << (allocated1 ? "sim" : "nao") << ", ";
    std::cout << "P2: " << (allocated2 ? "sim" : "nao") << ", ";
    std::cout << "P3: " << (allocated3 ? "sim" : "nao") << "\n";

    std::cout << "P4 e P5 tentam alocar e devem bloquear\n";
    bool allocated4 = rm.allocate(p4);
    bool allocated5 = rm.allocate(p5);
    std::cout << "P4: " << (allocated4 ? "sim" : "nao") << ", ";
    std::cout << "P5: " << (allocated5 ? "sim" : "nao") << "\n";
    std::cout << "Bloqueados apos tentativas: " << scheduler.getBlockedQueue().size() << " (esperado: 2)\n";

    rm.printStatus();

    std::cout << "\nLiberando P1 (printer + scanner) e rechecando bloqueados\n";
    rm.release(1);
    rm.printStatus();
    std::cout << "Bloqueados apos release de P1: " << scheduler.getBlockedQueue().size() << " (esperado: 1)\n";

    std::cout << "\nLiberando P2 (modem) e rechecando bloqueados\n";
    rm.release(2);
    rm.printStatus();
    std::cout << "Bloqueados apos release de P2: " << scheduler.getBlockedQueue().size() << " (esperado: 0)\n";

    std::cout << "=== FIM TESTE 3 ===\n";
    return 0;
}