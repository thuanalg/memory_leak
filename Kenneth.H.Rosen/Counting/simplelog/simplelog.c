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
int	simple_init_log( char *pathcfg) {
	int ret = 0;
	FILE* fp = 0;
	do {
		fp = fopen(pathcfg, "r");
		if (!fp) {
			ret = 1;
			consimplelog("Cannot open file error.");
			break;
		}

	} while (0);
	if (fp) {
		ret = fclose(fp);
		consimplelog("Error, close file got trouble, error: ret: %d.\n", ret);
	}
	return ret;
}
//========================================================================================
