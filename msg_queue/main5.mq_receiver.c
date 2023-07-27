#include <mqueue.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define PATH_MQ "/path_mq2"
mqd_t mqdes;
struct mq_attr attr;
struct sigevent sev;

int reg_user_sig();


#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

void handler(int signo, siginfo_t *info, void *context)
{
    ssize_t nr;
    void *buf;
	int sval = 0;
	union  sigval val;
	val = info->si_value;
	sval = val.sival_int;
	fprintf(stdout, "si_value: %d.\n", sval);
	fprintf(stdout, "sending pid: %llu\n", (unsigned long long)info->si_pid);
    if (mq_notify(mqdes, &sev) == -1) {
        handle_error("mq_notify");
    }
    printf("HELLO\n", nr);
   
    /* Determine max. msg size; allocate buffer to receive msg */
    if (mq_getattr(mqdes, &attr) == -1) {
        handle_error("mq_getattr");
    }
    buf = calloc(1, attr.mq_msgsize + 1);
    if (buf == NULL) {
        handle_error("malloc");
    }
    nr = mq_receive(mqdes, buf, attr.mq_msgsize, NULL);
    if (nr == -1) {
        handle_error("mq_receive");
    }
    printf("Read %zd bytes from MQ: buf: %s\n", nr, buf);
    free(buf);
    if(nr >= 10) {
        mq_close(mqdes);
        exit(0);
    }
}


int reg_user_sig() {
	struct sigaction act = { 0 };	
	act.sa_flags = SA_SIGINFO | SA_ONSTACK;
	act.sa_sigaction = &handler;
	if (sigaction(SIGUSR1, &act, NULL) == -1) {
	    perror("sigaction");
	    exit(EXIT_FAILURE);
	}		
}



int main(int argc, char *argv[])
{
    reg_user_sig();
    mqdes = mq_open(PATH_MQ, O_RDONLY | O_CREAT , 0666, NULL);
    if (mqdes == (mqd_t) -1) {
        handle_error("mq_open");
    }
    mq_getattr(mqdes, &attr);
    sev.sigev_notify = SIGEV_SIGNAL;
    //sev.sigev_notify_function = tfunc;
    sev.sigev_notify_attributes = NULL;
    sev.sigev_signo = SIGUSR1;
    sev.sigev_value.sival_ptr = &mqdes;   /* Arg. to thread func. */
    
    if (mq_notify(mqdes, &sev) == -1) {
        handle_error("mq_notify");
    }
    while(1) {
        pause();    /* Process will be terminated by thread function */
    }
    return 0;
}