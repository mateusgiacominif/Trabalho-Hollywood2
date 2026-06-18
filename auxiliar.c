case 12: // (l) Questoes de (f) a (k) filtradas por decada
	printf("\n--- Escolha qual podio filtrar por decada ---\n");
	printf(" (6) Atores que MAIS atuaram\n");
	printf(" (7) Atores que MENOS atuaram\n");
	printf(" (8) Diretores que MAIS dirigiram\n");
	printf(" (9) Diretores que MENOS dirigiram\n");
	printf("(10) Produtores MAIS atuantes\n");
	printf("(11) Produtores MENOS atuantes\n");
	printf("Escolha a sub-opcao: ");
	int sub_op;
	scanf("%d", &sub_op);
	if (sub_op == 6)       relatorio_podio_decadas(arq_indices, offset, arq_grafo_leitura, t, "ACTED_IN", 1);
	else if (sub_op == 7)  relatorio_podio_decadas(arq_indices, offset, arq_grafo_leitura, t, "ACTED_IN", 0);
	else if (sub_op == 8)  relatorio_podio_decadas(arq_indices, offset, arq_grafo_leitura, t, "DIRECTED", 1);
	else if (sub_op == 9)  relatorio_podio_decadas(arq_indices, offset, arq_grafo_leitura, t, "DIRECTED", 0);
	else if (sub_op == 10) relatorio_podio_decadas(arq_indices, offset, arq_grafo_leitura, t, "PRODUCED", 1);
	else if (sub_op == 11) relatorio_podio_decadas(arq_indices, offset, arq_grafo_leitura, t, "PRODUCED", 0);
	else printf("Sub-opcao invalida!\n");
	break;




void relatorio_duplas(FILE *arq_indices, int offset_raiz, FILE *arq_grafo, int t, int tipo_filtro);
void relatorio_decadas(FILE *arq_indices, int offset_raiz, FILE *arq_grafo, int t, int tipo_filtro);
void relatorio_podio_decadas(FILE *arq_indices, int offset_raiz, FILE *arq_grafo, int t, char *tipo_aresta, int buscar_maior);



//questoes f a k por decada
void acha_campeao_rec(FILE *arq_indices, int offset, FILE *arq_grafo, int t, char *tipo_aresta, int buscar_maior, int *recorde, char *nome_campeao) {
    if (offset == -1) return;
    TARVBM *no = le_no(arq_indices, offset, t);
    if (!no) return;

    if (!no->folha) {
        for (int i = 0; i <= no->nchaves; i++) {
            acha_campeao_rec(arq_indices, no->filho[i], arq_grafo, t, tipo_aresta, buscar_maior, recorde, nome_campeao);
        }
    } else {
        char nome_arq_folha[50];
        sprintf(nome_arq_folha, "bin/folha_%d.bin", no->filho[0]);
        FILE *f_dados = fopen(nome_arq_folha, "rb");

        if (f_dados) {
            dados d;
            for (int i = 0; i < no->nchaves; i++) {
                fseek(f_dados, i * sizeof(dados), SEEK_SET);
                fread(&d, sizeof(dados), 1, f_dados);

                if (strcmp(d.tipo, "Person") == 0 && d.offset_prim_viz != -1) {
                    int contagem = 0;
                    long offset_atual = d.offset_prim_viz;
                    Relacionamentos rel; 

                    while (offset_atual != -1) {
                        fseek(arq_grafo, offset_atual, SEEK_SET);
                        fread(&rel, sizeof(Relacionamentos), 1, arq_grafo);

                        if (strcmp(rel.tipo_relacao, tipo_aresta) == 0) {
                            contagem++;
                        }
                        offset_atual = rel.offset_prox_viz;
                    }

                    if (contagem > 0) {
                        if (buscar_maior && contagem > *recorde) {
                            *recorde = contagem;
                            strcpy(nome_campeao, d.nome);
                        } 
                        else if (!buscar_maior && contagem < *recorde) {
                            *recorde = contagem;
                            strcpy(nome_campeao, d.nome);
                        }
                    }
                }
            }
            fclose(f_dados);
        }
    }
    libera_no(no, t);
}

