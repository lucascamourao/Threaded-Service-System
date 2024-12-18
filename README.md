# Sistema de Atendimento com Threads (C)

Este projeto implementa um sistema com múltiplas threads em C que simula um processo de atendimento a clientes. O programa possui três threads principais: **Recepção**, **Atendimento** e **Analista**. O sistema permite a criação de clientes com diferentes prioridades e os atende dentro de um tempo de paciência, enquanto registra as informações dos clientes atendidos em um arquivo.

## Funcionalidades

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

## Instruções para Configuração e Execução

### 1. Instalação do WSL
Se você estiver usando o Windows, é necessário instalar o **WSL (Windows Subsystem for Linux)** para compilar e executar o programa:

1. Abra o **PowerShell** como administrador e execute o seguinte comando:
   ```powershell
   wsl --install
   ```
   Isso instalará o WSL e a distribuição Ubuntu por padrão.

2. Reinicie o computador se solicitado.

3. Após reiniciar, abra o **Ubuntu** no menu Iniciar e siga as instruções para configurar um nome de usuário e senha.

### 2. Instalar Dependências no WSL
Certifique-se de que as ferramentas necessárias estão instaladas no WSL:

```bash
sudo apt update
sudo apt install build-essential
```

### 3. Navegar até o Diretório do Projeto
No WSL, navegue até o diretório onde os arquivos do projeto estão localizados. Por exemplo:

```bash
cd /mnt/c/Users/user/Desktop/TUDO/C_programming/SistemasOperacionais/SO
```

### 4. Compilar o Código
Use o seguinte comando para compilar os arquivos:

```bash
gcc -pthread -o atendimento main.c analista.c recepcao.c atendente.c fila.c
```

Isso gerará um arquivo executável chamado `atendimento`.

### 5. Executar o Programa
Para executar o programa, use:

```bash
./atendimento
```

Se o programa aceitar argumentos, passe-os como no exemplo abaixo:

```bash
./atendimento 10 1000
```

Se preferir, adicione um terminal Ubuntu (WSL) no próprio VS Code do Windows.

### 6. Limpeza dos Arquivos
Você pode usar o comando `rm` para remover os arquivos gerados, ou criar um **Makefile** para facilitar o processo. Consulte a seção de automação com `make` abaixo.

## Automação com Makefile
Se o projeto incluir um arquivo `Makefile`, você pode automatizar os passos de compilação e execução. Aqui estão os comandos mais comuns:

- **Compilar todos os programas**:
  ```bash
  make
  ```

- **Limpar arquivos temporários e executáveis**:
  ```bash
  make clean
  ```

- **Executar o programa**:
  ```bash
  make run
  ```

Certifique-se de que o `Makefile` esteja configurado corretamente no mesmo diretório dos arquivos fonte.
