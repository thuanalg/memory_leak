LIBS += -lpthread
SRC_SERVER=server.c gen_list.*
all:
	rm -f main.server main.client main.save
	gcc -g -o main.server ${SRC_SERVER} ${LIBS}
	gcc -g -o main.client client.c ${LIBS}
	gcc -g -o main.save main.save.bin.c ${LIBS}
clean:
	rm -f main.server main.client main.save
