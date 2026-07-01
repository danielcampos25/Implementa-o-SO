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

    int printer; // 0 = nenhuma, 1 = impressora 1, 2 = impressora 2
    int scanner; // 0 = nenhum, 1 = scanner
    int modem;   // 0 = nenhum, 1 = modem
    int sata;    // 0 = nenhum, 1 = SATA 1, 2 = SATA 2
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