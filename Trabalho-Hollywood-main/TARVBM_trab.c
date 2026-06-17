#include "TARVBM_trab.h"

void libera_no(TARVBM *no, int t){
    if(!no) return;
    
    for (int j = 0; j < ((2 * t) - 1); j++){
        free(no->chave[j]);
    }
    free(no->chave);
    free(no->filho);
    free(no);
}

void TARVBM_cria(char *arq, int t){
	FILE *fp = fopen(arq,"wb");
	if(!fp)exit(1);
	
	fwrite(&t, sizeof(int), 1, fp);
	
	int offset_inicial = sizeof(int) * 3;
	fwrite(&offset_inicial, sizeof(int), 1, fp);
	
	int id_inicial = 1;
	fwrite(&id_inicial, sizeof(int), 1, fp);
	
	FILE *arq_folha = fopen("bin/folha_1.bin", "w+b");
	if(arq_folha){
		dados dado = {0};
		dado.offset_prim_viz = -1;
		for(int i = 0; i < ((2 * t) - 1); i++) {
			fwrite(&dado, sizeof(dados), 1, arq_folha); 
		}
		fclose(arq_folha);
	}
	
	int folha = 1, nchaves = 0, prox = -1, filho = -1;
	char chave[150] = {0};
	
	fwrite(&nchaves, sizeof(int), 1, fp);
	fwrite(&folha, sizeof(int), 1, fp);
	fwrite(&prox, sizeof(int), 1, fp);
	
	for(int i=0; i < ((2 * t) - 1); i++){
		fwrite(chave, sizeof(char), 150, fp);
	}
	
	fwrite(&id_inicial, sizeof(int), 1, fp);
	for(int i=1; i < (2 * t); i++){
		fwrite(&filho, sizeof(int), 1, fp);
	}
	
	fclose(fp);
}

TARVBM *le_no(FILE *fp, int offset, int t){
    if (offset == -1) return NULL;

    TARVBM *no = (TARVBM *)malloc(sizeof(TARVBM));
    
    fseek(fp, offset, SEEK_SET);

    fread(&no->nchaves, sizeof(int), 1, fp);
    fread(&no->folha, sizeof(int), 1, fp);
    fread(&no->prox, sizeof(int), 1, fp);

    no->chave = (char **)malloc(sizeof(char *) * ((2 * t) - 1));
    for (int i = 0; i < ((2 * t) - 1); i++){
        no->chave[i] = (char *)malloc(150);
        fread(no->chave[i], sizeof(char), 150, fp);
    }

    no->filho = (int *)malloc(sizeof(int) * (2 * t));
    for (int i = 0; i < (2 * t); i++){
        fread(&no->filho[i], sizeof(int), 1, fp);
    }

    return no;
}

TARVBM *TARVBM_busca1(char *arq, char *nome, int t){
    FILE *fp = fopen(arq, "rb");
    if (!fp) return NULL;

    int t_lido, offset, id;
    fread(&t_lido, sizeof(int), 1, fp);
    fread(&offset, sizeof(int), 1, fp);
    fread(&id, sizeof(int), 1, fp);

    while (offset != -1){
        TARVBM *no = le_no(fp, offset, t);

        int i = 0;
        while ((i < no->nchaves) && (strcmp(nome, no->chave[i]) > 0)){
            i++;
        }

        if ((i < no->nchaves) && no->folha && (strcmp(nome, no->chave[i]) == 0)){
            fclose(fp);
            return no;
        }

        if (no->folha){
            for (int j = 0; j < ((2 * t) - 1); j++){
                free(no->chave[j]);
            }
            free(no->chave);
            free(no->filho);
            free(no);
            fclose(fp);
            return NULL;
        }

        if ((i < no->nchaves) && (strcmp(nome, no->chave[i]) == 0)){
            i++;
        }

        offset = no->filho[i];
        
        for (int j = 0; j < ((2 * t) - 1); j++){
			free(no->chave[j]);
       	}
       	free(no->chave);
		free(no->filho);
		free(no); 
    }

    fclose(fp);
    return NULL;
}

