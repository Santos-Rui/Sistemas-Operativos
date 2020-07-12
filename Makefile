CC = gcc -g
CFLAGS = -Wall 


all: ma sv ag cv clean2

cv:
	$(CC) $(CFLAGS) cv.c -o cv	

ma: 
	$(CC) $(CFLAGS) ma.c -o ma

sv: 
	$(CC) $(CFLAGS) sv.c -o sv

ag: 
	$(CC) $(CFLAGS) ag.c -o ag

clean2:
	rm ARTIGOS
	rm STOCKS
	rm STRINGS
	rm VENDAS
clean:
	rm ma sv ag cv
	rm ARTIGOS
	rm STOCKS
	rm STRINGS
	rm VENDAS
