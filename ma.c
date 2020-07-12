#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <stdio.h>
#include <sys/wait.h>

#define BUFFER_SIZE 128

//i <nome> <preço>         --> insere novo artigo, mostra o código
//n <código> <novo nome>   --> altera nome do artigo
//p <código> <novo preço>  --> altera preço do artigo


int readline(int fildes, void* buf, size_t nbytes){     // devolve bytes lidos, -2 se bytes+eof, -1 se eof

    int size = 0;
    char c;
    char* buff = (char*)buf;
    int rd;
    rd = read(fildes, &c, 1);

    while (size < nbytes && rd == 1) {
        buff[size] = c;                               //guarda o carater
        if (c == '\n'){                               //se for \n troca por \0
            buff[size] = '\0';
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

char* iToa (int i){         //itoa mas com 11 caracteres fixos

	int j=0;
    int aux;
	char* arr = (char*) malloc(sizeof(char)*11);

	while(j < 11){
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

int alteraPreco (char* codigo, char* prec, int fdArtigos){
    int nqs = 0; //int ninguém quer saber, server para guardar o return dos writes e calar o gcc
    size_t nbytes = 36;  //bytes por linha nao sei se este nr está bem
    char * preco = iToa(atoi(prec));

   /* if (fdArtigos == -1) {
        strerror(fdArtigos);
        return EXIT_FAILURE;
    }*/

    int offset = (atoi(codigo)-1)*nbytes;
    lseek(fdArtigos, offset ,SEEK_SET);                                             // Ir para a linha certa
    lseek(fdArtigos, 24, SEEK_CUR);                                                 // Puxa para a frente
    for(int i = 0; preco[i] != '\0'; i++)nqs += write(fdArtigos, &preco[i], 1);     // escreve novo preço

    free(preco);
    return 0;
}

int escreveStrings (char* string, int fdStrings){

    int i=0;
    if (fdStrings == -1) {
        strerror(fdStrings);
        return EXIT_FAILURE;  // não sei mt bem o que estas 2 linhas fazem mas tenho fé
    }
    for(i = 0; i < strlen(string);)
        i+=write(fdStrings, &string[i], 1);

    i+=write(fdStrings, "\n",1);
    return 1;
}

int alteraNome (char * codigo, char * nome, int fdArtigos, int fdStrings){
	int nqs = 0; //int ninguém quer saber, server para guardar o return dos writes e calar o gcc
    size_t nbytes = 36;  //bytes por linha

    lseek(fdStrings,0,SEEK_END);                                  // offset de srtings para o fim
    escreveStrings (nome, fdStrings);                             // escrever nova string no strings
    int offSET = lseek(fdStrings, -(strlen(nome)+1), SEEK_CUR);   // poe o fd no inicio do nome
    char *n = iToa(offSET);                                       // poe o offset num char*

    int offset = (atoi(codigo)-1)*nbytes;                         // offset de artigos o sitio certo
    lseek(fdArtigos, offset ,SEEK_SET);                           // Ir para a linha certa

    lseek(fdArtigos, 12, SEEK_CUR);                               // Puxa para a frente para parte do codigo
    nqs+=write(fdArtigos, n, 11);                                 // escreve novo nome

    free(n);
    return 0;
}

int escreveStocks (int fdStocks, int contador) {
    int nqs = 0; //int ninguém quer saber, server para guardar o return dos writes e calar o gcc
    char space = ' ';
    char* codigo = iToa(contador);
    char* quant = "00000000000";

    lseek(fdStocks,0,SEEK_END);

    for(int i = 0; codigo[i]!='\0';i++){      // escreve o codigo
        nqs = write(fdStocks, &codigo[i], 1);
    }

    nqs = write (fdStocks, &space, 1);

    for(int i = 0; quant[i]!='\0';i++) {      // escreve a quantidade
        nqs = write(fdStocks, &quant[i], 1);
    }

    nqs += write(fdStocks, "\n" ,1);            // paragrafo
    free(codigo);
    return 1;
}

int escreveArtigos (int fdArtigos, int ref, int contador, char* prec){
    int nqs = 0; //int ninguém quer saber, server para guardar o return dos writes e calar o gcc
    char space = ' ';
    char* codigo = iToa(contador);
    char* referencia = iToa (ref);
    int temp = atoi (prec);
    char* preco = iToa(temp); //esta estupidez server para fixar o tamanho do preco

    for(int i = 0; codigo[i]!='\0';i++){                // escreve o codigo
        nqs = write(fdArtigos, &codigo[i], 1);
    }
    nqs = write (fdArtigos, &space, 1);                 //espaço

    for(int i = 0; referencia[i]!='\0';i++){            // escreve a referencia
        nqs = write(fdArtigos, &referencia[i], 1);
    }
    nqs = write (fdArtigos, &space, 1);                 //espaço

    for(int i = 0; preco[i]!='\0';i++) {     // escreve o preço
        nqs = write(fdArtigos, &preco[i], 1);
    }
    nqs += write(fdArtigos, "\n" ,1);            // paragrafo

    free(codigo);
    free(referencia);
    free(preco);
    return 1;
}

char* getTime(char* nome) {
    struct tm *local;
    time_t t;
    t=time(NULL);
    local=localtime(&t);
    sprintf(nome,"%d-%d-%dT%d:%d:%d",1900+local->tm_year,1+local->tm_mon,local->tm_mday,local->tm_hour,local->tm_min,local->tm_sec);
    return nome;
}

void agrega(){

    char * novoNome = (char *) malloc(sizeof(char)*64);
    getTime(novoNome);
	int fdr = open("VENDAS", O_RDWR|O_CREAT, 0666);
	int fdw = open(novoNome, O_RDWR|O_CREAT, 0666);  // é preciso por o nome de DATA

	if(! fork() ){ 					//o filho correr um agreg depo
		dup2(fdr,0);             //novo processo lê daquele file
		dup2(fdw,1);			    //e escreve naquele, falta mudaro nome para a data
		execlp("./ag","./ag", NULL);
	}
	else{
		int status;
		wait(&status);
	}
	close(fdr);
	close(fdw);
}

int escreveCat(char* command, char* firstArgument, char* secondArgument){
    int fdStrings = open("STRINGS", O_RDWR|O_CREAT, 0666);
    int fdArtigos = open("ARTIGOS", O_RDWR|O_CREAT, 0666);
    int fdStocks  = open("STOCKS", O_RDWR|O_CREAT, 0666);
    lseek(fdStrings,0,SEEK_END);								// fdsringfs para o fim, para escrever
    int temp = lseek(fdArtigos,0,SEEK_END) + 1;   				// +1 porque a ultima linha tem menos um char
    int contador = (temp/36)+1;									// proximo contador a escrever
    int offset;
	char option =  * command;
	switch (option){

     	case 'i': 				                     			//i <nome> <preço>        --> insere novo artigo, mostra o código
     		escreveStrings(firstArgument,fdStrings);
     		offset = lseek(fdStrings,-(strlen(firstArgument)+1), SEEK_CUR);   		 //puxa para o inicio no nome
        	escreveArtigos(fdArtigos,offset, contador, secondArgument);              //fd, codigo, preço
        	escreveStocks(fdStocks,contador);
			break;

        case 'n':   						                     //n <código> <novo nome>   --> altera nome do artigo
        	alteraNome(firstArgument,secondArgument,fdArtigos,fdStrings);
        	break;

        case 'p': 	                                            //p <código> <novo preço>  --> altera preço do artigo
       		alteraPreco(firstArgument,secondArgument,fdArtigos);
       		break;
      	case 'a':
       		agrega();
       		break;
    }
    close(fdStrings);
    close(fdArtigos);
    close(fdStocks);
    return 0;
}


int main(int argc, char* argv[]) {

    char* command;
    char* firstArgument = NULL;
    char* secondArgument = NULL;
    int input = 0;
    char* buffer = (char*) malloc(sizeof(char)*BUFFER_SIZE);

    if(argc > 1)                            //estas 2 linhas poupam um total de 2 CHARS!!! na consola
        input= open(argv[1], O_RDONLY);     //"./ma < comandos" passa a "ma comandos". worth it

    int bytesread = readline(input, buffer, BUFFER_SIZE);
    while (bytesread != -1) {
        command = (strtok(buffer, " "));
        firstArgument = strtok(NULL, " ");
        secondArgument = strtok(NULL, "");
        if( (command!= NULL  && firstArgument!=NULL && secondArgument!=NULL) || command[0] == 'a')
            escreveCat(command, firstArgument, secondArgument);
        else
            write(1,"Erro de argumentos\n", 19);
        memset(buffer,0,BUFFER_SIZE);
        if(bytesread == -2)                //truque feio para ler a ultima linha dum file que acaba em eof
            bytesread = -1;
        else
            bytesread = readline(input, buffer, BUFFER_SIZE);
    }
    free(buffer);
    return 0;
}