void relatorio_podio(FILE *arq_indices, int offset_raiz, FILE *arq_grafo, int t, char *tipo_aresta, int buscar_maior) {
    int recorde = buscar_maior ? -1 : 999999;
    char nome_campeao[150] = "";

    acha_campeao_rec(arq_indices, offset_raiz, arq_grafo, t, tipo_aresta, buscar_maior, &recorde, nome_campeao);

    if (strlen(nome_campeao) > 0) {
        if (buscar_maior) {
            printf("O %s que mais trabalhou foi %s sao %d registros!\n", tipo_aresta, nome_campeao, recorde);
        } else {
            printf("O %s que menos trabalhou foi %s sao %d registros!\n", tipo_aresta, nome_campeao, recorde);
        }
    } else {
        printf("Nenhum registro encontrado para a categoria %s.\n", tipo_aresta);
    }
}

//estruturas auxiliares para questao de guerra

typedef struct {
    char nome[150];
    char relacao[20];
} PessoaFilme;

typedef struct par_freq {
    char p1[150];
    char p2[150];
    int decada;
    int contagem;
    struct par_freq *prox;
} ParFreq;


//questoes D e E

ParFreq* insere_ou_soma_par(ParFreq *lista, char *nome1, char *nome2, int decada) {
    char a[150], b[150];
    if (strcmp(nome1, nome2) < 0) {
        strcpy(a, nome1); strcpy(b, nome2);
    } else {
        strcpy(a, nome2); strcpy(b, nome1);
    }

    ParFreq *atual = lista;
    while (atual) {
        if (atual->decada == decada && strcmp(atual->p1, a) == 0 && strcmp(atual->p2, b) == 0) {
            atual->contagem++; 
            return lista;
        }
        atual = atual->prox;
    }

    ParFreq *novo = (ParFreq*)malloc(sizeof(ParFreq));
    strcpy(novo->p1, a);
    strcpy(novo->p2, b);
    novo->decada = decada;
    novo->contagem = 1;
    novo->prox = lista;
    return novo; 
}

ParFreq* varre_filmes_decada_rec(FILE *arq_indices, int offset, FILE *arq_grafo, int t, int tipo_filtro, ParFreq *lista) {
    if (offset == -1) return lista;
    TARVBM *no = le_no(arq_indices, offset, t);
    if (!no) return lista;

    if (!no->folha) {
        for (int i = 0; i <= no->nchaves; i++) {
            lista = varre_filmes_decada_rec(arq_indices, no->filho[i], arq_grafo, t, tipo_filtro, lista);
        }
    } else {
        char arq_folha[50];
        sprintf(arq_folha, "bin/folha_%d.bin", no->filho[0]);
        FILE *f_dados = fopen(arq_folha, "rb");

        if (f_dados) {
            dados d_filme;
            for (int i = 0; i < no->nchaves; i++) {
                fseek(f_dados, i * sizeof(dados), SEEK_SET);
                fread(&d_filme, sizeof(dados), 1, f_dados);

                if (strcmp(d_filme.tipo, "Movie") == 0 && d_filme.offset_prim_viz != -1) {
                    
                    int decada = (d_filme.ano / 10) * 10; 
                    long offset_aresta = d_filme.offset_prim_viz;
                    Relacionamentos rel;
                    
                    PessoaFilme elenco[200]; 
                    int qtd = 0;

                    while (offset_aresta != -1 && qtd < 200) {
                        fseek(arq_grafo, offset_aresta, SEEK_SET);
                        fread(&rel, sizeof(Relacionamentos), 1, arq_grafo);
                        
                        strcpy(elenco[qtd].nome, rel.nome_destino);
                        strcpy(elenco[qtd].relacao, rel.tipo_relacao);
                        qtd++;
                        
                        offset_aresta = rel.offset_prox_viz;
                    }

                    for (int x = 0; x < qtd; x++) {
                        for (int y = x + 1; y < qtd; y++) {
                            int valido = 0;

                            if (tipo_filtro == 1) {
                                if (strcmp(elenco[x].relacao, "ACTED_IN") == 0 && strcmp(elenco[y].relacao, "ACTED_IN") == 0) {
                                    valido = 1;
                                }
                            }
                            else if (tipo_filtro == 2) {
                                if ((strcmp(elenco[x].relacao, "ACTED_IN") == 0 && strcmp(elenco[y].relacao, "DIRECTED") == 0) ||
                                    (strcmp(elenco[x].relacao, "DIRECTED") == 0 && strcmp(elenco[y].relacao, "ACTED_IN") == 0)) {
                                    valido = 1;
                                }
                            }

                            if (valido) {
                                lista = insere_ou_soma_par(lista, elenco[x].nome, elenco[y].nome, decada);
                            }
                        }
                    }
                }
            }
            fclose(f_dados);
        }
    }
    libera_no(no, t);
    return lista;
}

