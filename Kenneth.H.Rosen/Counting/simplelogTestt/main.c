#include "simplelog.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
	int n = 0, ret = 0;
	char nameday[64];
	consimplelog("Test consimplelog   ");
	char pathcfg[1024];
	char* path = "D:\\z.en\\en.2022.07.08\\memory_leak\\Kenneth.H.Rosen\\Counting\\simplelog\\simplelog.cfg";
	snprintf(pathcfg, 1024, path);
	n = strlen(pathcfg);
	for (int i = 0; i < n; ++i) {
		if (pathcfg[i] == '\\') {
			pathcfg[i] = '/';
		}
	}
	ret = simple_init_log(pathcfg);
	consimplelog("\nret: %d. now: %llu\n", ret, simple_log_time_now(0));
	simple_log_name_now(nameday);
	return EXIT_SUCCESS;
}