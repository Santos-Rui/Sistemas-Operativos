#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <sys/wait.h>

#define Max 1024
#define PAR "\n"
#define PARN 1


int nqs = 0;
int counter = 0;


char* iToa (int i){                         //itoa mas com 11 caracteres fixos

    int j=0;
    int aux;
    char* arr = (char*) malloc(sizeof(char)*11);

    while(j < 11){ // alterei isto e o malloc para 11 para nao ter que mudar o resto do codigo todo
        arr[j] = '0';
        j++;
    }
    j--; // sem este j-- tu incrementavas o j no while a ultima vez, ele nao entrava no ciclo porque ja passava da posiçao max do array, mas depois escrevias nesse j
    while(i){
        aux = i % 10;
        i = i/10;
        arr[j] = aux + '0';
        j--;
    }
  return arr;
}

char* getTime(char* nome) {


    struct tm *local;
    time_t t;
    t=time(NULL);
    local=localtime(&t);

    sprintf(nome,"%d-%d-%dT%d:%d:%d",1900+local->tm_year,1+local->tm_mon,local->tm_mday,local->tm_hour,local->tm_min,local->tm_sec);
return nome;
}


typedef struct venda{
    char* contador;
    char* quantidade;
    char* montante;
}*Venda;


typedef struct array{
    Venda array[Max];
}*Array;


char* getFstArg(char* linha) {
    return iToa(atoi(linha));
}

char* getSecArg(char* linha) {
    return iToa(atoi(linha+12));
}

char* getThirdArg(char* linha) {
    return iToa(atoi(linha+24));
}

Venda criaVenda(char* linha) {
    Venda v = malloc(sizeof(struct venda));
    v->contador = strdup(getFstArg(linha));
    v->quantidade = strdup(getSecArg(linha));
    v->montante = strdup(getThirdArg(linha));
    return v;
}

void apagaVenda(Venda v) {
    free(v->contador);
    free(v->quantidade);
    free(v->montante);
    free(v);
}

Venda add(Venda v1, Venda v2) {
    int q1,q2,m1,m2;
    q1 = atoi(v1->quantidade);
    q2 = atoi(v2->quantidade);
    m1 = atoi(v1->montante);
    m2 = atoi(v2->montante);
    q1 = q1+q2; m1 = m1+m2;
    v1->quantidade = iToa (q1);
    v1->montante = iToa(m1);
    return v1;
}

Array atualizaAgrega(Array a, char* linha) {
    Venda v = criaVenda(linha);

    for(int i=0; i<Max; i++) {
        if (a->array[i]==NULL) {a->array[i]=v;return a;}
        if ((strcmp(v->contador,a->array[i]->contador))==0) {
                a->array[i]=add(a->array[i],v);
                return a;
            }
    }
    apagaVenda(v);
    return a;
}


void escreveArray(Array a, char* nome) {
    int i,x;
    char space = ' ';

    int fdAgregador  = open(getTime(nome), O_RDWR|O_CREAT, 0666);

    for(i = 0; a->array[i]!=NULL; i++) {

        for(x=0; a->array[i]->contador[x]!='\0'; x++) {
            nqs += write(fdAgregador, &(a->array[i]->contador[x]), 1);
        }

        nqs += write(fdAgregador, &space, 1);

        for(x=0; a->array[i]->quantidade[x]!='\0'; x++) {
            nqs += write(fdAgregador, &(a->array[i]->quantidade[x]), 1);
        }

        nqs += write(fdAgregador, &space, 1);

        for(x=0; a->array[i]->montante[x]!='\0'; x++) {
            nqs += write(fdAgregador, &(a->array[i]->montante[x]), 1);
        }

        nqs += write(fdAgregador, PAR ,PARN);
    }
    close(fdAgregador);
}

int readline(int fildes, void* buf, size_t nbytes){     // devolve quantos bytes leu, e escreve no buffer um máximo de nbytes, ou até bater em \n \0 ou EOF

    int size = 0;
    char c;
    char* buff = (char*)buf;

    while (size < nbytes && read(fildes, &c, 1) == 1) {
        buff[size++] = c;                               //guarda o carater

        if (c == '\0'){                                 // se for \0 fica
            return -1;
        }

        if (c == '\n'){                                 //se for \n troca por \0
            buff[size-1] = '\0';
            return size;
        }

    }
    buff[size] = '\0';
    return -1;                                          // se sair do ciclo é porque nao ha mais nada, eof i guess
}


