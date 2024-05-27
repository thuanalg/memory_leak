#include "simplelog.h"
#include <stdio.h>
//========================================================================================
#define				SPLOG_PATHFOLDR					"pathfoder="
#define				SPLOG_LEVEL						"level="
static const char*				__splog_pathfolder[]		= { SPLOG_PATHFOLDR, SPLOG_LEVEL, 0 };
static	int						simple_log_levwel			=			0;
static	SIMPLE_LOG_ST			__simple_log_static__;;

static int	simple_init_log_parse(char* buff, char* key);
//========================================================================================
int simple_set_log_levwel(int val) {
	simple_log_levwel = val;
	__simple_log_static__.llevel = val;
	return 0;
}
//========================================================================================
int simple_get_log_levwel() {
	int ret = 0;
	ret = __simple_log_static__.llevel;
	return ret;
}
//========================================================================================

int	simple_init_log_parse(char* buff, char *key) {
	int ret = 0;
	do {
		if (strcmp(key, SPLOG_PATHFOLDR) == 0) {
			snprintf(__simple_log_static__.folder, 1024, "%s", buff);
			break;
		}
		if (strcmp(key, SPLOG_LEVEL) == 0) {
			int n = 0;
			int count = 0;
			count = sscanf(buff, "%d", &n);
			__simple_log_static__.llevel = n;
			break;
		}
	} while (0);
	return ret;
}
int	simple_init_log( char *pathcfg) {
	int ret = 0;
	FILE* fp = 0;
	char c = 0;
	int count = 0;
	char buf[1024];
	do {
		memset(buf, 0, sizeof(buf));
		fp = fopen(pathcfg, "r");
		if (!fp) {
			ret = 1;
			consimplelog("Cannot open file error.");
			break;
		}
		while (c != EOF) {
			c = fgetc(fp);
			if (c == '\r' || c == '\n') {
				if (count > 0) {
					int  j = 0;
					char* node = 0;
					while (1) {
						node = __splog_pathfolder[j];
						if (!node) {
							break;
						}
						if (strstr(buf, node))
						{
							consimplelog("Find out the keyword: %s, %s.", node, buf + strlen(node));
							simple_init_log_parse(buf + strlen(node), node);
							break;
						}
						j++;
					}
				}
				count = 0;
				memset(buf, 0, sizeof(buf));
				continue;
				
			}
			//if (c == '\n') {
			//	count = 0;
			//	count = 0;
			//	memset(buf, 0, sizeof(buf));
			//	continue;
			//}
			buf[count++] = c;

		}
	} while (0);
	if (fp) {
		ret = fclose(fp);
		consimplelog("Error, close file got trouble, error: ret: %d.\n", ret);
	}
	return ret;
}
//========================================================================================
