#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
//"AB[2CFD]" -->> "ABCFDCFD"

#define ull unsigned long long
typedef struct __lklist__ {
    struct __lklist__ *next;
    int index;
} lklist ;

lklist *lkidx = 0;

void singleplain(char *in, int len, char **out, int *outlen);
void gen_lkidx(char *str, int len);
int duplex(char *str);
int main(int argc, char *argv[]) {
    ull n = 0;
    char *out = 0;
    int outlen = 0;
    sscanf(argv[1], "[%llu]", &n);
    fprintf(stdout, "n: %llu\n", n);
    gen_lkidx(argv[1], strlen(argv[1]));
    //singleplain(argv[1], strlen(argv[1]), &out, &outlen);
    //fprintf(stdout, "out: \"%s\"\noutlen: %d\n", out, outlen);
    return 0;
}
int duplex(char *str)
{
    return 0;
}
void singleplain(char *in, int len, char **out, int *outlen)
{
    ull n = 0;
    int i = 0;
    int j = 0;
    int err = 0;
    ull sz = 0;
    fprintf(stdout, "string len: %d\n", len);
    do {
        if(!in)  { 
            break;
        }
        if(!out)  { 
            break;
        }
        if(!outlen)  { 
            break;
        }
        *outlen = 0;
        sscanf(in, "[%llu]", &n);
        i = 0;
        while(i < len) {
            if(in[i] >= 'A' && in[i] <= 'Z') {
                break;
            }
            if(in[i] >= 'a' && in[i] <= 'z') {
                break;
            }
            ++i;
        }
        fprintf(stdout, "i: %d\n", i);
        if(i >= len) {
            err = 1;
            break;
        }
        j = i;
        while(j < len) {
            if(in[j] < 'A') {
                j--;
                break;
            }
            if(in[j] > 'z') {
                j--;
                break;
            }
            if(in[j] > 'Z' && in[j] < 'a') {
                j--;
                break;
            }
            ++j;
        }
        fprintf(stdout, "j: %d\n", j);
        if(j >= len) {
            err = 1;
            break;
        }
        sz = n * (j - i + 1);
        *out = malloc(sz + 1);
        memset(*out, 0, sz + 1);
        for(int k = 0; k < n; ++k) {
            memcpy(*out + k * (j -i + 1), in + i, (j - i + 1));
            *outlen += (j - i + 1);
        }
    } while(0);
}
void gen_lkidx(char *str, int len) {
    lklist *tmp = 0;
    char *p = 0;
    do {
        if(!str) {
            break;
        }
        p = str;
        do {
            p = strstr(p, "[");
            if(!p) {
                break;
            }
            tmp = (lklist*) malloc(sizeof(lklist));
            memset(tmp, 0, sizeof(lklist));
            tmp->index = p - str;
            tmp->next = lkidx;
            lkidx = tmp;            
            ++p;
        } while(1);

    } while(0);
    tmp = lkidx;
    while(tmp) {
        fprintf(stdout, "---index: %d\n", tmp->index);
        tmp = tmp->next;
    }
}
//https://edabit.com/challenge/su4fC3zugSBmS5bfq
//https://edabit.com/challenge/Fe9X2DxpSCMun6t5D
//https://edabit.com/challenge/FYKdBjg2kf7sAANiT
//https://edabit.com/challenge/gvcmR4McLyia2PDac
//https://edabit.com/challenge/CFYzzHAmM75pmsGae