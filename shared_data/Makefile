#SRC_01= *.h share*.c main.c
SRC_02=../common/share*.c main.read.c
SRC_03=../common/share*.c main.write.c
SRC_04=../common/share*.c main.stop.c
LIBS= -lpthread -lrt
#FLAGS= -DUSING_MUTEX
#FLAGS= -DUSING_SEMAPHORE
FLAGS= -DUSING_RWLOCK
INC=-I../common/
all:
	rm -f main.read main.write main.stop
	gcc -g -o main.read ${SRC_02} ${LIBS} ${FLAGS} ${INC}
	gcc -g -o main.write ${SRC_03} ${LIBS} ${FLAGS} ${INC}
	gcc -g -o main.stop ${SRC_04} ${LIBS} ${FLAGS} ${INC}
clean:
	rm -f main main.read main.write main.stop
