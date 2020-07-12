#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>

#define BUFFER_SIZE 128

int readline(int fildes, void* buf, int nbytes){     // devolve quantos bytes leu, e escreve no buffer um máximo de nbytes, ou até bater em \n \0 ou EOF

    int size = 0;
    char c;
    char* buff = (char*)buf;

    while (size < nbytes && read(fildes, &c, 1) == 1) {
        buff[size++] = c; 								//guarda o carater

        if (c == '\n'){									//se for \n troca por \0
        	buff[size-1] = '\0';
            return size;
        }
        if (c == '*'){
        	read(fildes, &c, 1);
        	buff[size-1] = '\0';
            return size;
        }

    }
    buff[size] = '\0';
    return size;											// se sair do ciclo é porque nao ha mais nada, eof i guess
}

char* iToa (int i, char* arr){            //itoa mas com 11 caracteres fixos
    int j=0;
    int aux;
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
    arr[11] = '\0';
  return arr;
}


void getStock(char* fstArgument, char* resposta){

	int fdStocks = open("STOCKS", O_RDWR|O_CREAT, 0666);
	int fdArtigos = open("ARTIGOS", O_RDWR|O_CREAT, 0666);

	char* buffArtigos = (char*)malloc(sizeof(char)*BUFFER_SIZE);
	char* buffStocks = (char*)malloc(sizeof(char)*BUFFER_SIZE);

	int offset = ((atoi(fstArgument)-1)*36)+24;
	lseek(fdArtigos,offset,SEEK_CUR);
	readline(fdArtigos, buffArtigos,12);

	offset = ((atoi(fstArgument)-1)*24)+12;
	lseek(fdStocks,offset,SEEK_CUR);
	readline(fdStocks, buffStocks,12);


	//ISTO em baixo PODE SAIR, NAO TA NO ENUNCIADO           ISTO em baixo PODE SAIR, NAO TA NO ENUNCIADO
	strcpy(resposta,"Codigo : ");
	strcat(resposta, fstArgument);
	strcpy(resposta," | ");
	//ISTO em cima PODE SAIR, NAO TA NO ENUNCIADO           ISTO em baixo PODE SAIR, NAO TA NO ENUNCIADO

	strcat(resposta,"Stock : ");
	strcat(resposta,buffStocks);
	strcat(resposta," | Preco : ");
	strcat(resposta,buffArtigos);


	free(buffStocks);
	free(buffArtigos);
	close(fdStocks);
	close(fdArtigos);
}

void registaVenda(char* codigo, char* quantidade, char* montante){
	char space = ' ';
	int fdVendas = open("VENDAS", O_RDWR|O_CREAT, 0666);
	char* miniBuff = (char*)malloc(sizeof(char)*16);
	lseek(fdVendas,0,SEEK_END);
 	int nqs = 0; //int ninguém quer saber, server para guardar o return dos writes e calar o gcc


    iToa(atoi(codigo),miniBuff);
	write(fdVendas, miniBuff, 11);  		// escreve o codigo
	write (fdVendas, &space, 1);     		//espaço

	iToa(atoi(quantidade),miniBuff);
	nqs = write(fdVendas, miniBuff, 11);	// escreve a quantidade
	nqs = write (fdVendas, &space, 1);          //espaço



    nqs = write(fdVendas, montante, 11);		// escreve o montante total
   	nqs += write(fdVendas, "\n" ,1);          // paragrafo

	close(fdVendas);
	free(miniBuff);
}

