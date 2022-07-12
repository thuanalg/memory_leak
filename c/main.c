#include<stdlib.h>
#include<stdio.h>

void LOG(void *p) {
	//Will embed rsyslog later
	fprintf(stdout, "p:%p\n", p);
}

#define MY_MALLOC(p, n) {p=malloc(n);LOG(p);}
#define MY_FREE(p) {free(p);LOG(p);}


int main(int argc, char *argv) {
	char *str = 0;
	MY_MALLOC(str, 10)
	MY_FREE(str);
	return 0;
}
