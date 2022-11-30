#include <stdlib.h>

#ifdef __cpluspplus
extern "C" {
#endif

typedef struct {
	unsigned long long total;
	unsigned long long used;
	int sz_item;
	
	char data[0];
} GEN_LIST;

int add_item_gen_list(GEN_LIST **p, char *item, int n);
int add_item_traffic(GEN_LIST **p, char *item, int sz);

#ifdef __cpluspplus
}
#endif
