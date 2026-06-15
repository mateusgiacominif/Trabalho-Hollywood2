# Trabalho-Hollywood2
# Alterações Implementadas - Offset e ID nos Nós Folha

## Resumo
Implementação de **offset** (posição no arquivo original) e **ID** (identificador único) nos nós folha da Árvore B, permitindo rastrear a origem dos dados e acessá-los rapidamente pelo identificador.

## Alterações Mínimas Realizadas

### 1. **TARVBM_trab.h** - Estrutura de Dados
```c
typedef struct arvbm{
  int nchaves, folha, *filho, prox;
  char **chave;
  int *offset_chave;    // NEW: offset no arquivo original para cada chave
  int *id_folha;        // NEW: ID do nó folha para cada chave
}TARVBM;
```

**Alteração**: Adicionados dois novos campos à estrutura `TARVBM`:
- `offset_chave`: Array que armazena a posição no arquivo original para cada chave
- `id_folha`: Array que armazena o ID único do nó folha para cada chave

---

### 2. **TARVBM_trab.c** - Funções Atualizadas

#### A) `tamanho_no(int t)`
- **Antes**: Calculava tamanho sem os novos campos
- **Depois**: Adiciona 2 arrays de `(2*t-1)` inteiros cada = `2 * (2*t-1) * sizeof(int)`

#### B) `TARVBM_cria(char *arq, int t)`
- **Antes**: Inicializava nó com apenas nchaves, folha, prox, chaves e filhos
- **Depois**: Agora também inicializa offset_chave (-1) e id_folha (0) como vazios

#### C) `le_no(FILE *fp, int offset, int t)` 
- **Antes**: Lia apenas chaves e filhos
- **Depois**: Agora também aloca e lê `offset_chave` e `id_folha` do arquivo

#### D) **Nova Função** `libera_no(TARVBM *no, int t)`
- Centraliza a liberação de memória incluindo os novos campos
- Evita duplicação de código em múltiplas funções

#### E) `TARVBM_busca()` - Uso de `libera_no()`
- Simplificada para usar a nova função de liberação

#### F) `aloca(FILE *fp, int offset, int t)`
- **Antes**: Alocava espaço vazio para nó folha
- **Depois**: Agora também escreve arrays vazios para offset_chave (-1) e id_folha (0)

#### G) `insere_nao_completo()` - **Assinatura Alterada**
```c
// Antes:
void insere_nao_completo(FILE *fp, int offset, char *nome, int t)

// Depois:
void insere_nao_completo(FILE *fp, int offset, char *nome, int offset_arquivo, int id_folha, int t)
```

**Alterações internas**:
- Ao inserir em nó folha: salva `offset_arquivo` e `id_folha` para a chave
- Ao deslocar chaves: também desloca os offsets e IDs correspondentes
- Agora escreve os novos campos no arquivo após guardar as chaves

#### H) `divisao()` - Cópia de Offsets e IDs
- Quando divide um nó folha, copia os `offset_chave` e `id_folha` para as chaves movidas
- Limpa os valores (-1 e 0) das chaves mantidas no nó original
- Escreve os novos campos para todos os 3 nós (pai, filho dividido, novo nó)

#### I) `TARVBM_insere()` - **Assinatura Alterada**
```c
// Antes:
int TARVBM_insere(FILE *fp, int offset, char *nome, int t)

// Depois:
int TARVBM_insere(FILE *fp, int offset, char *nome, int offset_arquivo, int id_folha, int t)
```

---

### 3. **main.c** - Rastreamento de Offset e ID

**Alterações**:
- Adicionado contador `id_folha_contador` que incrementa a cada inserção
- Adicionada variável `offset_arquivo` que rastreia a posição de leitura em `Nodes.txt`
- Atualizado para chamar `TARVBM_insere()` com os novos parâmetros

```c
int id_folha_contador = 0;
long offset_arquivo = 0;

while(fgets(linha, N, arq_nodes)){
    // ... divide linha ...
    offset = TARVBM_insere(arq_indices, offset, nome, offset_arquivo, id_folha_contador, t);
    id_folha_contador++;
    offset_arquivo = ftell(arq_nodes);  // Atualiza offset para próxima leitura
}
```

---

## Como Usar

### Acessar Offset e ID de uma Chave

Após buscar um nó folha com `TARVBM_busca()`:

```c
TARVBM *folha = TARVBM_busca("bin/indices", "Tom Hanks", t);

if (folha && folha->folha) {
    // Encontrar índice da chave
    for (int i = 0; i < folha->nchaves; i++) {
        if (strcmp(folha->chave[i], "Tom Hanks") == 0) {
            printf("Offset no arquivo: %d\n", folha->offset_chave[i]);
            printf("ID da folha: %d\n", folha->id_folha[i]);
            break;
        }
    }
}
```

---

## Estrutura do Arquivo de Índices

Cada nó agora armazena, em ordem:
1. `nchaves` (int)
2. `folha` (int)
3. `prox` (int)
4. Chaves (char[150] × (2t-1))
5. Filhos (int × 2t)
6. **Offsets das chaves** (int × (2t-1)) ← NEW
7. **IDs das folhas** (int × (2t-1)) ← NEW

---

## Benefícios

✅ **Rastreabilidade**: Sabe-se exatamente onde cada dado vem no arquivo original  
✅ **Acesso Direto**: ID do nó permite recuperar dados rapidamente  
✅ **Mínimas Alterações**: Apenas 2 campos adicionados, funcionalidade original preservada  
✅ **Compatível**: Código existente continua funcionando, apenas com novos parâmetros  

---

## Compilação

```bash
gcc -c TARVBM_trab.c -o TARVBM_trab.o
gcc -c main.c -o main.o
gcc TARVBM_trab.o main.o -o bin/hollywood
```
