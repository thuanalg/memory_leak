#ifndef _GNU_SOURCE
    #define _GNU_SOURCE
#endif
//#include <linux/getcpu.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int main(int argc, char *argv[]) {
    unsigned int cpu, node;
    long num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
    int k = 0, i = 0;
    getcpu(&cpu, &node);
    fprintf(stdout, "getcpu: %d\n", cpu);
    /* One billion */
    while(i < 1000000000) {
        k = sched_getcpu();
        ++i;
    }
    
    fprintf(stdout, "sched_getcpu: %d\n", k);
    fprintf(stdout, "num_cpus: %ld\n", num_cpus);
    return 0;
}