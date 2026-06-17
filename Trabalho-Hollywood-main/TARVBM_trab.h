#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct dados_das_folhas{
	char nome[150];
	char tipo[10];
	int ano;
	char frase[240];
  long offset_prim_viz;
}dados;

typedef struct arvbm{
  int nchaves, folha, *filho, prox;
  char **chave;
}TARVBM;


void libera_no(TARVBM *no, int t);
void TARVBM_cria(char *arq, int t);
TARVBM *TARVBM_busca1(char *arq, char *nome, int t);
TARVBM *TARVBM_busca(FILE *fp, int offset, char *nome, int t);
int TARVBM_insere(FILE *fp, int offset, char *nome, dados dado,int t);
char *divide_string(char **linha);
void TARVBM_imprime(FILE *fp, int offset, int t);
