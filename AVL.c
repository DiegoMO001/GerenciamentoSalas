#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define M 8

typedef struct sala
{
	int codigo_sala;
	int capacidade_sala;
	struct sala *esq;
	struct sala *dir;
	int altura;
} Sala;

typedef struct reserva
{
    int codigo_sala;
    int prioridade;
} Reserva;

typedef struct heap
{
    Reserva *vetor;
    int tamanho_heap;
    int capacidade_heap;
} Heap;

typedef struct hash
{
    int codigo_hash;
    char estado[30];
    struct hash *proximo;
} Hash;

typedef struct
{
	Hash *ini;
} ListaHash;

// Funções da Árvore AVL
Sala* criarSala(int codigo_sala, int capacidade_sala);
int calcularAltura(Sala *p);
int calcularMaior(int a, int b);
int calcularFB(Sala *p);
Sala *balanceamento(Sala *p);
Sala* rotacaoEsquerda(Sala *p);
Sala* rotacaoDireita(Sala *p);
Sala* inserirSala(Sala *raiz, int codigo_sala, int capacidade_sala);
Sala* removerSala(Sala *raiz, int codigo_sala);
Sala* buscarSala(Sala *raiz, int codigo_sala);
void listarSalas(Sala *raiz, FILE *arqlog);
void freeAVL(Sala *raiz);

// Funções da Heap
Heap* criarHeap(int capacidade_heap);
void trocarReserva(Reserva* r1, Reserva* r2);
void heapify(Heap* heap, int i);
void adicionarReserva(Heap* heap, int codigo_sala, int prioridade);
void listarReservas(Heap* heap, FILE *arqlog);
Reserva removerHeap(Heap* heap);
void freeHeap(Heap* heap);
void removerReservaDoHeap(Heap *heap, int codigo_sala);

// Funções da Tabela Hash
void inicializarHash(ListaHash p[]);
int funcaoHash(int codigo);
void inserirNaHash(ListaHash p[], int codigo_sala, char* estado);
Hash* buscarNaHash(ListaHash p[], int codigo_sala);
void atualizarEstado(Hash* hash_node, char* novoEstado);
void removerDaHash(ListaHash p[], int codigo_sala);
void freeHash(ListaHash p[]);

// Função de Integração
void cancelarTudo(Sala **raiz, Heap *heap, ListaHash p[], int codigo_sala);