TARVBM *TARVBM_busca(FILE *fp, int offset, char *nome, int t){
    if (!fp) return NULL;

    while (offset != -1){
        TARVBM *no = le_no(fp, offset, t);
        if(!no) return NULL;

        int i = 0;
        while ((i < no->nchaves) && (strcmp(nome, no->chave[i]) > 0)){
            i++;
        }

        // Achou e é folha!
        if ((i < no->nchaves) && no->folha && (strcmp(nome, no->chave[i]) == 0)){
            return no;
        }

        // Chegou na folha e não achou
        if (no->folha){
            libera_no(no, t);
            return NULL;
        }

        // Se for igual a uma chave de nó interno, vai para o filho à direita
        if ((i < no->nchaves) && (strcmp(nome, no->chave[i]) == 0)){
            i++;
        }

        // Desce na árvore
        offset = no->filho[i];
        libera_no(no, t);
    }
    return NULL;
}

int aloca_interno(FILE *fp, int t){
	fseek(fp, 0, SEEK_END);
	int offset = ftell(fp);
	
	int folha = 0, nchaves = 0, prox = -1, filho = -1;
	char chave[150] = {0};
	
	fwrite(&nchaves, sizeof(int), 1, fp);
	fwrite(&folha, sizeof(int), 1, fp);
	fwrite(&prox, sizeof(int), 1, fp);
	
	for(int i=0; i < ((2 * t) - 1); i++){
		fwrite(chave, sizeof(char), 150, fp);
	}
	for(int i=0; i < (2 * t); i++){
		fwrite(&filho, sizeof(int), 1, fp);
	}
	
	return offset;
}

int aloca_folha(FILE *fp, int t){
	fseek(fp, 2*sizeof(int), SEEK_SET);//pula o t e offset que estão no inicio do arquivo
	
	int id;
	fread(&id, sizeof(int),1,fp);
	id++;
	
	fseek(fp, 2*sizeof(int), SEEK_SET);
	fwrite(&id,sizeof(int), 1, fp);//atualizo o contador de id no arquivo
	
	char nome_arq[50];
	sprintf(nome_arq, "bin/folha_%d.bin", id);
	
	FILE *arq_folha = fopen(nome_arq, "w+b");
	if(!arq_folha){
		exit(1);
	}
	
	/*entra a struct dados*/
	dados dado = {0};
	dado.offset_prim_viz = -1;
	for(int i = 0; i < ((2 * t) - 1); i++) {
		fwrite(&dado, sizeof(dados), 1, arq_folha); 
	}
	
	fclose(arq_folha);
	
	fseek(fp, 0, SEEK_END);
	int offset = ftell(fp);
	
	int folha = 1, nchaves = 0, prox = -1, filho = -1;
	char chave[150] = {0};
	
	fwrite(&nchaves, sizeof(int), 1, fp);
	fwrite(&folha, sizeof(int), 1, fp);
	fwrite(&prox, sizeof(int), 1, fp);
	
	for(int i=0; i < ((2 * t) - 1); i++){
		fwrite(chave, sizeof(char), 150, fp);
	}
	
	fwrite(&id,sizeof(int),1,fp);//o primeiro filho de uma folha que guarda o id do arquivo da folha
	for(int i=1; i < (2 * t); i++){
		fwrite(&filho, sizeof(int), 1, fp);
	}
	
	return offset;
}

