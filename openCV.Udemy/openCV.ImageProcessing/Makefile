TARGET=testopencv
LIBS+=$$(pkg-config --libs --cflags opencv4)
SRC=0functions.h 0functions.hpp 0Image.h 0Image.cpp
all: 
	g++ -g -o ${TARGET} 0main.cpp ${SRC} $(LIBS)
clean:
	rm ${TARGET}
