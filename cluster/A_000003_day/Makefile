target=lcs_A_000003_day
deb_mod=-g -Wall
rel_mod=-DLCS_RELEASE_MOD
#rel_mod=-DLCS_RELEASE_MOD -s
build_mod=${deb_mod}
#build_mod=${rel_mod}
HEADER_FILE=lcs_common.h
SRC_FILE=../common/lcs_common.c a_000003_day.c

lib=`mysql_config --libs` -lpthread -lsqlite3
include=`mysql_config --cflags` -I../common/

all: 
	gcc ${build_mod} -o ${target} ${SRC_FILE} ${lib} ${include}
	cp -f ${target} ../bin/
clean:
	rm -f ${target} *.o
