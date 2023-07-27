#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <iostream>

using namespace std;

unsigned long long factorial(int n);
void fdeg(int n, char *c, char *out);
unsigned long long combination(int n, int k);
void fprbinomial(int n, std::string &str);

std::string formula(int n) {
	std::string str = "";
    fprbinomial(n, str);
    return str;
}


unsigned long long factorial(int n) {
    if(n  == 0) return 1; 
    if(n  == 1) return 1; 
    return (n * factorial(n - 1));
}
unsigned long long combination(int n, int k) {
    if(n  == 0) return 1; 
    if(n  == 1) return 1; 
    return factorial(n)/(factorial(k) * factorial(n-k));
}
void fprbinomial(int degg, std::string& str) {
    //int n = 0;
    int deg = degg;
    if(degg < 0) {
        deg = -degg;
    }
    do {
        if(deg == 0) {
            str = "1";
            break;
        }
        else if(deg == 1) {
            str = ("a+b");
            break;
        }
        else {
            char tmp[128];
            int i = 0;
            char *fdega;
            char *fdegb;
            fdega = (char*)malloc(32);
            fdegb = (char*)malloc(32);
            memset(tmp, 0, 128);
            for(i = 0; i <= deg; ++i) {

                memset(tmp, 0, 128);
                if(i == 0) {
                    snprintf(tmp, 100, "a^%d+", deg - i);
                    str += tmp;
                }
                else if(i == 1 || (deg - i) == 1) {
                    memset(fdega, 0, sizeof(fdega));
                    memset(fdegb, 0, sizeof(fdegb));

                    fdeg(deg - i, "a", fdega);
                    fdeg(i, "b", fdegb);

                    snprintf(tmp, 100, "%d%s%s+", combination(deg, i), fdega, fdegb);
                    str += tmp;                    
                }                
                else if(i < deg) {
                    snprintf(tmp, 100, "%da^%db^%d+", combination(deg, i), deg - i, i);
                    str += tmp;
                }
                else if(i == deg) {
                    snprintf(tmp, 100, "b^%d", i);
                    str += tmp;
                }                
            }
            free(fdega);
            free(fdegb);
        }
    } while(0);
    if(degg < 0) {
        str.insert(0, "1/(");
        str += ")";
    }
}
void fdeg(int n, char *c, char *out){
    if(n == 0) {
        snprintf(out, 16, "%s", "");
    }
    else if(n == 1) {
        snprintf(out, 16, "%s", c);
    } else {
        snprintf(out, 16, "%s^%d", c, n);;
    }
}









int main(int argc, char *argv[]) {
    int n = 0;
    int k = 0;
    int deg = 0;
    std::string str;
    sscanf(argv[1], "%d", &n);
    sscanf(argv[2], "%d", &k);
    sscanf(argv[3], "%d", &deg);
    fprintf(stdout, "combination(%d, %d) = %d\n", n, k, combination(n,k));
    str = formula(deg);
    fprintf(stdout, "---%s---\n\n", str.c_str());
    return 0;
}
