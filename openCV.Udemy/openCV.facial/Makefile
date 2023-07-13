TARGET=testopencvfacial
LIBS+=$$(pkg-config --libs --cflags opencv4)
SRC=
all: 
	g++ -g -o ${TARGET} main.cpp ${SRC} $(LIBS)
clean:
	rm ${TARGET}