int main()
{
	Sala *raiz = NULL;
	Heap *heap = criarHeap(10);
	ListaHash p[M];

    if (heap == NULL) 
    {
        printf("Falha critica: Nao foi possivel alocar memoria para o sistema de reservas (Heap).\n");
        return 1;
    }

	inicializarHash(p);

	char op[30], nomeAr[30];
	int codigo, capacidade, prioridade;

	FILE *arqlog = fopen("saida.log","w");
	if(!arqlog) 
    {
		printf("Erro ao criar o arquivo de log.\n");
		return 1;
	}

	printf("\nDigite o nome do arquivo de entrada (com .txt): ");
    scanf("%s", nomeAr);

    FILE* arq = fopen(nomeAr, "rt");
    if (!arq)
    {
        printf("\nFalha ao abrir o arquivo de entrada '%s'\n", nomeAr);
        fprintf(arqlog,"Falha ao abrir o arquivo de entrada '%s'\n", nomeAr);
		fclose(arqlog);
		return 1;
	}

	fprintf(arqlog, "--- INICIO DO PROCESSAMENTO ---\n");

	while(fscanf(arq, "%s", op) != EOF)
	{
		if (strcmp(op,"CRIAR_SALA") == 0)
        {
            fscanf(arq,"%d %d", &codigo, &capacidade);
    
            if (buscarSala(raiz, codigo) != NULL) 
            {
                fprintf(arqlog, "\n[ERRO] Falha ao criar: Sala com codigo %d ja existe.\n", codigo);
                printf("Erro: Sala com codigo %d ja existe.\n", codigo);
            } else 
            {
                raiz = inserirSala(raiz, codigo, capacidade);
                inserirNaHash(p, codigo, "Disponivel");
                fprintf(arqlog,"\n[COMANDO: CRIAR_SALA] Sala %d (Cap: %d) criada.\n", codigo, capacidade);
                printf("-> Sala %d criada.\n", codigo);
            }
        }
		else if (strcmp(op, "REMOVER_SALA") == 0)
        {
            fscanf(arq, "%d", &codigo);
            cancelarTudo(&raiz, heap, p, codigo);
            fprintf(arqlog, "\n[COMANDO: REMOVER_SALA] Sala %d e quaisquer reservas associadas foram removidas.\n", codigo);
            printf("-> Sala %d removida de todos os sistemas.\n", codigo);
        }
		else if (strcmp(op, "RESERVAR_SALA") == 0)
		{
			fscanf(arq, "%d %d", &codigo, &prioridade);
			Hash* sala_hash = buscarNaHash(p, codigo);
			if (sala_hash && strcmp(sala_hash->estado, "Disponivel") == 0) 
            {
				adicionarReserva(heap, codigo, prioridade);
				atualizarEstado(sala_hash, "Reservado");
				fprintf(arqlog, "\n[COMANDO: RESERVAR_SALA] Sala %d reservada com prioridade %d.\n", codigo, prioridade);
				printf("-> Sala %d reservada.\n", codigo);
			} 
            else if (sala_hash) 
            {
				fprintf(arqlog, "\n[COMANDO: RESERVAR_SALA] Falha: Sala %d nao esta disponivel (Estado: %s).\n", codigo, sala_hash->estado);
				printf("-> Falha ao reservar sala %d: nao esta disponivel.\n", codigo);
			} else 
            {
				fprintf(arqlog, "\n[COMANDO: RESERVAR_SALA] Falha: Sala %d nao encontrada.\n", codigo);
				printf("-> Falha ao reservar sala %d: nao encontrada.\n", codigo);
			}
		}
		else if (strcmp(op, "BLOQUEAR_SALA") == 0)
        {
            fscanf(arq, "%d", &codigo);
            Hash* sala_hash = buscarNaHash(p, codigo);
            if (sala_hash) 
            {
                atualizarEstado(sala_hash, "Bloqueado");
                removerReservaDoHeap(heap, codigo);
                fprintf(arqlog, "\n[COMANDO: BLOQUEAR_SALA] Sala %d bloqueada.\n", codigo);
                printf("-> Sala %d bloqueada.\n", codigo);
            } else 
            {
                fprintf(arqlog, "\n[COMANDO: BLOQUEAR_SALA] Falha: Sala %d nao encontrada.\n", codigo);
				printf("-> Falha ao bloquear sala %d: nao encontrada.\n", codigo);
            }
        }
        else if (strcmp(op, "DESBLOQUEAR_SALA") == 0)
        {
            fscanf(arq, "%d", &codigo);
            Hash* sala_hash = buscarNaHash(p, codigo);
            if (sala_hash) 
            {
                atualizarEstado(sala_hash, "Disponivel");
                fprintf(arqlog, "\n[COMANDO: DESBLOQUEAR_SALA] Sala %d desbloqueada e agora esta disponivel.\n", codigo);
                printf("-> Sala %d desbloqueada.\n", codigo);
            } else 
            {
                fprintf(arqlog, "\n[COMANDO: DESBLOQUEAR_SALA] Falha: Sala %d nao encontrada.\n", codigo);
				printf("-> Falha ao desbloquear sala %d: nao encontrada.\n", codigo);
            }
        }
		else if (strcmp(op, "LISTAR_SALAS") == 0)
		{
			fprintf(arqlog, "\n[COMANDO: LISTAR_SALAS]\n--- Lista de Todas as Salas ---\n");
			printf("\n--- Lista de Salas (ver saida.log) ---\n");
			if(raiz == NULL) {
				fprintf(arqlog, "Nenhuma sala cadastrada.\n");
			} else 
            {
				listarSalas(raiz, arqlog);
			}
			fprintf(arqlog, "-----------------------------\n");
		}
		else if (strcmp(op, "LISTAR_RESERVAS") == 0)
		{
			fprintf(arqlog, "\n[COMANDO: LISTAR_RESERVAS]\n--- Fila de Prioridade de Reservas ---\n");
			printf("\n--- Lista de Reservas (ver saida.log) ---\n");
			listarReservas(heap, arqlog);
			fprintf(arqlog, "-------------------------------------\n");
		} else 
        {
			char buffer[100];
            fgets(buffer, 100, arq);
            fprintf(arqlog, "\n[ERRO] Comando desconhecido: '%s'\n", op);
            printf("Comando desconhecido: '%s'\n", op);
		}
	}

	fprintf(arqlog, "\n--- FIM DO PROCESSAMENTO ---\n");
	printf("\nProcessamento concluido. Verifique o arquivo 'saida.log' para detalhes.\n");

	fclose(arq);
	fclose(arqlog);
	freeAVL(raiz);
	freeHeap(heap);
	freeHash(p);

	return 0;
}

