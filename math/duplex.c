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

void singleplain(char **out, int index, int *outlen);
void MessagefromSpace(char **out, int *len);
void gen_lkidx(char *str, int len);
int duplex(char *str);
/////////////////////////
int main(int argc, char *argv[]) {
    ull n = 0;
    char *out = 0;
    int outlen = 0;
    int len = 0;
    sscanf(argv[1], "[%llu]", &n);
    fprintf(stdout, "n: %llu\n", n);
    gen_lkidx(argv[1], len = strlen(argv[1]));


    out = malloc(len + 1);
    memset(out, 0, len + 1);
    memcpy(out, argv[1], len);
    MessagefromSpace(&out, &len);
    //singleplain(argv[1], strlen(argv[1]), &out, &outlen);
    fprintf(stdout, "out: \"%s\"\noutlen: %d\n", out, len);
    return 0;
}
int duplex(char *str)
{
    return 0;
}
void MessagefromSpace(char **out, int *len) {
    int j = 0;
    char *p = 0;
    //int outlen = 0;
    do {
        if(!len) {
            break;
        }
        if(*len < 1) {
            break;
        }
        while(lkidx) {
            //outlen = strlen(*out);
            lklist *tmp = lkidx;
            singleplain(out, tmp->index, len);
            lkidx = lkidx->next;                        
            free(tmp);
        }
    } while(0);
}
void singleplain(char **out, int index, int *outlen)
{
    ull n = 0;
    int i = 0;
    int j = 0;
    int err = 0;
    ull sz = 0;
    int len = 0;
    char *p = 0;
    int delta = 0;
    char *buf = 0;
    //fprintf(stdout, "string len: %d\n", len);
    do {
        if(!out)  { 
            break;
        }
        if(!(*out))  { 
            break;
        }        
        if(!outlen)  { 
            break;
        }
        p = strstr(*out + index, "]");
        if(!p) {
            break;
        }
        sscanf(*out + index, "[%llu]", &n);
        if(n < 2) {
            fprintf(stderr, "Must >= 2.\n");
            break;
        }
        len = p - (*out + index) + 1;        
        i = 0;
        while(i < len) {
            if((*out + index)[i] >= 'A' && (*out + index)[i] <= 'Z') {
                break;
            }
            if((*out + index)[i] >= 'a' && (*out + index)[i] <= 'z') {
                break;
            }
            ++i;
        }
        fprintf(stdout, "----i: %d\n", i);
        if(i >= len) {
            err = 1;
            break;
        }
        j = i + 1;
        while(j < len) {
            if((*out + index)[j] < 'A') {
                j--;
                break;
            }
            if((*out + index)[j] > 'z') {
                j--;
                break;
            }
            if((*out + index)[j] > 'Z' && (*out + index)[j] < 'a') {
                j--;
                break;
            }
            ++j;
        }
        
        fprintf(stdout, "----j: %d\n", j);
        if(j >= len) {
            err = 1;
            break;
        }

        if(j - i + 1 < 1) {
            break;
        }

        buf = malloc(j - i + 1 + 1);
        memset(buf, 0, j - i + 1 + 1);
        memcpy(buf, *out + i, j - i + 1);
        fprintf(stdout, "buf: %s\n", buf);

        //sz = *outlen + (n - 1) * (j - i + 1);
        delta = n * (j - i + 1) - (j - index + 1);
        if(delta > 0) {
            sz = *outlen + delta;
            //aaa|bb
            *out = realloc( *out, sz + 1);
            memset(*out + *outlen, 0, delta + 1);
        }
        if(*(*out + j + 2) && delta > 0) {
            memmove(*out + j + 1, *out + j + 1 + delta, (*outlen - j -1));
        }
        //[2kk]aaa, j = 3, len = 7, i = 2, 
        for(int k = 0; k < n; ++k) {
            memcpy(*out + index + k * (j -i + 1), buf, (j - i + 1));
        }
        *outlen += delta;
        (*out)[*outlen] = 0;
    } while(0);

    if(buf)
    {
        free(buf);
    }
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