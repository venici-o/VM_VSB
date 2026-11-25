# Memória virtual

Simulador com 128 frames, páginas de 256 bytes e 16 entradas na TLB.
Requer um compilador C e threads POSIX.

Com os arquivos do professor no diretório acima de `vsb`, execute **dentro de `vsb`**:

```sh
cp ../BACKING_STORE.bin .
make clean
make
./vm ../addresses.txt lru fifo
tail -5 correct.txt
```

O primeiro algoritmo controla as páginas; o segundo, a TLB. Ambos aceitam
`fifo` ou `lru`. A entrada contém um endereço decimal de 0 a 65535 por linha.
`correct.txt` é gerado no diretório atual; mantenha a referência original fora dele.
