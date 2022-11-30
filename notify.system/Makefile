LIBS += -lpthread
SRC_SERVER=server.c gen_list.*
all:
	rm -f server client
	gcc -g -o server ${SRC_SERVER} ${LIBS}
	gcc -g -o client client.c ${LIBS}
