# Sistema de Atendimento com Threads (C)

Este projeto implementa um sistema com múltiplas threads em C que simula um processo de atendimento a clientes. O programa possui três threads principais: **Recepção**, **Atendimento**, e **Analista**. O sistema permite a criação de clientes com diferentes prioridades e os atende dentro de um tempo de paciência, enquanto registra as informações dos clientes atendidos em um arquivo.

## Funcionalidade

O sistema é dividido em três módulos principais:

1. **Recepção**: 
    - Cria clientes com duas prioridades possíveis: alta e baixa.
    - Insere os clientes em uma fila de atendimento.
    - A cada criação de cliente, a prioridade é atribuída aleatoriamente com 50% de chance de ser alta ou baixa.

2. **Atendimento**:
    - Retira clientes da fila e realiza o atendimento.
    - Os clientes de prioridade alta têm um tempo de paciência mais curto (X / 2 ms), enquanto os de baixa prioridade têm um tempo de paciência maior (X ms).
    - O atendimento é considerado bem-sucedido se o cliente for atendido dentro do seu tempo de paciência.

3. **Analista**:
    - Lê um arquivo de texto que contém os PIDs dos clientes atendidos.
    - Imprime os 10 primeiros PIDs de cada vez, limpando o arquivo após a leitura.

O sistema termina quando todos os clientes forem atendidos (se N for um valor específico) ou quando o usuário pressionar a tecla "s" (se N for 0, ou seja, clientes infinitos).

## Estrutura do Projeto

- **main.c**: Arquivo principal com a lógica das threads e sincronização.
- **analista.c**: Lógica da thread do Analista que imprime os PIDs dos clientes atendidos.
- **recepcao.c**: Lógica da thread de Recepção, que cria os clientes e os coloca na fila.
- **atendente.c**: Lógica da thread de Atendimento, que retira os clientes da fila e realiza o atendimento.
- **fila.c**: Implementação da fila de atendimento e controle de acesso com mutex.
- **pids.txt**: Arquivo utilizado para armazenar os PIDs dos clientes atendidos.

## Como Funciona

### Fluxo Geral
1. O processo **Recepção** cria os clientes e os coloca na fila de atendimento.
2. O processo **Atendimento** retira os clientes da fila, realiza o atendimento e registra os PIDs em um arquivo.
3. O processo **Analista** lê o arquivo e imprime os PIDs dos clientes atendidos.

### Como o Programa Lida com a Prioridade
- **Prioridade alta**: Clientes com paciência X / 2 ms.
- **Prioridade baixa**: Clientes com paciência X ms.
- A prioridade dos clientes é atribuída aleatoriamente (50% para cada tipo).

### Saída Final
- Quando o atendimento for concluído, o programa calcula e exibe a taxa de satisfação (clientes atendidos no tempo determinado).
- O processo "Analista" imprime os primeiros 10 PIDs dos clientes atendidos a cada iteração e os apaga do arquivo.

## Compilação e Execução

1. **Compilar o código**:
   Para compilar o programa, use o seguinte comando:
   ```bash
   gcc -o atendimento main.c analista.c recepcao.c atendente.c fila.c -lpthread
