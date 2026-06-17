#include "TG.h"
#include "TARVBM_trab.h"

TG* TG_inicializa(){
  return NULL;
}


void TG_imprime(TG *g){
  while(g){
    printf("Vizinhos do No %d: ", g->id_no);
    TVIZ *v = g->prim_viz;
    while(v){
      printf("%d ", v->id_viz);
      v = v->prox_viz;
    }
    printf("\n");
    g = g->prox_no;
  }
}

void TG_imp_rec(TG *g){
  if(g){
    printf("Vizinhos do No %d: ", g->id_no);
    TVIZ *v = g->prim_viz;
    while(v){
      printf("%d ", v->id_viz);
      v = v->prox_viz;
    }
    printf("\n");
    TG_imp_rec(g->prox_no);
  }
}

void TG_libera_viz(TVIZ *v){
  while(v){
    TVIZ *temp = v;
    v = v->prox_viz;
    free(temp);
  }
}

void TG_libera_viz_rec(TVIZ *v){
  if(!v) return;
  TG_libera_viz_rec(v->prox_viz);
  free(v);
}

void TG_libera(TG *g){
  while(g){
    TG_libera_viz(g->prim_viz);
    TG *temp = g;
    g = g->prox_no;
    free(temp);
  }
}

void TG_libera_rec(TG *g){
  if(g){
    TG_libera_viz(g->prim_viz);
    TG_libera_rec(g ->prox_no);
    free(g);
  }
}

TG* TG_busca_no(TG* g, int x){
  if((!g) || (g->id_no == x)) return g;
  return(TG_busca_no(g->prox_no, x));
}

TVIZ* TG_busca_aresta(TG *g, int no1, int no2){
  TG *p1 = TG_busca_no(g, no1), *p2 = TG_busca_no(g, no2);
  if((!p1) || (!p2)) return NULL;
  TVIZ *resp = p1->prim_viz;
  while((resp) && (resp->id_viz != no2)) resp = resp->prox_viz;
  return resp;
}

TG* TG_ins_no(TG *g, int x){
  TG *p = TG_busca_no(g, x);
  if(!p){
    p = (TG*) malloc(sizeof(TG));
    p->id_no = x;
    p->prox_no = g;
    p->prim_viz = NULL;
    g = p;
  }
  return g;
}

void TG_ins_um_sentido(TG *g, int no1, int no2){
  TG *p = TG_busca_no(g, no1), *q = TG_busca_no(g, no2);
  if((!p) || (!q)) return;
  TVIZ *nova = (TVIZ *) malloc(sizeof(TVIZ));
  nova->id_viz = no2;
  nova->prox_viz = p->prim_viz;
  p->prim_viz = nova;
}

void brincar_com_grafo(FILE *arq_indices, int offset_raiz, char *nome_busca, int t) {
    // 1. Busca a entidade na Árvore B+
    TARVBM *no = TARVBM_busca(arq_indices, offset_raiz, nome_busca, t);
    if (!no) {
        printf("\n[X] Ops! '%s' nao foi encontrado na arvore.\n", nome_busca);
        return;
    }

    // 2. Acha a posição exata da chave dentro da folha
    int i = 0;
    while (i < no->nchaves && strcmp(no->chave[i], nome_busca) != 0) i++;

    // 3. Abre o arquivo da folha para ler o offset inicial do grafo
    char nome_arq_folha[50];
    sprintf(nome_arq_folha, "bin/folha_%d.bin", no->filho[0]);
    FILE *f_folha = fopen(nome_arq_folha, "rb");
    if (!f_folha) {
        libera_no(no, t);
        return;
    }

    dados d;
    fseek(f_folha, i * sizeof(dados), SEEK_SET);
    fread(&d, sizeof(dados), 1, f_folha);
    fclose(f_folha);
    libera_no(no, t); // Já pegamos o que queríamos, libera a RAM!

    // 4. Verifica se tem relacionamentos
    if (d.offset_prim_viz == -1) {
        printf("\n>> '%s' existe, mas nao possui relacionamentos cadastrados.\n", nome_busca);
        return;
    }

    // 5. O Show da Navegação: Pulando pelo arquivo de relacionamentos
    printf("\n====================================================\n");
    printf(" Ficha de: %s\n", nome_busca);
    printf("====================================================\n");

    FILE *f_grafo = fopen("bin/relacionamentos.bin", "rb");
    if (!f_grafo) return;

    long offset_atual = d.offset_prim_viz;
    Relacionamentos rel;

    // Loop que percorre a lista encadeada no disco!
    while (offset_atual != -1) {
        fseek(f_grafo, offset_atual, SEEK_SET);
        fread(&rel, sizeof(Relacionamentos), 1, f_grafo);

        if (strlen(rel.papel) > 0) {
            printf(" -> %s [%s] no papel de '%s'\n", rel.tipo_relacao, rel.nome_destino, rel.papel);
        } else {
            printf(" -> %s [%s]\n", rel.tipo_relacao, rel.nome_destino);
        }

        // Pula para a próxima struct no disco
        offset_atual = rel.offset_prox_viz; 
    }

    fclose(f_grafo);
    printf("====================================================\n\n");
}

