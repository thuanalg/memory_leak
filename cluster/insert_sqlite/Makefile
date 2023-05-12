target=lcs_insert_sqlite
deb_mod=-g -Wall
rel_mod=-DLCS_RELEASE_MOD
#rel_mod=-DLCS_RELEASE_MOD -s
build_mod=${deb_mod}
#build_mod=${rel_mod}
HEADER_FILE=lcs_common.h
SRC_FILE=../common/lcs_common.c main.c

lib=-lsqlite3
include=-I../common/

all: 
	gcc ${build_mod} -o ${target} ${SRC_FILE} ${lib} ${include}
	cp -f ${target} ../bin/
clean:
	rm -f ${target} *.o
