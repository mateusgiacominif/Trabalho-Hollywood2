#include "TARVBM_trab.h"
#include "TG.h"
#define N 1000

void imprime_folha(int id_folha, int t){
    char nome_arq[50];
    sprintf(nome_arq, "bin/folha_%d.bin", id_folha);
    
    FILE *fp = fopen(nome_arq, "rb");
    if(!fp){
        printf("Arquivo folha_%d.bin nao encontrado\n", id_folha);
        return;
    }
    
    printf("=== folha_%d.bin ===\n", id_folha);
    dados d;
    for(int i = 0; i < (2*t)-1; i++){
        fread(&d, sizeof(dados), 1, fp);
        if(d.nome[0] == '\0'){
            printf("[%d] (vazio)\n", i);
        } else {
            printf("[%d] tipo=%s | nome=%s | ano=%d | prim_viz=%ld |frase=%s\n",
                i, d.tipo, d.nome, d.ano, d.offset_prim_viz, d.frase);
        }
    }
    
    fclose(fp);
}

int main(void){
	//int t = 2;
	int t, offset;
	
	
	FILE *arq_indices = fopen("bin/indices","r+b");
	if(!arq_indices){
		printf("Informe o grau minimo t da arvore: ");
    	scanf("%d", &t);
	
		TARVBM_cria("bin/indices", t);
		arq_indices = fopen("bin/indices","r+b");
		if(!arq_indices) exit(1);
		
		fread(&t, sizeof(int), 1, arq_indices);
    	fread(&offset, sizeof(int), 1, arq_indices);
		
		FILE *arq_nodes = fopen("data/Nodes.txt","r");
		if(!arq_nodes) exit(1);
		
		//FILE *arq_relationships = fopen("data/Relationships.txt","r");
		//if(!arq_relationships) exit(1);
		FILE *arq_rel = fopen("data/Relationships.txt", "r");
		FILE *arq_grafo = fopen("bin/relacionamentos.bin", "w+b");
		
		char *tipo, *nome, *ano, *frase_filme;
		char *linha = (char*)malloc(sizeof(char)*N);
		if(!linha) exit(1);
		
		while(fgets(linha, N, arq_nodes)){
			char *ponteiro_linha = linha;
			
			dados dado = {0};
			dado.offset_prim_viz = -1;
		
			tipo = divide_string(&ponteiro_linha);
			if(!tipo || strlen(tipo) == 0){
				continue;
			}
			strcpy(dado.tipo, tipo);
			
			if(strcmp(tipo,"Person") == 0){
				nome = divide_string(&ponteiro_linha);
				ano = divide_string(&ponteiro_linha);

				if(!nome || !ano) continue;
				
				strcpy(dado.nome, nome);
				dado.ano = atoi(ano);
				
				offset = TARVBM_insere(arq_indices, offset, nome, dado, t);
			}else{
				nome = divide_string(&ponteiro_linha);
				ano = divide_string(&ponteiro_linha);
				if(!nome || !ano) continue;
				frase_filme = divide_string(&ponteiro_linha);
				
				if(!nome || !ano || !frase_filme) continue;
				
				strcpy(dado.nome, nome);
				dado.ano = atoi(ano);
				if(!ano || !isdigit(ano[0]))continue;
				strcpy(dado.frase, frase_filme);
				
				offset = TARVBM_insere(arq_indices, offset, nome, dado, t);
			}
		}

		while(fgets(linha, N, arq_rel)){
			char *ponteiro_linha = linha;
			char *lixo = divide_string(&ponteiro_linha); // start person ou start movie joga fora

			char *nome_origem = divide_string(&ponteiro_linha);
			char *relacao = divide_string(&ponteiro_linha);
			
			lixo = divide_string(&ponteiro_linha); //pular end movie/person

			char *nome_destino = divide_string(&ponteiro_linha);
			if(!nome_origem || !nome_destino) continue;
			char *papel = ponteiro_linha; //pode existir ou nao?
			if(papel && strncmp(papel, "role: ", 6) == 0){ //teste
				papel += 6; //pula role
				
				char *nova_linha = strchr(papel, '\n');
				if(nova_linha) *nova_linha = '\0';
				char *r_linha = strchr(papel, '\r'); //limpeza
				if(r_linha) *r_linha = '\0'; 
			} else {
				papel = ""; //nao tem papel
			} 
			insere_aresta_disco(arq_indices, arq_grafo, offset, nome_origem, nome_destino, relacao, papel, t);
		}

		fseek(arq_indices, sizeof(int), SEEK_SET);
		fwrite(&offset, sizeof(int), 1, arq_indices);
		fclose(arq_nodes);
		fclose(arq_grafo);
		free(linha);
	}else{
		int id;
		fread(&t, sizeof(int), 1, arq_indices);
    	fread(&offset, sizeof(int), 1, arq_indices);
    	fread(&id, sizeof(int), 1, arq_indices);
    	printf("Arvore já existe com t=%d", t);
	}
    
    TARVBM_imprime(arq_indices, offset,t);
    
    // imprime as primeiras N folhas para testar
	for(int i = 1; i <= 10; i++){
		imprime_folha(i, t);
	}
	
	//fclose(arq_relationships);
	fclose(arq_indices);

	brincar_com_grafo(arq_indices, offset, "Charlize Theron", t);
    brincar_com_grafo(arq_indices, offset, "The Matrix", t);
    brincar_com_grafo(arq_indices, offset, "Tom Hanks", t);

	return 0;
}
