//Rafael Teodoro Cruz Ra: 10723258
//Leonardo Aquino Cruz Ra: 10445016
//João Pedro Pereira Monteiro RA:10727509

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma pack(1) // Garante que não haja padding entre os campos da struct

#define LARGURA_MAXIMA 1024
#define ALTURA_MAXIMA 1024
#define TAM_CABECALHO 54 // Tamanho fixo do cabeçalho BMP

// Estrutura para representar um pixel no formato BMP (ordem BGR)
typedef struct {
    unsigned char azul;
    unsigned char verde;
    unsigned char vermelho;
} Pixel;

// Matrizes para imagem original e imagem reconstruída
Pixel imagem[ALTURA_MAXIMA][LARGURA_MAXIMA];
Pixel imagemReconstruida[ALTURA_MAXIMA][LARGURA_MAXIMA];

// Vetores para armazenar os pixels representativos compactados
unsigned char vetorR[LARGURA_MAXIMA * ALTURA_MAXIMA];
unsigned char vetorG[LARGURA_MAXIMA * ALTURA_MAXIMA];
unsigned char vetorB[LARGURA_MAXIMA * ALTURA_MAXIMA];

// Cabeçalho BMP e variáveis auxiliares
unsigned char cabecalho[TAM_CABECALHO];
int largura, altura, padding;
int total_pixels = 0;
int contador_descompactar = 0;

// Verifica se o arquivo lido é um BMP válido
void verificar_bmp_valido() {
    if (cabecalho[0] != 'B' || cabecalho[1] != 'M') {
        fprintf(stderr, "ERRO: Arquivo nao e um BMP valido!\n");
        exit(EXIT_FAILURE);
    }
}

// Verifica se as dimensões da imagem são válidas e dentro dos limites
void verificar_dimensoes() {
    if (largura <= 0 || altura <= 0) {
        fprintf(stderr, "ERRO: Dimensoes invalidas (largura: %d, altura: %d)!\n", largura, altura);
        exit(EXIT_FAILURE);
    }
    if (largura > LARGURA_MAXIMA || altura > ALTURA_MAXIMA) {
        fprintf(stderr, "ERRO: Imagem excede tamanho maximo de %dx%d!\n", 
                LARGURA_MAXIMA, ALTURA_MAXIMA);
        exit(EXIT_FAILURE);
    }
}

// Calcula o padding necessário por linha (múltiplos de 4 bytes)
void calcular_padding() {
    padding = (4 - (largura * 3) % 4) % 4;
}

// Compacta recursivamente uma região da imagem, salvando apenas o pixel central
void compactar(int ini_linha, int fim_linha, int ini_coluna, int fim_coluna) {
    int linhas = fim_linha - ini_linha + 1;
    int colunas = fim_coluna - ini_coluna + 1;

    // Se a região for pequena (<= 3x3), salva o pixel central
    if (linhas <= 3 || colunas <= 3) {
        int meio_linha = (ini_linha + fim_linha) / 2;
        int meio_coluna = (ini_coluna + fim_coluna) / 2;
        Pixel p = imagem[meio_linha][meio_coluna];

        vetorR[total_pixels] = p.vermelho;
        vetorG[total_pixels] = p.verde;
        vetorB[total_pixels] = p.azul;
        total_pixels++;
        return;
    }

    // Caso contrário, divide em 4 quadrantes e compacta cada um
    int meio_linha = (ini_linha + fim_linha) / 2;
    int meio_coluna = (ini_coluna + fim_coluna) / 2;

    compactar(ini_linha, meio_linha, ini_coluna, meio_coluna);
    compactar(ini_linha, meio_linha, meio_coluna + 1, fim_coluna);
    compactar(meio_linha + 1, fim_linha, ini_coluna, meio_coluna);
    compactar(meio_linha + 1, fim_linha, meio_coluna + 1, fim_coluna);
}

// Descompacta a imagem, preenchendo blocos com o pixel representativo salvo
void descompactar(int ini_linha, int fim_linha, int ini_coluna, int fim_coluna) {
    int linhas = fim_linha - ini_linha + 1;
    int colunas = fim_coluna - ini_coluna + 1;

    // Se a região for pequena (<= 3x3), preenche com o pixel representativo
    if (linhas <= 3 || colunas <= 3) {
        Pixel p;
        p.vermelho = vetorR[contador_descompactar];
        p.verde = vetorG[contador_descompactar];
        p.azul = vetorB[contador_descompactar];
        contador_descompactar++;

        for (int i = ini_linha; i <= fim_linha; i++) {
            for (int j = ini_coluna; j <= fim_coluna; j++) {
                imagemReconstruida[i][j] = p;
            }
        }
        return;
    }

    // Caso contrário, divide em 4 quadrantes e descompacta cada um
    int meio_linha = (ini_linha + fim_linha) / 2;
    int meio_coluna = (ini_coluna + fim_coluna) / 2;

    descompactar(ini_linha, meio_linha, ini_coluna, meio_coluna);
    descompactar(ini_linha, meio_linha, meio_coluna + 1, fim_coluna);
    descompactar(meio_linha + 1, fim_linha, ini_coluna, meio_coluna);
    descompactar(meio_linha + 1, fim_linha, meio_coluna + 1, fim_coluna);
}

