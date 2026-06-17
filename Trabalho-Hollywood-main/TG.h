#include <stdlib.h>
#include <stdio.h>

typedef struct viz {
	int id_viz;
	struct viz *prox_viz;
}TVIZ;

typedef struct grafo{
	int id_no;
	TVIZ *prim_viz;
	struct grafo *prox_no;
}TG;

typedef struct {
	char nome_destino[150];
	char tipo_relacao[20];
	char papel[100];
	long offset_prox_viz;
} Relacionamentos;

void insere_aresta_disco(FILE *arq_indices, FILE *arq_grafo, int offset_raiz, char *origem, char *destino, char *rel, char *papel, int t);
void TG_ins_aresta(TG *g, int no1, int no2);
void brincar_com_grafo(FILE *arq_indices, int offset_raiz, char *nome_busca, int t);
TG* TG_inicializa();
void TG_imprime(TG *g);
void TG_imp_rec(TG *g);
void TG_libera(TG *g);
TG* TG_busca_no(TG* g, int x);
TVIZ* TG_busca_aresta(TG *g, int no1, int no2);
TG* TG_ins_no(TG *g, int x);
void TG_ins_aresta(TG *g, int no1, int no2);
void TG_retira_aresta(TG *g ,int no1, int no2);
TG* TG_retira_no(TG *g, int no);
