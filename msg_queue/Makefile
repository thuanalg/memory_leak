all:
	gcc -g -o main5.mq_receiver main5.mq_receiver.c -lpthread -lrt
	mv  main5.mq_receiver ./bin/
	gcc -g -o main6.mq_send main6.mq_send.c -lpthread -lrt
	mv  main6.mq_send ./bin/
clean:
	rm ./bin/* 