// Lê a imagem BMP original e armazena em memória
void processar_imagem_original(const char* nome_arquivo) {
    FILE *entrada = fopen(nome_arquivo, "rb");
    if (!entrada) {
        fprintf(stderr, "ERRO: Nao foi possivel abrir %s\n", nome_arquivo);
        exit(EXIT_FAILURE);
    }

    // Lê o cabeçalho BMP
    if (fread(cabecalho, 1, TAM_CABECALHO, entrada) != TAM_CABECALHO) {
        fclose(entrada);
        fprintf(stderr, "ERRO: Nao foi possivel ler o cabecalho BMP\n");
        exit(EXIT_FAILURE);
    }

    verificar_bmp_valido();

    // Extrai largura e altura da imagem
    largura = *(int*)&cabecalho[18];
    altura = *(int*)&cabecalho[22];
    verificar_dimensoes();
    calcular_padding();

    // Lê os pixels (de baixo para cima, como no BMP)
    for (int i = altura - 1; i >= 0; i--) {
        for (int j = 0; j < largura; j++) {
            if (fread(&imagem[i][j], sizeof(Pixel), 1, entrada) != 1) {
                fclose(entrada);
                fprintf(stderr, "ERRO: Falha ao ler pixel [%d][%d]\n", i, j);
                exit(EXIT_FAILURE);
            }
        }
        fseek(entrada, padding, SEEK_CUR); // Ignora o padding
    }
    fclose(entrada);
}

// Salva os dados compactados no arquivo .zmp
void salvar_compactado(const char* nome_arquivo) {
    FILE *compactado = fopen(nome_arquivo, "wb");
    if (!compactado) {
        fprintf(stderr, "ERRO: Nao foi possivel criar %s\n", nome_arquivo);
        exit(EXIT_FAILURE);
    }

    // Escreve o cabeçalho BMP
    fwrite(cabecalho, 1, TAM_CABECALHO, compactado);

    // Escreve os vetores RGB compactados
    for (int i = 0; i < total_pixels; i++) {
        fputc(vetorR[i], compactado);
        fputc(vetorG[i], compactado);
        fputc(vetorB[i], compactado);
    }
    fclose(compactado);
}

// Lê os dados compactados do arquivo .zmp
void ler_compactado(const char* nome_arquivo) {
    FILE *comp = fopen(nome_arquivo, "rb");
    if (!comp) {
        fprintf(stderr, "ERRO: Nao foi possivel abrir %s\n", nome_arquivo);
        fprintf(stderr, "Certifique-se que o arquivo existe e o nome esta correto\n");
        exit(EXIT_FAILURE);
    }

    // Lê o cabeçalho BMP
    if (fread(cabecalho, 1, TAM_CABECALHO, comp) != TAM_CABECALHO) {
        fclose(comp);
        fprintf(stderr, "ERRO: Nao foi possivel ler o cabecalho do arquivo compactado\n");
        exit(EXIT_FAILURE);
    }

    verificar_bmp_valido();

    // Extrai largura e altura da imagem
    largura = *(int*)&cabecalho[18];
    altura = *(int*)&cabecalho[22];
    verificar_dimensoes();
    calcular_padding();

    // Lê os valores RGB representativos até o fim do arquivo
    int i = 0;
    while (!feof(comp)) {
        if (fread(&vetorR[i], 1, 1, comp) != 1) break;
        if (fread(&vetorG[i], 1, 1, comp) != 1) break;
        if (fread(&vetorB[i], 1, 1, comp) != 1) break;
        i++;
    }
    fclose(comp);
}

// Salva a imagem descompactada como um novo arquivo BMP
void salvar_descompactado(const char* nome_arquivo) {
    FILE *saida = fopen(nome_arquivo, "wb");
    if (!saida) {
        fprintf(stderr, "ERRO: Nao foi possivel criar %s\n", nome_arquivo);
        exit(EXIT_FAILURE);
    }

    // Escreve o cabeçalho BMP
    fwrite(cabecalho, 1, TAM_CABECALHO, saida);

    // Escreve os pixels da imagem reconstruída
    for (int i = altura - 1; i >= 0; i--) {
        for (int j = 0; j < largura; j++) {
            fwrite(&imagemReconstruida[i][j], sizeof(Pixel), 1, saida);
        }
        for (int p = 0; p < padding; p++) {
            fputc(0x00, saida); // Preenche o padding com zeros
        }
    }
    fclose(saida);
}

// Função principal que organiza todas as etapas
int main() {
    // 1. Lê a imagem BMP original
    processar_imagem_original("imagemOriginal.bmp");

    // 2. Compacta a imagem usando divisão recursiva (quadtree simplificada)
    compactar(0, altura - 1, 0, largura - 1);
    salvar_compactado("imagemCompactada.zmp");

    // 3. Descompacta a imagem para uma nova matriz
    ler_compactado("imagemCompactada.zmp");
    contador_descompactar = 0;
    descompactar(0, altura - 1, 0, largura - 1);
    salvar_descompactado("imagemDescompactada.bmp");

    // 4. Exibe estatísticas e relatório final
    printf("\nProcessamento concluido com sucesso!\n");
    printf("------------------------------------\n");
    printf("Arquivos gerados:\n");
    printf("- imagemCompactada.zmp (arquivo compactado)\n");
    printf("- imagemDescompactada.bmp (imagem reconstruida)\n");
    printf("\nEstatisticas:\n");
    printf("- Dimensoes originais: %dx%d pixels\n", largura, altura);
    printf("- Total de pixels representativos: %d\n", total_pixels);
    printf("- Taxa de compactacao: %.2f%%\n", 
           (1 - (float)total_pixels/(largura*altura)) * 100);

    return EXIT_SUCCESS;
}