void alteraStock(char* codigo,char* stockDif,char* resposta){
	int control = 0;
	int fdStocks = open("STOCKS", O_RDWR|O_CREAT, 0666);
	int fdArtigos = open("ARTIGOS", O_RDWR|O_CREAT, 0666);

	char* buffArtigos = (char*)malloc(sizeof(char)*BUFFER_SIZE);
	char* buffStocks = (char*)malloc(sizeof(char)*BUFFER_SIZE);
	char * novoStock = (char*)malloc(sizeof(char)*BUFFER_SIZE);
	char * montante = (char*)malloc(sizeof(char)*BUFFER_SIZE);

	int offset = ((atoi(codigo)-1)*36); 		//info do artigos
	lseek(fdArtigos,offset,SEEK_SET);
	readline(fdArtigos, buffArtigos,36);
	char* preco = buffArtigos+24;
	int prec = atoi(preco);

	offset = ((atoi(codigo)-1)*24); 			 //info dos stocks
	lseek(fdStocks,offset,SEEK_SET);
	readline(fdStocks, buffStocks,24);
	char* stock = buffStocks+12;
	int stockActual = atoi(stock);

	int stockAmudar = 0;

	if(stockDif[0]=='-'){         				//caso seja possivel venda
		stockAmudar=atoi(stockDif+1);
		if (stockActual >= stockAmudar){  		//venda valida
			iToa( stockActual - stockAmudar, novoStock);
			strncpy(buffStocks+12,novoStock,11);
			offset = ((atoi(codigo)-1)*24)+12;
			lseek(fdStocks,offset,SEEK_SET);
			write(fdStocks,novoStock,11);
			iToa((stockAmudar * prec),montante);
			registaVenda(codigo,stockDif+1,montante);

		}
		else{                 					 //venda invalida
			control = -1;
		}
	}
	else{										//caso seja aumento stock
		stockAmudar=atoi(stockDif);
		iToa(stockAmudar + stockActual, novoStock);
		strncpy(buffStocks+12,novoStock,11);
		offset = ((atoi(codigo)-1)*24)+12;
		lseek(fdStocks,offset,SEEK_SET);
		write(fdStocks,novoStock,11);
	}

	if (control==-1){						// tratar da resposta
		strcpy(resposta,"stock invalido");
	}

	else{

		// VV no need de isto em baixo VV VV no need de isto em baixo VVVV no need de isto em baixo VVVV no need de isto em baixo VV
		strcpy(resposta,"Codigo Produto : ");
		strcat(resposta,codigo);
		strcat(resposta," | ");
		// ^^no need de isto em cima^^  ^^no need de isto em cima^^ ^^no need de isto em cima^^
		strcat(resposta,"Novo Stock : ");
		strcat(resposta,novoStock);
	}
	free(novoStock);
	free(buffStocks);
	free(buffArtigos);
	free(montante);
	close(fdStocks);
	close(fdArtigos);
}

void execComando(char* comando){
	char* fifoCliente = strtok(comando," ");
	char* fstArgument =strtok(NULL," ");
	char* sndArgument = strtok(NULL,"");
	char * resposta = (char*) malloc(sizeof(char)*BUFFER_SIZE);

	if (sndArgument==NULL){
		getStock(fstArgument,resposta);
	}
	else{
		alteraStock(fstArgument,sndArgument,resposta);
	}


    int fdcliente = open(fifoCliente, O_WRONLY);
	write(fdcliente, resposta,strlen(resposta)+1);
	close (fdcliente);
	free(resposta);
}

int main(int argc, char* argv[]) {
	remove("/tmp/fifosv");  // caso o sv ja tenha sido corrido para evitar ler lixo
	int clientenr = 1;
	int bytesRead = 1;
	char* bufferComando = NULL;

	write(1,"\nSERVER RUNNING\n\n",19);  ///    ----teste    ----teste ----teste ----teste ----teste

	char* fifosv = "/tmp/fifosv";
    mkfifo(fifosv, 0666);
    int fdservidor = open(fifosv, O_RDONLY);
    bufferComando = (char*) malloc(sizeof(char)*BUFFER_SIZE);


    while(1){
    	memset(bufferComando,0,BUFFER_SIZE);
    	bytesRead  =  readline(fdservidor, bufferComando, BUFFER_SIZE);

    	if (bytesRead > 1){
    		clientenr++;
    		execComando(bufferComando);
    	}
    	else
    		sleep(0.01);  //se nao ler nada dorme 0.01 segundos, so para nao tar a destruir cpu para nada
    }
    free(bufferComando);
    close(fdservidor);
    return 123;
}
