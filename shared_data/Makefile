#SRC_01= *.h share*.c main.c
SRC_02= *.h share*.c main.read.c
SRC_03= *.h share*.c main.write.c
SRC_04= *.h share*.c main.stop.c
LIBS= -lpthread -lrt
#FLAGS= -DUSING_MUTEX
#FLAGS= -DUSING_SEMAPHORE
FLAGS= -DUSING_RWLOCK
INC=
all:
	rm -f main.read main.write main.stop
	find -type l -delete
	ln -s ../common/*.* .
#	gcc -g -o main $(SRC_01) ${LIBS} ${FLAGS} ${INC}
	gcc -g -o main.read $(SRC_02) ${LIBS} ${FLAGS} ${INC}
	gcc -g -o main.write $(SRC_03) ${LIBS} ${FLAGS} ${INC}
	gcc -g -o main.stop $(SRC_04) ${LIBS} ${FLAGS} ${INC}
clean:
	find -type l -delete
	rm -f main main.read main.write main.stop
