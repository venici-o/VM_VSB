#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

enum { PAGINAS = 256, TAM = 256 };
#ifndef FRAMES
#define FRAMES 256
#endif

static int tabela[PAGINAS], usados;
static int8_t memoria[FRAMES][TAM];
static size_t total, faltas, hits;

static int politica(const char *s)
{
    return !strcmp(s, "fifo") || !strcmp(s, "lru");
}

/* O arquivo do enunciado começa com BOM e usa CRLF. */
static int endereco(const char *s, size_t n, int primeiro)
{
    size_t i = primeiro && n >= 3 && !memcmp(s, "\xef\xbb\xbf", 3) ? 3 : 0;
    int valor = 0;
    if (n && s[n-1] == '\n') --n;
    if (n && s[n-1] == '\r') --n;
    if (i == n) return -1;
    for (; i < n; ++i) {
        if (s[i] < '0' || s[i] > '9') return -1;
        valor = valor * 10 + s[i] - '0';
        if (valor >= PAGINAS * TAM) return -1;
    }
    return valor;
}

static int traduzir(int addr, FILE *store, FILE *saida)
{
    int pagina = addr / TAM, offset = addr % TAM;
    int frame = tabela[pagina];
    if (frame < 0) {
        frame = usados++;
        if (fseek(store, pagina*TAM, SEEK_SET) ||
            fread(memoria[frame], 1, TAM, store) != TAM) {
            fprintf(stderr, "Falha ao ler a página %d do backing store.\n", pagina);
            return 0;
        }
        tabela[pagina] = frame;
        ++faltas;
    }
    return fprintf(saida, "Virtual address: %d TLB: %d Physical address: %d Value: %d\n",
                   addr, -1, frame*TAM + offset, memoria[frame][offset]) >= 0;
}

int main(int argc, char **argv)
{
    if (argc != 4 || !politica(argv[2]) || !politica(argv[3])) {
        fprintf(stderr, "Uso: ./vm arquivo fifo|lru fifo|lru\n");
        return 1;
    }
    FILE *entrada = fopen(argv[1], "rb");
    if (!entrada) { perror(argv[1]); return 1; }
    FILE *store = fopen("BACKING_STORE.bin", "rb");
    if (!store) { perror("BACKING_STORE.bin"); fclose(entrada); return 1; }
    FILE *saida = tmpfile();
    int ok = 0;
    char *linha = NULL;
    size_t cap = 0;
    ssize_t n;
    if (!saida) { perror("arquivo temporário"); goto fim; }
    for (int i = 0; i < PAGINAS; ++i) tabela[i] = -1;
    while ((n = getline(&linha, &cap, entrada)) >= 0) {
        int addr = endereco(linha, (size_t)n, total == 0);
        if (addr < 0) {
            fprintf(stderr, "Endereço inválido na linha %zu.\n", total+1);
            goto fim;
        }
        ++total;
        if (!traduzir(addr, store, saida)) goto fim;
    }
    if (!feof(entrada) || ferror(entrada)) { perror("leitura da entrada"); goto fim; }
    if (!total) { fprintf(stderr, "Arquivo de endereços vazio.\n"); goto fim; }
    if (fprintf(saida, "Number of Translated Addresses = %zu\nPage Faults = %zu\n"
                "Page Fault Rate = %.3f\nTLB Hits = %zu\nTLB Hit Rate = %.3f\n",
                total, faltas, (double)faltas/total, hits, (double)hits/total) < 0 ||
        fseek(saida, 0, SEEK_SET)) { perror("saída temporária"); goto fim; }
    FILE *dest = fopen("correct.txt", "wb");
    if (!dest) { perror("correct.txt"); goto fim; }
    char buf[4096];
    size_t lidos;
    while ((lidos = fread(buf, 1, sizeof buf, saida)) &&
           fwrite(buf, 1, lidos, dest) == lidos) {}
    ok = !ferror(saida) && !ferror(dest);
    if (fclose(dest)) ok = 0;
    if (!ok) perror("correct.txt");
fim:
    free(linha);
    if (saida) fclose(saida);
    fclose(store);
    fclose(entrada);
    return !ok;
}
