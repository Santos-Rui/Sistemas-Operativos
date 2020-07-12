#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <sys/wait.h>

#define BUFFER_SIZE 128
#define TAMANHO_ARRAY 65536

typedef struct vend {
    int quantidade;
    int montante;
} venda;

char* iToa (int i, char* arr){            //itoa mas com 11 caracteres fixos
    int j=0;
    int aux;
    while(j < 11){ // alterei isto e o malloc para 11 para nao ter que mudar o resto do codigo todo
        arr[j] = '0';
        j++;
    }
    j--;
    while(i){
        aux = i % 10;
        i = i/10;
        arr[j] = aux + '0';
        j--;
    }
    arr[11] = '\0';
    return arr;
}

int readline(int fildes, void* buf, size_t nbytes){     // devolve bytes lidos, -2 se bytes+eof, -1 se eof

    int size = 0;
    char c;
    char* buff = (char*)buf;
    int rd;
    rd = read(fildes, &c, 1);

    while (size < nbytes && rd == 1) {
        buff[size] = c;
                                  //guarda o carater
        if (c == '\n'){                               //se for \n troca por \0
            buff[size] = '\n';
            return size;
        }
        if (c == '*'){                                //isto é para as astrings no fifosv
            buff[size] = '\0';
            return size;
        }
        size++;
        rd = read(fildes, &c, 1);
    }
    if(rd !=1 && size>0){
        return -2;          //caso encontre eof no fim da linha
    }
    return -1;              // se sair do ciclo é porque nao ha mais nada, eof i guess
}

void imprimeVendas(venda * arr){
    char* buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
    for(int i= 0;i<TAMANHO_ARRAY;i++){
        if(arr[i].quantidade>0){
            iToa(i,buffer);
            strcat(buffer," ");
            iToa(arr[i].quantidade,buffer+12);
            strcat(buffer," ");
            iToa(arr[i].montante,buffer+24);
            strcat(buffer,"\n");
            write(1,buffer,36);
            memset(buffer,0,BUFFER_SIZE);
        }
    }
    free(buffer);
}

void registaVenda(venda * arr, char * linha){

    char* firstArgument = strtok(linha, " ");
    char* secondArgument = strtok(NULL, " ");
    char* thirdArgument = strtok(NULL, "");

    int codigo = atoi(firstArgument);
    int quant = atoi(secondArgument);
    int mont = atoi(thirdArgument);

    arr[codigo].quantidade +=  quant;
    arr[codigo].montante += mont;
}

void lerVendas(venda * arr){

    char* buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);
    int bytesRead = 0;
    bytesRead = read(0, buffer, 36);
    while(bytesRead != -1){
            registaVenda(arr, buffer);
            memset(buffer,0,BUFFER_SIZE);

            if(bytesRead == -2)
                bytesRead = -1;

            else
                bytesRead = readline(0, buffer, 36);
    }
    free(buffer);
}

int main(int argc, char* argv[]) {

    venda * arr = malloc(sizeof(venda)*TAMANHO_ARRAY);
    venda v={0,0};
    for(int i= 0;i<TAMANHO_ARRAY;i++){
        arr[i]=v;
    }
    lerVendas(arr);   
    imprimeVendas(arr);
    return 0;
}
