#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <my_global.h>
#include <mysql.h>
#include <pthread.h>
#include <sqlite3.h>
#include <signal.h>
#include <error.h>
#include <time.h>
#include <lcs_common.h>
/***********************************************************************************/
int main(int argc, char *argv[])
{
	int err = 0;
	char **result = 0;
	int i = 0;

	fprintf(stdout, "user: %s\n", getenv("USER"));
	lcs_comm_list_files((char *)"/dev/shm", (char ***)&result, &err);
	if(result)
	{
		while(result[i])
		{
			fprintf(stdout, "file name: %s\n", result[i]);
			free(result[i]);
			++i;
		}
		free(result);
	}
	return EXIT_SUCCESS;
}
/***********************************************************************************/
