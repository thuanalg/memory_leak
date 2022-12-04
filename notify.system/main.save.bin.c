#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


typedef struct {
	unsigned long long sysid;
	char devid[128];
	struct timespec cfg;
} NOTIFY_ST;
#define notify_st NOTIFY_ST


int main(int argc, char *argv[])
{
	int i = 0;
	int n = 0;
	notify_st item;
	FILE *fp = fopen ("./mysave", "wb");
	do {
		memset(&item, 0, sizeof(item));
		item.sysid = (unsigned long long)i + 2;
		n = fwrite((char*) &item, 1, sizeof(item), fp);
		if(n < 1) {
			break;
		}
		fprintf(stdout, "(n, item)=(%d, %d).\n", n, sizeof(item));
	} while ( (++i) < 10);
	if(fp)
	{
		fclose(fp);
	}

	fp = fopen ("./mysave", "rb");
	do {
		int err = 0;
		char *data = 0;
		notify_st *p = 0;
		int m = 0;
		if(!fp) break;
		err = fseek(fp, 0, SEEK_END);
		n = ftell(fp);
		rewind(fp);
		data = (char*) malloc(n+1);
		memset(data, 0, n+1);
		n = fread(data, 1, n, fp);	
		m = n/sizeof(item);
		p = (notify_st*) data;
		for(i = 0; i < m; ++i)
		{
			fprintf(stdout, "(i, item[%d].sysid)=(%d, %d).\n", i, i, p[i].sysid);
		}
	} while(0);
	if(fp) {
		fclose(fp);
	}
	return EXIT_SUCCESS;
}
