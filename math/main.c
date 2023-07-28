#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
//"12 - x = 6"
int evalAlgebra(char *str);
int main(int argc, char *argv[]) {
    char str[100];
    sprintf(str, "%s", argv[1]);
    fprintf(stdout, "evalArg: %d\n", evalAlgebra(str));
    return 0;
}
int evalAlgebra(char *str)
{
    int x = 0;
    int a = 0;
    int b = 0;
    int c = 0;
    
    char *p1 = 0;
    char *p2 = 0;
    char *pc = 0;
    char *pa = 0;
    char *pb = 0;
    char *p0 = strstr(str, "=");
    char sign = 1;

    if(!p0) {
        return 0;
    }
    p0[0] = 0;
    p1 = str;
    p2 = p0 + 1;
    
    
    if(!strstr(p1, "x")) {
        pc = p1;
    } else {
        pc = p2;
        p2 = p1;
    }
    
    sscanf(pc, "%d", &c);
    fprintf(stdout, "c: %d\n", c);
    ///////////////
    fprintf(stdout, "--%s\n", p2);
    fprintf(stdout, "char p2[0] %d\n", (int)p2[0]);
    while(*p2 == 32  && *p2 != 0) {
        ++p2;
    }
    fprintf(stdout, "--%s\n", p2);
    p0 = strstr(p2, " ");
    if(!p0) {
        return 0;
    }
    p0[0] = 0;
    p1 = p2;
    p2 = p0 + 1;
    char buf[20];
    if(!strstr(p1, "x")) {
        pb = p1;
    } else {
        pb = p2;
        p2 = p1;
    }

    if(strstr(pb, " ")) {
        int i = 0;
        memset(buf, 0, sizeof(buf));

        while(*pb) {
            if(*pb != 32) {
                buf[i] = *pb;
                ++i;
            }
            ++pb;
        }
        pb = buf;
    }
    fprintf(stdout, "pb: %s\n", pb);
    sscanf(pb, "%d", &b);
    fprintf(stdout, "b: %d\n", b);

    if(strstr(p2, " ")) {
        int i = 0;
        memset(buf, 0, sizeof(buf));

        while(*p2) {
            if(*p2 != 32) {
                buf[i] = *p2;
                ++i;
            }
            ++p2;
        }
        p2 = buf;
    }

    if(!strcmp(p2, "+x")) {
        a = 1;
    } 
    else if(!strcmp(p2, "-x")) {
        a = -1;
    } else {
        fprintf(stdout, "p2: %s\n", p2);
        sscanf(p2, "%d", &a);
        
    }
    fprintf(stdout, "a: %d\n", a);

    if (a) {
        return (c - b)/a;
    }
    return 0;
}
//https://edabit.com/challenge/su4fC3zugSBmS5bfq
//https://edabit.com/challenge/Fe9X2DxpSCMun6t5D
//https://edabit.com/challenge/FYKdBjg2kf7sAANiT
//https://edabit.com/challenge/gvcmR4McLyia2PDac
//https://edabit.com/challenge/CFYzzHAmM75pmsGae