void insere_um_sentido_disco(FILE *arq_indices, FILE *arq_grafo, int offset_raiz, char *nome_o, char *nome_d, char *rel, char *papel, int t){
  TARVBM *no_origem = TARVBM_busca(arq_indices, offset_raiz, nome_o, t);
  if(!no_origem){
    printf("ERRO GRAFO: '%s' nao encontrado na Arvore!\n", nome_o);
    return;
  }

  int i = 0;
  while(i < no_origem->nchaves && strcmp(no_origem->chave[i], nome_o) != 0) i++;

  char nome_arq_folha[50];
  sprintf(nome_arq_folha, "bin/folha_%d.bin", no_origem->filho[0]);
  FILE *f_folha = fopen(nome_arq_folha, "r+b");
  
  if (!f_folha) {
      printf("ERRO: Nao foi possivel abrir %s\n", nome_arq_folha);
      libera_no(no_origem, t);
      return;
  }

  dados dado_origem;
  fseek(f_folha, i * sizeof(dados), SEEK_SET);
  fread(&dado_origem, sizeof(dados), 1, f_folha);

  Relacionamentos nova_aresta;
  strcpy(nova_aresta.nome_destino, nome_d);
  strcpy(nova_aresta.tipo_relacao, rel);
  strcpy(nova_aresta.papel, papel);

  nova_aresta.offset_prox_viz = dado_origem.offset_prim_viz;

  // salva a aresta no final do arq de relacionamentos
  fseek(arq_grafo, 0, SEEK_END);
  long offset_nova_aresta = ftell(arq_grafo); //offset da nova aresta
  fwrite(&nova_aresta, sizeof(Relacionamentos), 1, arq_grafo);

  //atualiza folha
  dado_origem.offset_prim_viz = offset_nova_aresta;
  fseek(f_folha, i * sizeof(dados), SEEK_SET);
  fwrite(&dado_origem, sizeof(dados), 1, f_folha);

  fclose(f_folha);
  libera_no(no_origem, t);
}

void insere_aresta_disco(FILE *arq_indices, FILE *arq_grafo, int offset_raiz, char *origem, char *destino, char *rel, char *papel, int t){
  insere_um_sentido_disco(arq_indices, arq_grafo, offset_raiz, origem, destino, rel, papel, t);
    
    // Ex: Matrix -> Keanu (HAS_ACTOR ou apenas repassa o ACTED_IN para simplificar)
    insere_um_sentido_disco(arq_indices, arq_grafo, offset_raiz, destino, origem, rel, papel, t);
}

void TG_ins_aresta(TG *g, int no1, int no2){
  TVIZ *v = TG_busca_aresta(g, no1, no2);
  if(v) return;
  TG_ins_um_sentido(g, no1, no2);
  TG_ins_um_sentido(g, no2, no1);
}

void TG_retira_um_sentido(TG *g, int no1, int no2){
  TG *p = TG_busca_no(g, no1);
  if(!p) return;
  TVIZ *ant = NULL, *atual = p->prim_viz;
  while((atual) && (atual->id_viz != no2)){
    ant = atual;
    atual = atual->prox_viz;
  }
  if(!atual) return;
  if(!ant) p->prim_viz = atual->prox_viz;
  else ant->prox_viz = atual->prox_viz;
  free(atual);
}

void TG_retira_aresta(TG *g ,int no1, int no2){
  TVIZ* v = TG_busca_aresta(g, no1, no2);
  if(!v) return;
  TG_retira_um_sentido(g, no1, no2);
  TG_retira_um_sentido(g, no2, no1);
}

TG* TG_retira_no(TG *g, int no){
  TG *p = g, *ant = NULL;
  while((p) && (p->id_no != no)){
    ant = p;
    p = p->prox_no;
  }
  if(!p) return g;
  TVIZ *v = p->prim_viz, *t;
  while(v){
    TG_retira_um_sentido(g, v->id_viz, no);
    t = v;
    v = v->prox_viz;
    free(t);
  }
  if(!ant) g = g->prox_no;
  else ant->prox_no = p->prox_no;
  free(p);
  return g;
}