void relatorio_decadas(FILE *arq_indices, int offset_raiz, FILE *arq_grafo, int t, int tipo_filtro) {
    ParFreq *lista_pares = NULL;
    lista_pares = varre_filmes_decada_rec(arq_indices, offset_raiz, arq_grafo, t, tipo_filtro, lista_pares);

    if (!lista_pares) {
        printf("\nNenhum dado encontrado para gerar o relatorio.\n");
        return;
    }

    printf("\n\n");
    if (tipo_filtro == 1) printf(" (d) ATORES QUE MAIS ATUARAM JUNTOS POR DECADA\n");
    else if (tipo_filtro == 2) printf(" (e) ATORES E DIRETORES JUNTOS POR DECADA\n");
    printf("\n\n");

    for (int d = 1900; d <= 2050; d += 10) {
        int max_contagem = 0;
        
        ParFreq *atual = lista_pares;
        while (atual) {
            if (atual->decada == d && atual->contagem > max_contagem) {
                max_contagem = atual->contagem;
            }
            atual = atual->prox;
        }

        if (max_contagem > 0) {
            printf("\n--- DECADA DE %d ---\n", d);
            atual = lista_pares;
            while (atual) {
                if (atual->decada == d && atual->contagem == max_contagem) {
                    printf("- %s & %s (Trabalharam juntos %d vezes!)\n", atual->p1, atual->p2, atual->contagem);
                }
                atual = atual->prox;
            }
        }
    }
    printf("\n\n");

    while (lista_pares) {
        ParFreq *temp = lista_pares;
        lista_pares = lista_pares->prox;
        free(temp);
    }
}


// questao l, maior frequencia por decada
typedef struct ind_freq {
    char nome[150];
    int decada;
    int contagem;
    struct ind_freq *prox;
} IndFreq;

IndFreq* insere_ou_soma_ind(IndFreq *lista, char *nome, int decada) {
    IndFreq *atual = lista;
    while (atual) {
        if (atual->decada == decada && strcmp(atual->nome, nome) == 0) {
            atual->contagem++;
            return lista;
        }
        atual = atual->prox;
    }
    
    IndFreq *novo = (IndFreq*)malloc(sizeof(IndFreq));
    strcpy(novo->nome, nome);
    novo->decada = decada;
    novo->contagem = 1;
    novo->prox = lista;
    return novo;
}