void divisao(FILE *fp, int offset, int indice_filho, TARVBM *no_pai, int t){
	TARVBM *f = le_no(fp, no_pai->filho[indice_filho], t);
	
	int offset_f = no_pai->filho[indice_filho];
	
	int final_arq;
	
	if(f->folha){
		final_arq = aloca_folha(fp,t);
	}else{
		final_arq = aloca_interno(fp,t);
	}
	
	TARVBM *novo_no = le_no(fp, final_arq, t);
	
	if(f->folha){	
		char valor_mediana[150];
		strcpy(valor_mediana, f->chave[t-1]);// valor exato da mediana
		
		for(int i=no_pai->nchaves-1;i>=indice_filho;i--){
			strcpy(no_pai->chave[i+1],no_pai->chave[i]);
			no_pai->filho[i+2] = no_pai->filho[i+1];
		}
		strcpy(no_pai->chave[indice_filho],valor_mediana);
		no_pai->filho[indice_filho+1] = final_arq;
		no_pai->nchaves++;
		
		char nome_arq_f[50], nome_arq_novo[50];
		sprintf(nome_arq_f, "bin/folha_%d.bin", f->filho[0]);// ID do arquivo velho
		sprintf(nome_arq_novo, "bin/folha_%d.bin", novo_no->filho[0]);// ID do arquivo novo
		
		FILE *arq_dados_f = fopen(nome_arq_f, "r+b");
		FILE *arq_dados_novo = fopen(nome_arq_novo, "r+b");
		
		dados novo_dado;
		dados d_vazio = {0};
		
		for(int i = 0; i < t; i++){// lado direito
			strcpy(novo_no->chave[i], f->chave[(t-1)+i]);
			novo_no->nchaves++;
			
			if (arq_dados_f && arq_dados_novo) {
				fseek(arq_dados_f, ((t-1)+i) * sizeof(dados), SEEK_SET);
				fread(&novo_dado, sizeof(dados), 1, arq_dados_f);
				
				fseek(arq_dados_novo, i * sizeof(dados), SEEK_SET);
				fwrite(&novo_dado, sizeof(dados), 1, arq_dados_novo);
				
				fseek(arq_dados_f, ((t-1)+i) * sizeof(dados), SEEK_SET);
				fwrite(&d_vazio, sizeof(dados), 1, arq_dados_f);
			}
		}
		
		if (arq_dados_f) fclose(arq_dados_f);
		if (arq_dados_novo) fclose(arq_dados_novo);
		
		int nchaves_original = f->nchaves;
		for(int i = t - 1; i < nchaves_original; i++){ // lado esquerdo
			f->chave[i][0] = '\0';
			f->nchaves--;
		}
		
		novo_no->prox = f->prox;
		f->prox = final_arq;
	} else{
		char valor_mediana[150];
		strcpy(valor_mediana, f->chave[t-1]);
		
		novo_no->folha = 0;
		
		for(int i=no_pai->nchaves-1;i>=indice_filho;i--){
			strcpy(no_pai->chave[i+1],no_pai->chave[i]);
			no_pai->filho[i+2] = no_pai->filho[i+1];
		}
		strcpy(no_pai->chave[indice_filho],valor_mediana);
		no_pai->filho[indice_filho+1] = final_arq;
		no_pai->nchaves++;
		
		for(int i=0;i<t-1;i++){// lado direito
			strcpy(novo_no->chave[i],f->chave[t+i]);
			novo_no->nchaves++;
		}
		
		for(int i=0;i<t;i++){
			novo_no->filho[i] = f->filho[t+i];
		}
		
		int nchaves_original = f->nchaves;
		for(int i=t-1;i<nchaves_original;i++){ // lado esquerdo
			f->chave[i][0] = '\0';
			f->nchaves--;
		}
		for(int i=t;i<(2*t);i++){
			f->filho[i]=-1;
		}
	}
	
	fseek(fp, offset, SEEK_SET);
	fwrite(&no_pai->nchaves, sizeof(int), 1, fp);
	fwrite(&no_pai->folha, sizeof(int), 1, fp);
	fwrite(&no_pai->prox, sizeof(int), 1, fp);
	for(int j=0;j<((2 * t) - 1);j++){
		fwrite(no_pai->chave[j], sizeof(char)*150, 1, fp);
	}
	for(int j=0;j<(2 * t);j++){
		fwrite(&no_pai->filho[j], sizeof(int), 1, fp);
	}
	
	fseek(fp, offset_f, SEEK_SET);
	fwrite(&f->nchaves, sizeof(int), 1, fp);
	fwrite(&f->folha, sizeof(int), 1, fp);
	fwrite(&f->prox, sizeof(int), 1, fp);
	for(int j=0;j<((2 * t) - 1);j++){
		fwrite(f->chave[j], sizeof(char)*150, 1, fp);
	}
	for(int j=0;j<(2 * t);j++){
		fwrite(&f->filho[j], sizeof(int), 1, fp);
	}
	
	fseek(fp, final_arq,SEEK_SET);
	fwrite(&novo_no->nchaves, sizeof(int), 1, fp);
	fwrite(&novo_no->folha, sizeof(int), 1, fp);
	fwrite(&novo_no->prox, sizeof(int), 1, fp);
	for(int j=0;j<((2 * t) - 1);j++){
		fwrite(novo_no->chave[j], sizeof(char)*150, 1, fp);
	}
	for(int j=0;j<(2 * t);j++){
		fwrite(&novo_no->filho[j], sizeof(int), 1, fp);
	}	 
	
	for (int j = 0; j < ((2 * t) - 1); j++){
		free(f->chave[j]);
   	}
    free(f->chave);
	free(f->filho);
	free(f);
	
	for (int j = 0; j < ((2 * t) - 1); j++){
		free(novo_no->chave[j]);
   	}
    free(novo_no->chave);
	free(novo_no->filho);
	free(novo_no);
}

