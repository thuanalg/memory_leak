LIBS +=-lpthread -lrt -lssl -lcrypto
SRC_SERVER=server.c gen_list.* msg_notify.c
SRC_CLIENT=client.c gen_list.* msg_notify.c
SRC_NOTIFIER=notifier.c gen_list.* msg_notify.c
SRC_MSG_NOT=test_msg_notify.c msg_notify.c
INC +=msg_notify.h
CFLAG +=-Wunused
all:
	rm -f main.server main.client main.save
	gcc -g -o zserver_notify ${SRC_SERVER} ${LIBS} ${INC} ${CFLAG} 
	gcc -g -o zclient_device ${SRC_CLIENT} ${LIBS} ${INC} ${CFLAG}
	gcc -g -o znotifier ${SRC_NOTIFIER} ${LIBS} ${INC} ${CFLAG}
	gcc -g -o zmain.save main.save.bin.c ${LIBS}
	gcc -g -o ztest.msg.notify ${SRC_MSG_NOT} ${INC} ${LIBS}
clean:
	rm -f zserver_notify zclient_device zmain.save ztest.msg.notify znotifier
