#include "simplelog.h"
#include <stdio.h>
//========================================================================================
static	int						simple_log_levwel			=			0;
//========================================================================================
int simple_set_log_levwel(int val) {
	simple_log_levwel = val;
	return 0;
}
//========================================================================================
int simple_get_log_levwel() {
	return simple_log_levwel;
}
//========================================================================================
int	simple_init_log(int lvel, char *pathFolder) {
	int ret = 0;
	FILE* fp = 0;
	do {
	} while (0);
	if (fp) {
		ret = fclose(fp);
		//consimplelog("Error, close file got trouble, error: ret: %d.\n", ret);
	}
	return ret;
}
//========================================================================================
