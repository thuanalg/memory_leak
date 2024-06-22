#include "simplelog.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
void dotest();
DWORD WINAPI MyThreadFunction(LPVOID lpParam);
int number = 2;
int main(int argc, char* argv[]) {
	int n = 0, ret = 0;
	if (argc > 1) {
		sscanf(argv[1], "%d", &number);
	}
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
	dotest();
	while (1) {
		FILE* fp = 0;
		Sleep(10 * 1000);
		spllog(SPL_LOG_DEBUG, "%s", "\n\n\n  ++++++++++++++ dsds\n\n");
		fp = fopen("D:\\z.en\\en.2022.07.08\\memory_leak\\Kenneth.H.Rosen\\Counting\\simplelog\\trigger.txt", "r");
		if(fp){
			fclose(fp);
			break;
		}

	}
	spllog(SPL_LOG_INFO, "%s", "<<<<<<<-----------------------------------------------------------------------0s\n");
	
	spl_console_log("--Main close--\n");
	spl_finish_log();
	return EXIT_SUCCESS;
}
void dotest() {
	DWORD dwThreadId = 0;
	HANDLE hThread = 0;
	for (int i = 0; i < number; ++i) {

		hThread = CreateThread(
			NULL,                   // default security attributes
			0,                      // use default stack size  
			MyThreadFunction,       // thread function name
			0,          // argument to thread function 
			0,                      // use default creation flags 
			&dwThreadId);   // returns the thread identifier 	
	}
}

DWORD WINAPI MyThreadFunction(LPVOID lpParam) {
	while (1) {
		spllog(SPL_LOG_INFO, "test log: %llu", (LLU)time(0));
		Sleep(1 * 1000);
	}
	return 0;
}