IndFreq *varre_filmes_ind_rec(FILE *arq_indices, int offset, FILE *arq_grafo, int t, char *tipo_aresta, IndFreq *lista) {
    if (offset == -1) return lista;
    TARVBM *no = le_no(arq_indices, offset, t);
    if (!no) return lista;

    if (!no->folha) {
        for (int i = 0; i <= no->nchaves; i++) {
            lista = varre_filmes_ind_rec(arq_indices, no->filho[i], arq_grafo, t, tipo_aresta, lista);
        }
    } else {
        char arq_folha[50];
        sprintf(arq_folha, "bin/folha_%d.bin", no->filho[0]);
        FILE *f_dados = fopen(arq_folha, "rb");

        if (f_dados) {
            dados d_filme;
            for (int i = 0; i < no->nchaves; i++) {
                fseek(f_dados, i * sizeof(dados), SEEK_SET);
                fread(&d_filme, sizeof(dados), 1, f_dados);

                if (strcmp(d_filme.tipo, "Movie") == 0 && d_filme.offset_prim_viz != -1) {
                    int decada = (d_filme.ano / 10) * 10;
                    long offset_aresta = d_filme.offset_prim_viz;
                    Relacionamentos rel;

                    PessoaFilme elenco[200];
                    int qtd = 0;

                    while (offset_aresta != -1 && qtd < 200) {
                        fseek(arq_grafo, offset_aresta, SEEK_SET);
                        fread(&rel, sizeof(Relacionamentos), 1, arq_grafo);
                        
                        strcpy(elenco[qtd].nome, rel.nome_destino);
                        strcpy(elenco[qtd].relacao, rel.tipo_relacao);
                        qtd++;
                        
                        offset_aresta = rel.offset_prox_viz;
                    }

                    for (int x = 0; x < qtd; x++) {
                        if (strcmp(elenco[x].relacao, tipo_aresta) == 0) {
                            lista = insere_ou_soma_ind(lista, elenco[x].nome, decada);
                        }
                    }
                }
            }
            fclose(f_dados);
        }
    }
    libera_no(no, t);
    return lista;
}

void relatorio_podio_decadas(FILE *arq_indices, int offset_raiz, FILE *arq_grafo, int t, char *tipo_aresta, int buscar_maior) {
    IndFreq *lista_ind = NULL;
    lista_ind = varre_filmes_ind_rec(arq_indices, offset_raiz, arq_grafo, t, tipo_aresta, lista_ind);

    if (!lista_ind) {
        printf("\nNenhum registro encontrado para processar o podio.\n");
        return;
    }

    printf("\n\n");
    printf(" MAIOR FREQUENCIA POR DECADA: %s (%s VALOR)\n", tipo_aresta, buscar_maior ? "MAIOR" : "MENOR");
    printf("\n");

    for (int d = 1900; d <= 2050; d += 10) {
        int recorde = buscar_maior ? -1 : 999999;
        int teve_dados = 0;

        IndFreq *atual = lista_ind;
        while (atual) {
            if (atual->decada == d) {
                teve_dados = 1;
                if (buscar_maior && atual->contagem > recorde) {
                    recorde = atual->contagem;
                } else if (!buscar_maior && atual->contagem < recorde) {
                    recorde = atual->contagem;
                }
            }
            atual = atual->prox;
        }

        if (teve_dados && ((buscar_maior && recorde > 0) || (!buscar_maior && recorde < 999999))) {
            printf("\n--- DECADA DE %d (Recorde: %d registros) ---\n", d, recorde);
            atual = lista_ind;
            while (atual) {
                if (atual->decada == d && atual->contagem == recorde) {
                    printf(" %s com %d participacoes!\n", atual->nome, atual->contagem);
                }
                atual = atual->prox;
            }
        }
    }
    printf("\n\n");

    while (lista_ind) {
        IndFreq *temp = lista_ind;
        lista_ind = lista_ind->prox;
        free(temp);
    }
}