Sala* criarSala(int codigo_sala, int capacidade_sala) 
{
	Sala *novo = (Sala *)malloc(sizeof(Sala));
	if(!novo) return NULL;
	novo->codigo_sala = codigo_sala;
	novo->capacidade_sala = capacidade_sala;
	novo->esq = NULL;
	novo->dir =  NULL;
	novo->altura = 0;
	return novo;
}
int calcularAltura(Sala *p) { return (p == NULL) ? -1 : p->altura; }
int calcularMaior(int a, int b) { return (a > b) ? a : b; }
int calcularFB(Sala *p) { return (p == NULL) ? 0 : (calcularAltura(p->dir) - calcularAltura(p->esq)); }

Sala *rotacaoEsquerda(Sala *p) 
{
    Sala *x = p->dir;
    Sala *y = x->esq;
    x->esq = p;
    p->dir = y;
    p->altura = calcularMaior(calcularAltura(p->esq), calcularAltura(p->dir)) + 1;
    x->altura = calcularMaior(calcularAltura(x->esq), calcularAltura(x->dir)) + 1;
    return x;
}

Sala *rotacaoDireita(Sala *p) 
{
    Sala *x = p->esq;
    Sala *y = x->dir;
    x->dir = p;
    p->esq = y;
    p->altura = calcularMaior(calcularAltura(p->esq), calcularAltura(p->dir)) + 1;
    x->altura = calcularMaior(calcularAltura(x->esq), calcularAltura(x->dir)) + 1;
    return x;
}

Sala *balanceamento(Sala *p) 
{
	int fb = calcularFB(p);
	if (fb > 1) 
    {
        if (calcularFB(p->dir) < 0) 
        {
            p->dir = rotacaoDireita(p->dir);
        }
        p = rotacaoEsquerda(p);
    } 
    else if (fb < -1) 
    {
        if (calcularFB(p->esq) > 0) 
        {
            p->esq = rotacaoEsquerda(p->esq);
        }
        p = rotacaoDireita(p);
    }
	return p;
}

Sala* buscarSala(Sala *raiz, int codigo_sala) 
{
    if (raiz == NULL || raiz->codigo_sala == codigo_sala) 
    {
        return raiz;
    }

    if (codigo_sala < raiz->codigo_sala) 
    {
        return buscarSala(raiz->esq, codigo_sala);
    }

    return buscarSala(raiz->dir, codigo_sala);
}

Sala* inserirSala(Sala *raiz, int codigo_sala, int capacidade_sala) 
{
	if(raiz == NULL) return criarSala(codigo_sala, capacidade_sala);
	
	if (codigo_sala < raiz->codigo_sala)
		raiz->esq = inserirSala(raiz->esq, codigo_sala, capacidade_sala);
	else if (codigo_sala > raiz->codigo_sala)
		raiz->dir = inserirSala(raiz->dir, codigo_sala, capacidade_sala);
	else
		printf("Erro: Sala com codigo %d ja existe.\n", codigo_sala);

	raiz->altura = calcularMaior(calcularAltura(raiz->esq), calcularAltura(raiz->dir)) + 1;
	return balanceamento(raiz);
}

