#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct dados_das_folhas{
	char nome[150];
	char tipo[10];
	int ano;
	char frase[240];
}dados;

typedef struct arvbm{
  int nchaves, folha, *filho, prox;
  char **chave;
  int *offset_chave, *id_folha;
}TARVBM;

void TARVBM_cria(char *arq, int t);
TARVBM *TARVBM_busca(char *arq, char *nome, int t);
int TARVBM_insere(FILE *fp, int offset, char *nome, int offset_arquivo, int id_folha, int t);
char *divide_string(char **linha);
void TARVBM_imprime(FILE *fp, int offset, int t);
