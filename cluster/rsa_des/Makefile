TARGET=lcs_ened_crypto
LIBS=-lpthread -lssl -lcrypto
#CFLAGS=-D__HIDE_SYMBOL___
CFLAGS=-g
all:
	gcc ${CFLAGS} -o ${TARGET} main.c ${LIBS}
	cp -f ${TARGET} ../bin/
clean:
	rm -f ${TARGET} *.o