Sala* removerSala(Sala *raiz, int codigo_sala) 
{
    if (raiz == NULL) return NULL;

    if (codigo_sala < raiz->codigo_sala) 
    {
        raiz->esq = removerSala(raiz->esq, codigo_sala);
    } 
    else if (codigo_sala > raiz->codigo_sala) 
    {
        raiz->dir = removerSala(raiz->dir, codigo_sala);
    } 
    else 
    {
        if (raiz->esq == NULL || raiz->dir == NULL) 
        { 
            Sala *temp = raiz->esq ? raiz->esq : raiz->dir;
            if (temp == NULL) 
            {
                temp = raiz;
                raiz = NULL;
            } else 
            {
                *raiz = *temp;
            }
            free(temp);
        } else 
        {
            Sala *temp = raiz->esq;
            while (temp->dir != NULL) temp = temp->dir;
            raiz->codigo_sala = temp->codigo_sala;
            raiz->capacidade_sala = temp->capacidade_sala;
            raiz->esq = removerSala(raiz->esq, temp->codigo_sala);
        }
    }
    if (raiz == NULL) return raiz;
    raiz->altura = calcularMaior(calcularAltura(raiz->esq), calcularAltura(raiz->dir)) + 1;
    return balanceamento(raiz);
}

void listarSalas(Sala *raiz, FILE *arqlog) 
{
    if (raiz != NULL) 
    {
        listarSalas(raiz->esq, arqlog);
        fprintf(arqlog, " -> Sala: %d | Capacidade: %d\n", raiz->codigo_sala, raiz->capacidade_sala);
        listarSalas(raiz->dir, arqlog);
    }
}
void freeAVL(Sala *raiz) 
{
    if (raiz == NULL) return;
    freeAVL(raiz->esq);
    freeAVL(raiz->dir);
    free(raiz);
}

Heap* criarHeap(int capacidade_heap) 
{
    Heap* heap = (Heap*) malloc(sizeof(Heap));
    if (heap == NULL) 
    {
        printf("Erro: Falha ao alocar memoria para a Heap.\n");
        return NULL;
    }

    heap->vetor = (Reserva*) malloc(capacidade_heap * sizeof(Reserva));
    if (heap->vetor == NULL) 
    {
        printf("Erro: Falha ao alocar memoria para o vetor da Heap.\n");
        free(heap);
        return NULL;
    }

    heap->tamanho_heap = 0;
    heap->capacidade_heap = capacidade_heap;
    return heap;
}

void trocarReserva(Reserva* r1, Reserva* r2) 
{
    Reserva aux = *r1;
    *r1 = *r2;
    *r2 = aux;
}

void heapify(Heap* heap, int i) 
{
    int maior = i;
    int esq = 2 * i + 1;
    int dir = 2 * i + 2;

    if (esq < heap->tamanho_heap && heap->vetor[esq].prioridade > heap->vetor[maior].prioridade)
        maior = esq;

    if (dir < heap->tamanho_heap && heap->vetor[dir].prioridade > heap->vetor[maior].prioridade)
        maior = dir;

    if (maior != i) 
    {
        trocarReserva(&heap->vetor[i], &heap->vetor[maior]);
        heapify(heap, maior);
    }
}

void adicionarReserva(Heap* heap, int codigo_sala, int prioridade) 
{
    if (heap->tamanho_heap == heap->capacidade_heap) 
    {
        printf("Heap cheio!\n");
        return;
    }

    int i = heap->tamanho_heap;
    heap->vetor[i].codigo_sala = codigo_sala;
    heap->vetor[i].prioridade = prioridade;
    heap->tamanho_heap++;

    while (i != 0 && heap->vetor[(i - 1) / 2].prioridade < heap->vetor[i].prioridade)
    {
       trocarReserva(&heap->vetor[i], &heap->vetor[(i - 1) / 2]);
       i = (i - 1) / 2;
    }
}

void listarReservas(Heap* heap, FILE *arqlog) 
{
    if (heap->tamanho_heap == 0) {
        fprintf(arqlog, "Nenhuma reserva ativa.\n");
    } else
    {
        for (int i = 0; i < heap->tamanho_heap; i++) {
            fprintf(arqlog, " -> Sala: %d | Prioridade: %d\n", heap->vetor[i].codigo_sala, heap->vetor[i].prioridade);
        }
    }
}

