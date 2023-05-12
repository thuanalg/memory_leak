#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <error.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/stat.h> /* For mode constants */
#include <fcntl.h> /* For O_* constants */
#include <lcs_common.h>
/***********************************************************************************/
#ifdef LCS_RELEASE_MOD

#endif
/***********************************************************************************/
int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		return EXIT_FAILURE;
	}
	shm_unlink(argv[1]);
	return EXIT_SUCCESS;
}
/***********************************************************************************/
