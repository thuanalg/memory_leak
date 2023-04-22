#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>


void ntt_daemonize(){
	//http://www.microhowto.info/howto/cause_a_process_to_become_a_daemon_in_c.html
	pid_t ppid = 0;
	ppid = fork();
	if(ppid > 0) {		
		exit(EXIT_SUCCESS);
	}
	setsid();
	signal(SIGHUP, SIG_IGN);

	ppid = fork();
	if(ppid > 0) {		
		exit(EXIT_SUCCESS);
	}
	chdir("/");
	umask(0);
	close(STDOUT_FILENO);
	close(STDIN_FILENO);
	close(STDERR_FILENO);
	open("/dev/null", O_RDONLY);
	open("/dev/null", O_WRONLY);
	open("/dev/null", O_RDWR);
}



#define MY_MALLOC(p, n) {p=malloc(n);syslog(LOG_INFO, "line: %d, malloc p: %p, n: %d\n", __LINE__, p, n); }
#define MY_FREE(p) {free(p);syslog(LOG_INFO, "line: %d, free p: %p\n", __LINE__, p);}


int main(int argc, char *argv) {
	char *str = 0;
	ntt_daemonize();
	fprintf(stdout, "dssds\n");
	fprintf(stdout, "dssds\n");
	fprintf(stdout, "dssds\n");
	fprintf(stdout, "dssds\n");
	fprintf(stdout, "dssds\n");
	fprintf(stdout, "dssds\n");
	setlogmask (LOG_UPTO (LOG_DEBUG));
	openlog ("memory_leak", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
	MY_MALLOC(str, 10)
	MY_FREE(str);
	closelog ();
	return 0;
}


//https://www.gnu.org/software/libc/manual/html_node/Syslog-Example.html
///etc/rsyslog.d
//
