LIBS += -lpthread
SRC_SERVER=server.c gen_list.*
all:
	rm -f main.server main.client
	gcc -g -o main.server ${SRC_SERVER} ${LIBS}
	gcc -g -o main.client client.c ${LIBS}
clean:
	rm -f main.server main.client
