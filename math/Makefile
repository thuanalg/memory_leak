all:
	gcc -g -o main main.c
	mv main ./bin/
	g++ -g -o main_combine main_combine.cpp
	mv main_combine ./bin/	
	./bin/main_combine 3 1 50
clean:
	rm ./bin/*
