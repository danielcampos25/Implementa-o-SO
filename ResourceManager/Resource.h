#ifndef RESOURCE_H
#define RESOURCE_H

/*
 * ResourceRequest descreve a requisição de recursos de E/S de um processo.
 *
 * Cada processo informa o seu PID e quais dispositivos deseja usar.
 */
struct ResourceRequest
{
    int pid; // Identificador do processo

    bool printer; // Requisição de impressora
    bool scanner; // Requisição de scanner
    bool modem;   // Requisição de modem
    bool sata;    // Requisição de disco SATA
};

#endif