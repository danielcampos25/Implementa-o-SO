# Implementação de Gerenciamento de Recursos e Scheduler

Este projeto demonstra um gerenciador simples de recursos de E/S e um scheduler que bloqueia e reavalia processos quando há disputa por dispositivos.

## Objetivo do Projeto

O projeto implementa um modelo de alocação de dispositivos de entrada/saída em que processos solicitam recursos e, caso algum recurso esteja ocupado, o processo é bloqueado.
Quando recursos são liberados, o scheduler tenta reavaliar os processos bloqueados e acordá-los se for possível.

## Estrutura do Projeto

- `ResourceManager/`
  - `ResourceManager.h` - Declarações da classe `ResourceManager` e estruturas auxiliares.
  - `ResourceManager.cpp` - Implementação do gerenciador de recursos.
  - `Resource.h` - Estrutura `ResourceRequest` que descreve uma requisição de E/S.

- `Scheduler/`
  - `scheduler.h` - Declarações da classe `Scheduler`, tipos de bloqueio e interfaces.
  - `scheduler.cpp` - Implementação do scheduler e da fila de processos bloqueados.

- `test_resource_scheduler.cpp` - Teste básico de alocação, bloqueio e liberação.
- `test_resource_scheduler2.cpp` - Teste de estresse com fila de bloqueio e reavaliação.
- `test_resource_scheduler3.cpp` - Teste de desbloqueio parcial e comportamento em cadeia.
- `Makefile` - Alvos de compilação e execução dos testes.

## Como o Sistema Funciona

### ResourceManager

O `ResourceManager` mantém o estado atual de cada dispositivo:

- `scannerOwner` - PID do processo dono do scanner
- `modemOwner` - PID do processo dono do modem
- `printerOwners` - array com os donos das duas impressoras
- `sataOwners` - array com os donos das duas portas SATA

As principais operações são:

- `canAllocate(const ResourceRequest &request)`
  - Verifica se os recursos solicitados estão disponíveis.
  - Retorna um struct `blockedBy` com a informação de quais recursos bloqueiam a requisição.
- `allocate(const ResourceRequest &req, bool canBlock = true)`
  - Tenta alocar os recursos solicitados.
  - Se algum recurso estiver ocupado, pode bloquear o processo no scheduler.
- `release(int pid)`
  - Libera todos os recursos que estavam associados ao processo.
  - Notifica o scheduler para reavaliar processos bloqueados.
- `printStatus()`
  - Exibe o estado atual dos recursos para depuração.

### Scheduler

O `Scheduler` gerencia a fila `blockedIO` de processos bloqueados por E/S.

- `blockProcess(const ResourceRequest &request, const blockedBy &reason)`
  - Insere um processo na fila de bloqueados junto com sua requisição original.
- `unblockProcess(int pid)`
  - Remove um processo específico da fila de bloqueio.
- `checkBlockedProcesses()`
  - Reavalia todos os processos bloqueados e tenta alocar seus recursos novamente.
  - Usa `allocate(..., false)` para evitar reinserir o processo na fila caso ainda não consiga alocar.

## Como Compilar e Executar

### Pré-requisitos

- Compilador C++ compatível com C++17 (por exemplo, `g++`)
- Make

### Passos

1. Abra o terminal na pasta do projeto:

   ```bash
   cd /home/daniel/os/Implementa-o-SO
   ```

2. Compile os testes:

   ```bash
   make
   ```

3. Execute um teste específico:

   ```bash
   ./test_resource_scheduler
   ./test_resource_scheduler2
   ./test_resource_scheduler3
   ```

4. Execute todos os testes em sequência:

   ```bash
   make run
   ```

5. Limpe os executáveis compilados:
   ```bash
   make clean
   ```

## O que Cada Teste Verifica

- `test_resource_scheduler.cpp`
  - Aloca recursos para um processo inicial.
  - Verifica bloqueio de um segundo processo.
  - Libera recursos e reavalia o processo bloqueado.
  - Testa alocação adicional após liberação.

- `test_resource_scheduler2.cpp`
  - Coloca dois processos em bloqueio simultâneo.
  - Libera um processo grande e verifica se a fila de bloqueados é esvaziada.

- `test_resource_scheduler3.cpp`
  - Valida desbloqueios parciais e reavaliação em cadeia.
  - Verifica comportamento com múltiplos recursos e diferentes combinações de requisição.

## Observações e Limitações

- A implementação atual trata cada recurso como exclusividade por processo.
- Impressoras e portas SATA têm duas unidades cada, mas o código aloca apenas uma instância por chamada de alocação.
- O scheduler não implementa prioridades ou ordem de espera completa além da fila FIFO.
- Ainda não há tratamento explícito de deadlock além do bloqueio/refenação natural de recursos.

## Estrutura de Recursos e Bloqueios

- `ResourceRequest` representa a requisição feita por um processo.
- `blockedBy` indica exatamente quais dispositivos estão impedindo a alocação.
- A relação `ResourceManager <-> Scheduler` permite:
  1. O `ResourceManager` detectar bloqueios.
  2. O `Scheduler` armazenar a requisição original.
  3. O `Scheduler` reavaliar o pedido quando recursos forem liberados.

## Sugestões de Extensões Futuras

- Adicionar prioridades entre processos bloqueados.
- Implementar deadlock detection e prevenção.
- Adicionar logs mais detalhados de alocação e desbloqueio.
- Incluir um módulo de execução de processos com estado READY/BLOCKED/RUNNING.

---

Este README documenta o projeto e a lógica atual. Ele pode ser ampliado com diagramas de estado, exemplos de uso e casos de teste adicionais se necessário.
