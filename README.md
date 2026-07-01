# Implementação de Gerenciamento de Recursos e ProcessScheduler

Este projeto demonstra um gerenciador simples de recursos de E/S integrado ao `ProcessScheduler`, que escalona CPU, bloqueia processos por disputa de dispositivos e reavalia bloqueados quando recursos são liberados.

## Objetivo do Projeto

O projeto implementa um modelo de alocação de dispositivos de entrada/saída em que processos solicitam recursos e, caso algum recurso esteja ocupado, o processo é bloqueado.
Quando recursos são liberados, o `ProcessScheduler` tenta reavaliar os processos bloqueados e recolocá-los na fila `READY` se for possível.

## Estrutura do Projeto

- `ResourceManager/`
  - `ResourceManager.h` - Declarações da classe `ResourceManager` e estruturas auxiliares.
  - `ResourceManager.cpp` - Implementação do gerenciador de recursos.
  - `Resource.h` - Estruturas `ResourceRequest` e `blockedBy`.

- `ProcessScheduler/`
  - `ProcessScheduler.h` - Declarações do escalonador de CPU e da fila de bloqueados por E/S.
  - `ProcessScheduler.cpp` - Implementação das filas `READY`, aging, quantum e reavaliação de bloqueados.

- `test_resource_scheduler.cpp` - Teste básico de alocação, bloqueio e liberação.
- `test_resource_scheduler2.cpp` - Teste de estresse com fila de bloqueio e reavaliação.
- `test_resource_scheduler3.cpp` - Teste de desbloqueio parcial e comportamento em cadeia.
- `test_process_scheduler.cpp` - Testes do modelo de processo, filas prontas e transições `READY/BLOCKED`.
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
  - Se algum recurso estiver ocupado, pode bloquear o processo no `ProcessScheduler`.
- `release(int pid)`
  - Libera todos os recursos que estavam associados ao processo.
  - Notifica o `ProcessScheduler` para reavaliar processos bloqueados.
- `printStatus()`
  - Exibe o estado atual dos recursos para depuração.

### ProcessScheduler

O `ProcessScheduler` gerencia as filas `READY` por prioridade, a execução de CPU e a fila `blockedIO` de processos bloqueados por E/S.

- `blockProcess(const ResourceRequest &request, const blockedBy &reason)`
  - Marca o processo como `Blocked`, remove o PID das filas `READY` e guarda a requisição original na fila de bloqueados.
- `unblockProcess(int pid)`
  - Remove um processo específico da fila de bloqueio.
- `checkBlockedProcesses()`
  - Reavalia todos os processos bloqueados e tenta alocar seus recursos novamente.
  - Usa `allocate(..., false)` para evitar reinserir o processo na fila caso ainda não consiga alocar.
  - Quando a realocação funciona, marca o processo como `Ready` e o reinsere na fila pronta correta.

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
- O desbloqueio por E/S segue a ordem FIFO da fila de bloqueados.
- Ainda não há tratamento explícito de deadlock além do bloqueio/reavaliação natural de recursos.

## Estrutura de Recursos e Bloqueios

- `ResourceRequest` representa a requisição feita por um processo.
- `blockedBy` indica exatamente quais dispositivos estão impedindo a alocação.
- A relação `ResourceManager <-> ProcessScheduler` permite:
  1. O `ResourceManager` detectar bloqueios.
  2. O `ProcessScheduler` armazenar a requisição original.
  3. O `ProcessScheduler` reavaliar o pedido quando recursos forem liberados.

## Sugestões de Extensões Futuras

- Adicionar prioridades entre processos bloqueados.
- Implementar deadlock detection e prevenção.
- Adicionar logs mais detalhados de alocação e desbloqueio.
- Expandir a integração entre recursos de E/S e os eventos do escalonador.

## Módulo de Processos e Filas Prontas

O `ProcessScheduler` executa uma simulação sequencial de processos prontos para CPU e concentra a fila de bloqueados por E/S usada pelo `ResourceManager`.

Arquivos adicionados:

- `Process/Process.h`
- `Process/Process.cpp`
- `ProcessScheduler/ProcessScheduler.h`
- `ProcessScheduler/ProcessScheduler.cpp`
- `test_process_scheduler.cpp`

Comportamentos implementados:

- PIDs sequenciais iniciando em 0.
- Processo de tempo real com prioridade 0.
- Processos de usuário com prioridades 1, 2 e 3.
- Fila de tempo real FIFO.
- Seleção de CPU na ordem: tempo real, usuário 1, usuário 2, usuário 3.
- Tempo real executa até finalizar, sem preempção.
- Usuário executa quantum de 1 unidade por despacho.
- Processos de usuário inacabados retornam para a fila pronta.
- Processos bloqueados por E/S saem das filas prontas e voltam para a prioridade correta quando seus recursos são realocados.
- Aging após 5 ciclos de espera: prioridade 3 sobe para 2, prioridade 2 sobe para 1.
- Limite de 1000 processos aceitos pelo escalonador.
- Processos com tempo total de CPU zero são finalizados sem despacho.

### Como validar o novo módulo

Compile todos os testes:

```bash
make
```

Execute somente o teste de processos e filas:

```bash
./test_process_scheduler
```

Execute todos os testes, incluindo recursos, arquivos e processos:

```bash
make run
```

Saída observada para o teste novo:

```text
===== ProcessScheduler Module Tests =====
All process scheduler tests passed.
```

Comparação com os critérios de aceite:

- Prioridade 0 executa antes dos usuários: coberto por `testRealTimeFifoAndUserPriority`.
- Dois processos de tempo real seguem FIFO: coberto por `testRealTimeFifoAndUserPriority`.
- Tempo real não sofre preempção: coberto por `testCpuSimulation`.
- Usuário consome no máximo 1 unidade por despacho: coberto por `testCpuSimulation`.
- Prioridade de usuário respeita 1 antes de 2 e 2 antes de 3: coberto por `testRealTimeFifoAndUserPriority`.
- Aging promove prioridades 3 para 2 e 2 para 1 após 5 ciclos: coberto por `testAging`.
- Mais de 1000 processos é rejeitado: coberto por `testCapacityLimit`.
- Testes existentes de `ResourceManager/`, `ProcessScheduler/` e `FileSystem/` continuam no alvo `make run`.

Limitações:

- `startTime` é armazenado no processo, mas admissão atrasada por tempo de inicialização fica para integração futura com parser/dispatcher.
- Memória e sistema de arquivos não são reimplementados por este módulo.

Próximos passos recomendados:

- Integrar o `ProcessScheduler` ao dispatcher principal quando o parser de entrada estiver definido.
- Conectar eventos de bloqueio/desbloqueio ao dispatcher principal.

## Entrega 2026/1

### Requisitos

- Compilador C++ com suporte a C++17.
- `make` disponível no ambiente.
- Três arquivos de entrada para a execução oficial: `processes.txt`, `files.txt` e `string.txt`.

### Como Compilar

```bash
make clean
make all
```

### Como Executar

```bash
make run
make smoke
```

Exemplo de comando direto:

```bash
./bin/g2_OS tests/input/processes_delivery.txt tests/objects/files_delivery.txt tests/input/string_delivery.txt
```

### Formato dos Arquivos de Entrada

- `processes.txt`: cada linha contém `startTime priority processorTime memoryBlocks printerRequest scannerRequest modemRequest sataDiskRequest`.
- `files.txt`: define blocos totais, segmentos iniciais e operações `create/delete` associadas a PIDs.
- `string.txt`: contém uma string de referência por processo, na mesma ordem de `processes.txt`.

### O Que a Saída Mostra

- Eventos da simulação do escalonador.
- `Número de Faltas de Páginas por processo:`
- `Sistema de arquivos =>`
- `Mapa de ocupação do disco:`

### Módulos Implementados

- Escalonamento de processos.
- Memória virtual com page faults.
- Sistema de arquivos com permissões por prioridade.

### Observação Sobre IA

Este projeto foi estruturado e documentado com apoio de Codex e Spec Kit para organização da entrega e dos artefatos de validação.

## Módulo de Carregamento de Entrada de Processos

Esta feature adiciona o módulo `ProcessInput/`, responsável por carregar um arquivo `processes.txt` e transformar cada linha válida em uma entrada `ProcessWorkloadEntry` para uso posterior pelo `Dispatcher`.

Formato esperado por linha:

```text
startTime priority processorTime memoryBlocks printerRequest scannerRequest modemRequest sataDiskRequest
```

Comportamentos implementados:

- Leitura linha por linha de um arquivo de processos.
- Ignora linhas vazias ou apenas com espaços.
- Aceita espaços extras entre campos.
- Exige exatamente 8 campos por linha não vazia.
- Converte todos os campos para inteiros.
- Preserva a ordem original das linhas válidas.
- Retorna erro observável para arquivo inexistente, falha de leitura, quantidade incorreta de colunas e campo numérico inválido.
- Não cria `Process`, não admite processos, não executa o escalonador e não valida regras de domínio como prioridade, quantum, aging, limite de processos, memória ou recursos.

### Como validar o carregador

Compile todos os testes:

```bash
make all
```

Execute somente o teste do carregador:

```bash
./test_process_input_loader
```

Execute todos os testes:

```bash
make run
```

Saída observada para o teste do carregador:

```text
===== Process Input Loader Tests =====
All process input loader tests passed.
```

Arquivos de exemplo usados nos testes:

- `tests/input/processes_valid.txt`
- `tests/input/processes_blank_lines.txt`
- `tests/input/processes_same_start_time.txt`
- `tests/input/processes_missing_column.txt`
- `tests/input/processes_extra_column.txt`
- `tests/input/processes_invalid_number.txt`

## Fluxo Principal do Dispatcher

O executável principal conecta o arquivo de processos ao carregador, ao dispatcher e ao escalonador de processos.

Compile o executável principal:

```bash
make dispatcher
```

Execute a simulação com um arquivo de processos:

```bash
./dispatcher tests/input/processes_valid.txt
```

Formato esperado do arquivo:

```text
startTime priority processorTime memoryBlocks printerRequest scannerRequest modemRequest sataDiskRequest
```

A saída é composta pelos eventos do dispatcher, por exemplo:

```text
cycle=0 type=admission pid=0 priority=1 remaining=2 message=Processo admitido
cycle=1 type=dispatch pid=0 priority=1 remaining=2 message=Processo despachado
cycle=1 type=quantum pid=0 priority=1 remaining=1 message=Processo de usuario consumiu quantum
```

Se o comando for chamado sem exatamente um argumento, ele exibe:

```text
Uso: ./dispatcher <processes.txt>
```

O alvo `make run` continua executando os testes existentes do projeto.

---

Este README documenta o projeto e a lógica atual. Ele pode ser ampliado com diagramas de estado, exemplos de uso e casos de teste adicionais se necessário.
