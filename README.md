# Visualizador de Objetos 3D e Pipeline Gráfico com Qt

Este projeto é uma aplicação em C desenvolvida para simular o gerenciamento de salas e suas respectivas reservas. O sistema funciona via processamento em lote (batch), lendo um arquivo de texto com comandos sequenciais e registrando o histórico de execução em um arquivo de log.

## Funcionalidades Principais

O sistema lê um arquivo `.txt` de entrada. Os comandos suportados são:

*   `CRIAR_SALA <codigo> <capacidade>`: Cadastra uma nova sala no sistema.
*   `REMOVER_SALA <codigo>`: Exclui uma sala e limpa todas as suas reservas e status associados.
*   `RESERVAR_SALA <codigo> <prioridade>`: Cria uma reserva para uma sala, caso ela esteja disponível.
*   `BLOQUEAR_SALA <codigo>`: Impede novas reservas para a sala e cancela as atuais.
*   `DESBLOQUEAR_SALA <codigo>`: Torna uma sala bloqueada novamente disponível.
*   `LISTAR_SALAS`: Imprime no log as salas cadastradas em ordem (percurso *in-order* na AVL).
*   `LISTAR_RESERVAS`: Imprime todas as reservas atuais contidas no Heap.

## Detalhes Técnicos

Para garantir o melhor desempenho na gestão dos dados, o projeto implementa três estruturas fundamentais do zero:

*   **Árvore AVL:** Armazena o cadastro das salas (código e capacidade). A propriedade de auto-balanceamento garante que a inserção, remoção e busca de salas ocorram em complexidade de tempo $O(\log n)$.
*   **Fila de Prioridade (Max-Heap):** Gerencia a fila de reservas ativas. O Max-Heap garante que a extração da reserva com a **maior prioridade** ocorra em $O(\log n)$.
*   **Tabela Hash:** Armazena o **estado** atual de cada sala (ex: `Disponivel`, `Reservado`, `Bloqueado`), utilizando encadeamento externo (lista encadeada) para o tratamento de colisões. A busca pelo estado da sala ocorre em tempo médio $O(1)$.

## Tecnologias Utilizadas

* **Linguagem:** C
* **IDE:** CodeBlocks 20.03
* **Compilador:** MinGW-w64
