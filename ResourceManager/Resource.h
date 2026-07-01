#ifndef RESOURCE_H
#define RESOURCE_H

/*
 * ResourceRequest descreve a requisição de recursos de E/S de um processo.
 *
 * Cada processo informa o seu PID e quais dispositivos deseja usar.
 */
struct ResourceRequest
{
    int pid;

    bool printer;
    bool scanner;
    bool modem;
    bool sata;
};

/*
 * blockedBy indica quais recursos impediram a alocação.
 *
 * true  -> recurso indisponível
 * false -> recurso disponível
 */
struct blockedBy
{
    int pid;

    bool printer;
    bool scanner;
    bool modem;
    bool sata;
};

#endif