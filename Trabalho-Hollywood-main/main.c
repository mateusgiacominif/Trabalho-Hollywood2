#include "TARVBM_trab.h"
#define N 210

int main(void){
	int t = 2;
	
	FILE *arq_nodes = fopen("data/Nodes.txt","r");
	if(!arq_nodes) exit(1);
	FILE *arq_relationships = fopen("data/Relationships.txt","r");
	if(!arq_relationships) exit(1);
	TARVBM_cria("bin/indices", t);
	FILE *arq_indices = fopen("bin/indices","r+b");
	if(!arq_indices){
		arq_indices = fopen("bin/indices","r+b");
		if(!arq_indices) exit(1);
	}
	
	int t_lido, offset;
    fread(&t_lido, sizeof(int), 1, arq_indices);//pula o t que ta no inicio do arquivo
    fread(&offset, sizeof(int), 1, arq_indices);//posição da raiz
    
	char *tipo, *nome, *ano, *frase_filme;
	char *linha = (char*)malloc(sizeof(char)*N);
	if(!linha) exit(1);
	
	int id_folha_contador = 0;
	long offset_arquivo = 0;
	
	while(fgets(linha, N, arq_nodes)){
		char *ponteiro_linha = linha;
	
		tipo = divide_string(&ponteiro_linha);
		if(strcmp(tipo,"Person") == 0){
			nome = divide_string(&ponteiro_linha);
			ano = divide_string(&ponteiro_linha);
			
			offset = TARVBM_insere(arq_indices, offset, nome, offset_arquivo, id_folha_contador, t);
			id_folha_contador++;
		}else{
			nome = divide_string(&ponteiro_linha);
			ano = divide_string(&ponteiro_linha);
			frase_filme = divide_string(&ponteiro_linha);
			
			offset = TARVBM_insere(arq_indices, offset, nome, offset_arquivo, id_folha_contador, t);
			id_folha_contador++;
		}
		
		offset_arquivo = ftell(arq_nodes);

		printf("Estado da Arvore apos inserir: %s\n", nome);
		TARVBM_imprime(arq_indices, offset, t);
		printf("\n");
	}
	
	fseek(arq_indices, sizeof(int), SEEK_SET);
    fwrite(&offset, sizeof(int), 1, arq_indices);
    
    TARVBM_imprime(arq_indices, offset,t);
	
	free(linha);
	fclose(arq_nodes);
	fclose(arq_relationships);
	fclose(arq_indices);
	
	return 0;
}