void insere_dado_arquivo(int id_folha, int indice_alvo, int nchaves_antigo, dados dado) {
	char arq_folha[50];
	sprintf(arq_folha, "bin/folha_%d.bin", id_folha);
	FILE *arq_dados = fopen(arq_folha, "r+b");
	
	if (!arq_dados) return;
	
	dados d_temp;
	
	for (int j = nchaves_antigo - 1; j >= indice_alvo; j--) {
		fseek(arq_dados, j * sizeof(dados), SEEK_SET);
		fread(&d_temp, sizeof(dados), 1, arq_dados);
		
		fseek(arq_dados, (j + 1) * sizeof(dados), SEEK_SET);
		fwrite(&d_temp, sizeof(dados), 1, arq_dados);
	}
	
	fseek(arq_dados, indice_alvo * sizeof(dados), SEEK_SET);
	fwrite(&dado, sizeof(dados), 1, arq_dados);
	
	fclose(arq_dados);
}

void insere_nao_completo(FILE *fp, int offset, char *nome, dados dado, int t){
	TARVBM *no = le_no(fp, offset, t);
	int i;
	
	if(no->folha){
		i = no->nchaves-1;
		
		while(i >= 0 && strcmp(nome, no->chave[i]) < 0){
			strcpy(no->chave[i+1], no->chave[i]);
			i--;
		}
		
		strcpy(no->chave[i+1], nome);
		insere_dado_arquivo(no->filho[0], i + 1, no->nchaves, dado);
		no->nchaves++;
		
		fseek(fp, offset, SEEK_SET);
		
		fwrite(&no->nchaves, sizeof(int), 1, fp);
		fwrite(&no->folha, sizeof(int), 1, fp);
		fwrite(&no->prox, sizeof(int), 1, fp);
		
		for(int j=0;j<((2 * t) - 1);j++){
			fwrite(no->chave[j], sizeof(char)*150, 1, fp);
		}
		
		for(int j=0;j<(2 * t);j++){
			fwrite(&no->filho[j], sizeof(int), 1, fp);
		}
		
		for (int j = 0; j < ((2 * t) - 1); j++){
			free(no->chave[j]);
		}
	   	free(no->chave);
		free(no->filho);
		free(no);
	}else{
		i = no->nchaves-1;
		while(i >= 0 && strcmp(nome, no->chave[i]) < 0){
			i--;
		}
		i++;
		
		TARVBM *filho_desce = le_no(fp, no->filho[i], t);
		
		if(filho_desce->nchaves == (2*t)-1){
			for(int j=0;j<(2*t)-1;j++) free(filho_desce->chave[j]);
			free(filho_desce->chave);
			free(filho_desce->filho);
			free(filho_desce);
			filho_desce = NULL;
			
			divisao(fp, offset, i, no, t);
			if(strcmp(nome, no->chave[i]) > 0) i++;
			insere_nao_completo(fp, no->filho[i], nome, dado, t);
		} else {
			for(int j=0;j<(2*t)-1;j++) free(filho_desce->chave[j]);
			free(filho_desce->chave);
			free(filho_desce->filho);
			free(filho_desce);
			filho_desce = NULL;
			
			insere_nao_completo(fp, no->filho[i], nome, dado, t);
		}
		
		//insere_nao_completo(fp,no->filho[i],nome, dado,t);
		
		for (int j = 0; j < ((2 * t) - 1); j++){
			free(no->chave[j]);
		}
	   	free(no->chave);
		free(no->filho);
		free(no);
	}
}