void removerReservaDoHeap(Heap *heap, int codigo_sala) 
{
    int i;
    for (i = 0; i < heap->tamanho_heap; i++) 
    {
        if (heap->vetor[i].codigo_sala == codigo_sala) 
        {
            break;
        }
    }

    if (i < heap->tamanho_heap) 
    {
        heap->vetor[i] = heap->vetor[heap->tamanho_heap - 1];
        heap->tamanho_heap--;

        int pai = (i - 1) / 2;
        if (i > 0 && heap->vetor[i].prioridade > heap->vetor[pai].prioridade) 
        {
            while (i != 0 && heap->vetor[(i - 1) / 2].prioridade < heap->vetor[i].prioridade) 
            {
                trocarReserva(&heap->vetor[i], &heap->vetor[(i - 1) / 2]);
                i = (i - 1) / 2;
            }
        } else 
        {
            heapify(heap, i);
        }
    }
}

Reserva removerHeap(Heap* heap) 
{
    Reserva reservaVazia = {-1, -1}; 

    if (heap->tamanho_heap <= 0) 
    {
        printf("Erro: Não há reservas no heap!\n");
        return reservaVazia;
    }

    if (heap->tamanho_heap == 1) 
    {
        heap->tamanho_heap--;
        return heap->vetor[0];
    }

    Reserva raiz = heap->vetor[0];

    heap->vetor[0] = heap->vetor[heap->tamanho_heap - 1];
    heap->tamanho_heap--;

    heapify(heap, 0);

    return raiz;
}

void freeHeap(Heap* heap)
{
    free(heap->vetor);
    free(heap);
}

void inicializarHash(ListaHash p[]) 
{
    for (int i = 0; i < M; i++)
    {
        p[i].ini = NULL;
    }
}

int funcaoHash(int codigo) { return codigo % M; }

void inserirNaHash(ListaHash p[], int codigo_sala, char* estado) 
{
    int pos = funcaoHash(codigo_sala);
    Hash* novo = (Hash*)malloc(sizeof(Hash));
    novo->codigo_hash = codigo_sala;
    strcpy(novo->estado, estado);
    novo->proximo = p[pos].ini;
    p[pos].ini = novo;
}

Hash* buscarNaHash(ListaHash p[], int codigo_sala) 
{
    int pos = funcaoHash(codigo_sala);
    Hash *aux = p[pos].ini;
    while(aux != NULL) 
    {
        if (aux->codigo_hash == codigo_sala) 
        {
            return aux;
        }
        aux = aux->proximo;
    }
    return NULL;
}

void atualizarEstado(Hash* hash_node, char* novoEstado) 
{
    if (hash_node != NULL) 
    {
        strcpy(hash_node->estado, novoEstado);
    }
}

void removerDaHash(ListaHash p[], int codigo_sala) 
{
    int pos = funcaoHash(codigo_sala);
    Hash *atual = p[pos].ini;
    Hash *anterior = NULL;

    while (atual != NULL && atual->codigo_hash != codigo_sala) 
    {
        anterior = atual;
        atual = atual->proximo;
    }

    if (atual == NULL) return;

    if (anterior == NULL) 
    {
        p[pos].ini = atual->proximo;
    } else {
        anterior->proximo = atual->proximo;
    }
    free(atual);
}

void freeHash(ListaHash p[]) 
{
    for (int i = 0; i < M; i++) 
    {
        Hash *aux = p[i].ini;
        while (aux != NULL) 
        {
            Hash *temp = aux;
            aux = aux->proximo;
            free(temp);
        }
    }
}

void cancelarTudo(Sala **raiz, Heap *heap, ListaHash p[], int codigo_sala) 
{
    if (buscarSala(*raiz, codigo_sala) == NULL) 
    {
        return; 
    }

    *raiz = removerSala(*raiz, codigo_sala);
    removerReservaDoHeap(heap, codigo_sala);
    removerDaHash(p, codigo_sala);
}