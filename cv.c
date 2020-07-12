#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define SIZE 128



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

void lerresposta(char* comando){
    char * myfifo = strtok(comando," ");
    char * resposta=(char*) malloc(sizeof(char)*SIZE);
    int bytesRead;
    int fdcliente = open (myfifo, O_RDONLY);                 // esperar por resposta
    bytesRead = read(fdcliente ,resposta, SIZE);
    write(1,"RESPOSTA RECEBIDA : ",20);
    write(1,resposta,bytesRead);
    write(1,"\n",1);
    free(resposta);
    close(fdcliente);
    remove(myfifo);
}

void menuCL (char* firstArgument, char* secondArgument){
	//vars
	char*  mypid = (char*)  malloc(sizeof(char)*SIZE);
    int fdservidor = 0;
    char* comando= (char*) malloc(sizeof(char)*SIZE);

    //abre fifoServer/cria o fifoCliente ta aqui para mudar a ordem
	char*  fifosv = "/tmp/fifosv";

    //constroi o fifoCliente
	pid_t pi = getpid();
    sprintf(mypid,"%d",(int)pi);  // atoi baiscamente
    strcpy(comando,"/tmp/");
    strcat(comando,mypid);
    mkfifo(comando, 0666);     //cria o fifoCliente ta aqui para mudar a ordem------------
    strcat(comando," ");
    strcat(comando,firstArgument);
    if(secondArgument!=NULL){
        strcat(comando," ");
        strcat(comando,secondArgument);
    }
    strcat(comando,"*");

    //manda o comando para o fifoServer
    fdservidor = open(fifosv, O_WRONLY); //abrir so aqui para haver menos delay
    write(fdservidor,comando,strlen(comando)+1);
    close(fdservidor);

    //resposta
    lerresposta(comando);
    free(comando);
}

int main(int argc, char* argv[]) { //LE A LINHA OU FILE E CHAMA O MENUCL

	int input = 0;
    char* buffer = (char*) malloc(sizeof(char)*SIZE);
    char* firstArgument = NULL;
    char* secondArgument = NULL;
    int bytesRead = 1;
    
    if(argc > 1)                            //estas 2 linhas poupam um total de 2 CHARS!!! na consola
       input= open(argv[1], O_RDONLY);      //"./ma < comandos" passa a "ma comandos". worth it

    bytesRead = readline(input, buffer, SIZE);

    while(bytesRead != -1){
            firstArgument = strtok(buffer, " ");
            secondArgument = strtok(NULL, "");
            menuCL(firstArgument, secondArgument);
            memset(buffer,0,SIZE);
            if(bytesRead == -2)
                bytesRead = -1;
            else
                bytesRead = readline(input, buffer, SIZE);  
    }
    free(buffer); 
    return 0;
}
