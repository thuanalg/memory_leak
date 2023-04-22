LIBS += -lpthread
SRC_SERVER=server.c gen_list.* msg_notify.c
SRC_CLIENT=client.c gen_list.* msg_notify.c
SRC_MSG_NOT=test_msg_notify.c msg_notify.c
INC +=msg_notify.h
all:
	rm -f main.server main.client main.save
	gcc -g -o main.server ${SRC_SERVER} ${LIBS} ${INC}
	gcc -g -o main.client ${SRC_CLIENT} ${LIBS} ${INC}
	gcc -g -o main.save main.save.bin.c ${LIBS}
	gcc -g -o test.msg.notify ${SRC_MSG_NOT} ${INC}
clean:
	rm -f main.server main.client main.save test.msg.notify
