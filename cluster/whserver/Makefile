target=lcs_whserver
deb_mod=-g -Wall
rel_mod=-DLCS_RELEASE_MOD
#rel_mod=-DLCS_RELEASE_MOD -s
build_mod=${deb_mod}
#build_mod=${rel_mod}
HEADER_FILE=lcs_common.h lcs_rsa_des.h
SRC_FILE=../common/lcs_common.c ../rsa_des/src/lcs_rsa_des.c main.c

lib=-lrt -lssl -lcrypto -lpthread
include=-I../common/ -I../rsa_des/include

all: 
	gcc ${build_mod} -o ${target} ${SRC_FILE} ${lib} ${include}
	cp -f ${target} ../bin/
clean:
	rm -f ${target} *.o
