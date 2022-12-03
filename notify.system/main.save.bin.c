#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
	unsigned long long sysid;
	char devid[128];
	struct timespec cfg;
}NOTIFY_STR;

int main(int argc, char *argv[])
{
	return EXIT_SUCCESS;
}