int TARVBM_insere(FILE *fp, int offset, char *nome, dados dado,int t){
	TARVBM *raiz = le_no(fp, offset, t);
	
	if(raiz->nchaves == ((2 * t) - 1)){
		int final_arq = aloca_interno(fp, t);
		TARVBM *nova_raiz = le_no(fp, final_arq, t);
		
		nova_raiz->folha = 0;
		nova_raiz->filho[0] = offset;
		
		fseek(fp, final_arq, SEEK_SET);
		fwrite(&nova_raiz->nchaves, sizeof(int), 1, fp);
		fwrite(&nova_raiz->folha, sizeof(int), 1, fp);
		fwrite(&nova_raiz->prox, sizeof(int), 1, fp);
		for(int j = 0; j < (2*t)-1; j++)
			fwrite(nova_raiz->chave[j], sizeof(char)*150, 1, fp);
		for(int j = 0; j < 2*t; j++)
			fwrite(&nova_raiz->filho[j], sizeof(int), 1, fp);
		
		
		divisao(fp, final_arq, 0, nova_raiz, t);
		insere_nao_completo(fp, final_arq, nome, dado,t);
		
		for (int j = 0; j < ((2 * t) - 1); j++){
			free(raiz->chave[j]);
		}
	   	free(raiz->chave);
		free(raiz->filho);
		free(raiz);
		
		for (int j = 0; j < ((2 * t) - 1); j++){
			free(nova_raiz->chave[j]);
		}
	   	free(nova_raiz->chave);
		free(nova_raiz->filho);
		free(nova_raiz);
		
		return final_arq;
	} else{
		insere_nao_completo(fp, offset, nome, dado, t);
		
		for (int j = 0; j < ((2 * t) - 1); j++){
			free(raiz->chave[j]);
		}
	   	free(raiz->chave);
		free(raiz->filho);
		free(raiz);
		
		return offset;
	}
}

char *divide_string(char **linha){
	if((*linha)==NULL || (**linha)=='\0'){
		return NULL;
	}
	
	char *inicio_linha = (*linha);
	
	char *pos_separador = strstr(inicio_linha," | ");
	
	if(pos_separador){
		(*pos_separador) = '\0';
		(*linha) = pos_separador + 3;
	}else{
		pos_separador = strchr(inicio_linha,'\n');
		
		if(pos_separador){
			(*pos_separador) = '\0';
		}
		pos_separador = strchr(inicio_linha, '\n');
		if(pos_separador) *pos_separador = '\0';
		
		// Limpa o \r (O CULPADO!)
		pos_separador = strchr(inicio_linha, '\r');
		if(pos_separador) *pos_separador = '\0';
		
		*linha = NULL;
	}
	
	return inicio_linha;
}

void imp(FILE *fp, int offset, int andar, int t) {
	if(offset==-1)return;

	TARVBM *no = le_no(fp, offset, t);
	if(!no)return;
	
	if(!no->folha){
		imp(fp, no->filho[no->nchaves], andar+1, t);
	}
	
	for(int i=no->nchaves-1;i>=0;i--){
		for(int j = 0;j<andar;j++){
			printf("\t");
		}
		printf("%s\n",no->chave[i]);
		if(!no->folha){
			imp(fp, no->filho[i], andar+1, t);
		}
	}
	
	for (int j = 0; j < ((2 * t) - 1); j++){
		free(no->chave[j]);
	}
   	free(no->chave);
	free(no->filho);
	free(no);
}

void TARVBM_imprime(FILE *fp, int offset, int t){
	imp(fp, offset, 0, t);
}
