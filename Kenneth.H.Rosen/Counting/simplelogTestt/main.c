#include "simplelog.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>

int main(int argc, char* argv[]) {
	int n = 0, ret = 0;
	//char nameday[64];
	spl_console_log("Main thread.\n");
	char pathcfg[1024];
	char* path = "D:\\z.en\\en.2022.07.08\\memory_leak\\Kenneth.H.Rosen\\Counting\\simplelog\\simplelog.cfg";
	char nowfmt[64];
	snprintf(pathcfg, 1024, path);
	n = strlen(pathcfg);
	for (int i = 0; i < n; ++i) {
		if (pathcfg[i] == '\\') {
			pathcfg[i] = '/';
		}
	}
	ret = spl_init_log(pathcfg);
//	spl_console_log("\nret: %d. now: %llu\n", ret, simple_log_time_now(0));
	//simple_log_name_now(nameday);
	spl_fmt_now(nowfmt, 64);
	fprintf(stdout, "\n\n------%s--------\n\n", nowfmt);
	spllog(SPL_LOG_INFO, "%s", "------------>>>>>>>>>>>>>>\n");
	n = 0;
	while (1) {
		++n;
		Sleep(3 * 1000);
		spllog(SPL_LOG_DEBUG, "%s", "  ++++++++++++++ dsds\n");
		if (n > 5) {
			break;
		}
	}
	spllog(SPL_LOG_INFO, "%s", "<<<<<<<-----------------------------------------------------------------------0s\n");
	spl_set_off(1);
	spl_console_log("--Main close--\n");
	return EXIT_SUCCESS;
}