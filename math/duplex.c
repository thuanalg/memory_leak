#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
//"AB[2CFD]" -->> "ABCFDCFD"
#define ull unsigned long long
void singleplain(char *in, int len, char **out, int *outlen);
int duplex(char *str);
int main(int argc, char *argv[]) {
    ull n = 0;
    sscanf(argv[1], "[%llu]", &n);
    fprintf(stdout, "n: %llu\n", n);
    return 0;
}
int duplex(char *str)
{
    return 0;
}
void singleplain(char *in, int len, char **out, int *outlen)
{

}
//https://edabit.com/challenge/su4fC3zugSBmS5bfq
//https://edabit.com/challenge/Fe9X2DxpSCMun6t5D
//https://edabit.com/challenge/FYKdBjg2kf7sAANiT
//https://edabit.com/challenge/gvcmR4McLyia2PDac
//https://edabit.com/challenge/CFYzzHAmM75pmsGae