//A B e C
void listar_duplas_rec(FILE *arq_indices, int offset_atual, int offset_raiz, FILE *arq_grafo, int t, int tipo_filtro) {
    if (offset_atual == -1) return;
    TARVBM *no = le_no(arq_indices, offset_atual, t);
    if (!no) return;

    if (!no->folha) {
        for (int i = 0; i <= no->nchaves; i++) {
            listar_duplas_rec(arq_indices, no->filho[i], offset_raiz, arq_grafo, t, tipo_filtro);
        }
    } else {
        char nome_arq_folha[50];
        sprintf(nome_arq_folha, "bin/folha_%d.bin", no->filho[0]);
        FILE *f_dados = fopen(nome_arq_folha, "rb");

        if (f_dados) {
            dados d_pessoa;
            for (int i = 0; i < no->nchaves; i++) {
                fseek(f_dados, i * sizeof(dados), SEEK_SET);
                fread(&d_pessoa, sizeof(dados), 1, f_dados);

                if (strcmp(d_pessoa.tipo, "Person") == 0 && d_pessoa.offset_prim_viz != -1) {
                    long offset_filme = d_pessoa.offset_prim_viz;
                    Relacionamentos rel_filme;

                    while (offset_filme != -1) {
                        fseek(arq_grafo, offset_filme, SEEK_SET);
                        fread(&rel_filme, sizeof(Relacionamentos), 1, arq_grafo);

                        int passa_filtro_1 = 0;
                        if (tipo_filtro == 1) passa_filtro_1 = 1; 
                        else if (tipo_filtro == 2 && (strcmp(rel_filme.tipo_relacao, "ACTED_IN") == 0 || strcmp(rel_filme.tipo_relacao, "DIRECTED") == 0)) passa_filtro_1 = 1; 
                        else if (tipo_filtro == 3 && strcmp(rel_filme.tipo_relacao, "ACTED_IN") == 0) passa_filtro_1 = 1; 

                        if (passa_filtro_1) {
                            TARVBM *no_filme = TARVBM_busca(arq_indices, offset_raiz, rel_filme.nome_destino, t);
                            if (no_filme) {
                                int j = 0;
                                while (j < no_filme->nchaves && strcmp(no_filme->chave[j], rel_filme.nome_destino) != 0) j++;

                                char arq_folha_f[50];
                                sprintf(arq_folha_f, "bin/folha_%d.bin", no_filme->filho[0]);
                                FILE *f_folha_filme = fopen(arq_folha_f, "rb");

                                if (f_folha_filme) {
                                    dados d_filme;
                                    fseek(f_folha_filme, j * sizeof(dados), SEEK_SET);
                                    fread(&d_filme, sizeof(dados), 1, f_folha_filme);
                                    fclose(f_folha_filme);

                                    long offset_colega = d_filme.offset_prim_viz;
                                    Relacionamentos rel_colega;

                                    while (offset_colega != -1) {
                                        fseek(arq_grafo, offset_colega, SEEK_SET);
                                        fread(&rel_colega, sizeof(Relacionamentos), 1, arq_grafo);

                                        int passa_filtro_2 = 0;
                                        if (tipo_filtro == 1) passa_filtro_2 = 1; 
                                        else if (tipo_filtro == 2 && (strcmp(rel_colega.tipo_relacao, "ACTED_IN") == 0 || strcmp(rel_colega.tipo_relacao, "DIRECTED") == 0)) passa_filtro_2 = 1; 
                                        else if (tipo_filtro == 3 && strcmp(rel_colega.tipo_relacao, "ACTED_IN") == 0) passa_filtro_2 = 1; 

                                        if (passa_filtro_2 && strcmp(d_pessoa.nome, rel_colega.nome_destino) < 0) {
                                            printf(" -> %s e %s (em %s)\n", d_pessoa.nome, rel_colega.nome_destino, rel_filme.nome_destino);
                                        }

                                        offset_colega = rel_colega.offset_prox_viz;
                                    }
                                }
                                libera_no(no_filme, t);
                            }
                        }
                        offset_filme = rel_filme.offset_prox_viz;
                    }
                }
            }
            fclose(f_dados);
        }
    }
    libera_no(no, t);
}

void relatorio_duplas(FILE *arq_indices, int offset_raiz, FILE *arq_grafo, int t, int tipo_filtro) {
    printf("\n====================================================\n");
    if (tipo_filtro == 1) printf(" (a) TODOS QUE TRABALHARAM JUNTOS\n");
    else if (tipo_filtro == 2) printf(" (b) ATORES E DIRETORES QUE TRABALHARAM JUNTOS\n");
    else if (tipo_filtro == 3) printf(" (c) ATORES QUE ATUARAM JUNTOS\n");
    printf("====================================================\n");

    listar_duplas_rec(arq_indices, offset_raiz, offset_raiz, arq_grafo, t, tipo_filtro);
    
    printf("====================================================\n\n");
}