int equals (Venda a, Venda b){
    if(strcmp(a->contador,b->contador)==0) return 1;
    return 0;
}

Array juntaArrays (Array a, Array b){
    for(int i = 0; a->array[i]; i++){
        for(int j = 0; b->array[j]; j++){
            if(equals(a->array[i],b->array[j])){
                add(a->array[i],b->array[j]);
            }
        }
    }
    return a;
}





Array agrega(Array a, char* file, int i){
    int fdVendas = open(file, O_RDWR|O_CREAT, 0666);
    int offsetINI = 0;
    int offsetFIN = 0;
    int numLinhas = (lseek(fdVendas,0,SEEK_END)) / 36;

    switch(i){
        case 1:
            offsetFIN = (numLinhas / 4)*36;
        break;

        case 2:
            offsetINI = ((numLinhas / 4) + 1)*36;
            offsetFIN = 2*(numLinhas / 4)*36;
        break;

        case 3:
            offsetINI = (2*(numLinhas / 4) + 1)*36;
            offsetFIN = 3*(numLinhas / 4)*36;
        break;

        case 4:
            offsetINI = (3*(numLinhas / 4) + 1)*36;
            offsetFIN = 4*(numLinhas / 4)*36;
        break;
    }

    lseek(fdVendas,0,SEEK_SET);
    lseek(fdVendas,offsetINI,SEEK_CUR);

    char* buffer = (char*) malloc(sizeof(char)*Max);

    while(1 && (offsetINI < offsetFIN)){
        if(readline(fdVendas, buffer, Max)==-1) break;
        counter += 36;
        offsetINI += 36;
        a = atualizaAgrega(a,buffer);
    }
    close(fdVendas);
    return a;
}




Array agregaTotal (char* file){

    Array helper[4];// = (Array)malloc(sizeof(struct array));
    for( int i = 0; i < 4; i++) helper[i] = (Array)malloc(sizeof(struct array));

    for(int i = 0; i < Max; i++)
        for(int j = 0; j < 4; j++)
                helper[j]->array[i] = NULL;

    for( int i = 0; i < 4; i++) {

        if(!fork()) {
            helper[i] = agrega(helper[i],file,i+1);
        }
    }

    for(int i = 0; i < 4; i++) wait(NULL);

    helper[0] = juntaArrays(juntaArrays(helper[0],helper[1]),juntaArrays(helper[2],helper[3]));

    return helper[0];
}







int main(int argc, char* argv[]) {
    char* nome = (char*)malloc(sizeof(char)*60);
    /*
    char* teste = (char*)malloc(sizeof(char)*1);
    char* offsetARR = (char*)malloc(sizeof(char)*11);

    int fdOFFSET = open("OFFSET",O_RDWR|O_CREAT, 0666);

    if(read(fdOFFSET,teste,1)==-1){
        lseek(fdOFFSET,0,SEEK_SET);
        write(fdOFFSET,iToa(0),11);
        lseek(fdOFFSET,-11,SEEK_CUR);
        read(fdOFFSET,offsetARR,11);
    }
    else{
        lseek(fdOFFSET,0,SEEK_END);
        lseek(fdOFFSET,-11,SEEK_CUR);
        read(fdOFFSET,offsetARR,11);
    }



    int offset = atoi(offsetARR); //Offset
    int linhasLidas = offset;

    */

    Array a = agregaTotal(argv[1]);
    escreveArray(a,nome);

    /*
    linhasLidas += counter;

    lseek(fdOFFSET,0,SEEK_END);
    write(fdOFFSET,iToa(linhasLidas),11);

    int fdOUT = open(nome,O_RDWR);
    lseek(fdOUT,0,SEEK_SET);
    char* escrever = (char*)malloc(sizeof(char)*nqs);
    read(fdOUT,escrever,nqs);
    write(1,escrever,nqs);
    */

    //close(fdOUT);
    //close(fdOFFSET);
    return 0;
}
