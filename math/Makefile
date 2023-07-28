all:
	gcc -g -o main main.c
	mv main ./bin/
	g++ -g -o main_combine main_combine.cpp
	mv main_combine ./bin/	
	gcc -g -o pascalg pascal.c
	mv pascalg ./bin/	 
	gcc -g -o duplex duplex.c -lrt
	mv duplex ./bin/
clean:
	rm ./bin/*

