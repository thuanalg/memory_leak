SRC_01= *.h share*.c main.c
SRC_02= *.h share*.c main.read.c
SRC_03= *.h share*.c main.write.c
LIBS= -lpthread -lrt
all:
	rm -f main main.read main.write
	gcc -g -o main $(SRC_01) ${LIBS}
	gcc -g -o main.read $(SRC_02) ${LIBS}
	gcc -g -o main.write $(SRC_03) ${LIBS}
