#include<stdlib.h>
#include<stdio.h>
#include <syslog.h>

#define MY_MALLOC(p, n) {p=malloc(n);syslog(LOG_ERR, "line: %d, malloc p: %p, n: %d\n", __LINE__, p, n); }
#define MY_FREE(p) {free(p);syslog(LOG_ERR, "line: %d, free p: %p\n", __LINE__, p);}


int main(int argc, char *argv) {
	char *str = 0;
	setlogmask (LOG_UPTO (LOG_NOTICE));
	openlog ("memory_leak", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
	MY_MALLOC(str, 10)
	MY_FREE(str);
	closelog ();
	return 0;
}


//https://www.gnu.org/software/libc/manual/html_node/Syslog-Example.html
///etc/rsyslog.d
//
