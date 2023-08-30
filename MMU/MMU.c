
#include <stdio.h>

#define BUFF_SIZE 32  //A segment
#define MAX_BLOCK 20    //Maximum of number of pointers are allocated.
#define CAST_PCHAR(p)   ((char*)(p))
#define myfprintf(fm, ...) fprintf(stdout, "\nline: %d, "fm"\n", __LINE__, ##__VA_ARGS__)

typedef struct __mmu_list__ {
    int total;
    int used;
    char data[0];
} mmu_list;



typedef struct __item_block__ {
    int n;
    void *p;
} item_block;

#define STACK_REMAIN(k)    (((k)->total - (k)->used - sizeof(mmu_list)) >= sizeof(item_block))
#define MMU_SIZE            (MAX_BLOCK * sizeof(item_block) + sizeof(mmu_list))
#define STACK_ITEMS(k)      (((k)->used)/sizeof(item_block))

char mysegment[BUFF_SIZE];//Auto set zero, bss data
char mymmu[MMU_SIZE];//Auto set zero, bss data
mmu_list *stack = (mmu_list *)mymmu;
void dump_list();
void try_clear();

void *mymalloc(int n);
void myfree(void *p);
void myinsert(mmu_list *s, char *data, int);
// Here, size of program may be stable of size because I don't use any heap. 
//However, I am not sure from other.
int main(int argc, char *argv[]) {
    //Non multi-thread here, not thread-safe
    void *p = 0, *p1 = 0, *p3;
    stack->total = MMU_SIZE;
    stack->used = 0;

    p = mymalloc(BUFF_SIZE + 1);
    if(!p) {
        myfprintf("p: %p\n", p);
    }
    p = mymalloc(BUFF_SIZE - 1);
    myfprintf("p: %p\n", p);

    dump_list();

    p1 = mymalloc(1);
    myfprintf("p1: %p\n", p1);
    dump_list();
    myfree(p);

    p1 = mymalloc(1);
    myfprintf("p1: %p\n", p1);
    dump_list();

    p1 = mymalloc(16);
    myfprintf("p1: %p\n", p1);
//
    p3 = mymalloc(3);
    myfprintf("p3: %p\n", p3);
//
    dump_list();
//
    myfree(p3);
//
    p1 = mymalloc(4);
    myfprintf("p1: %p\n", p1);
    dump_list();
//
    p1 = mymalloc(2);
    myfprintf("p1: %p\n", p1);
    dump_list();
//
    p1 = mymalloc(1);
    myfprintf("p1: %p\n", p1);
    dump_list();
//
    myfree(p1);
    dump_list();

    p1 = mymalloc(10);
    dump_list();

    p1 = mymalloc(1);
    p1 = mymalloc(1);
    dump_list();

    try_clear();
    return 0;
}
void *mymalloc(int n) {
    //Non multi-thread here, not thread-safe
    void *p = 0;
    int k = 0;
    int m = STACK_ITEMS(stack);
    item_block *t = (item_block *)stack->data;   
   
    do {
        if(n < 1) break;
        
        if(n > BUFF_SIZE) break;

        if(!STACK_REMAIN(stack)) {
            break;
        }   

        if(m < 1) {
            p = mysegment;
            myfprintf("--- option 0 new") ;  
            break;
        } 
        
        do {
            //myfprintf("range: %d", CAST_PCHAR(t[0].p) -  CAST_PCHAR(mysegment));
            if (CAST_PCHAR(t[0].p) >= CAST_PCHAR(mysegment) + n) {
                p = mysegment;
                //k = 0;  
                myfprintf("--- option 1 insert at the first") ;  
                break;          
            }

            while (k < (m -1)) {
                if(CAST_PCHAR(t[k + 1].p)  >= (CAST_PCHAR(t[k].p) + t[k].n) + n) {
                    p = CAST_PCHAR(t[k].p) + t[k].n;
                    myfprintf("--- option 2 insert at middle") ;  
                    ++k;
                    break;
                }
                ++k;
            }
        
            if ((CAST_PCHAR(mysegment) + BUFF_SIZE) >= (CAST_PCHAR(t[m-1].p) + t[m-1].n) + n) 
            {
                p = CAST_PCHAR(t[m-1].p) + t[m-1].n;
                k = m;         
                myfprintf("--- option 3 Append, p: %p, len: %d", CAST_PCHAR(t[m-1].p), t[m-1].n) ;  
                break;          
            }
        }
        while(0);    
    } while(0);
    if(p) {
        //insert  INTO MMU STACK AT k position
        item_block mem;
        mem.n = n;
        mem.p = p;
        myinsert(stack, CAST_PCHAR(&mem), k);
    } 
    return p;
}
void myfree(void *p) {
    //Non multi-thread here, not thread-safe
    int n = STACK_ITEMS(stack);
    item_block *t = (item_block *)stack->data;
    int i = 0;
    //Should use binary search, but I am lazy.
    for(i = 0; i < n; ++i) {
        if(p == t[i].p) {   
            if(i < n -1) {
                //memcpy(t + i, t + i + 1, sizeof(item_block) * (n - i - 1));
                int j = 0;
                for(j = i; j < (n - 1); ++j) {
                    t[j] = t[j + 1];
                }
            }
            t[n-1].n = 0;
            stack->used -= (int)sizeof(item_block);
            break;
        }
    }
}

void myinsert(mmu_list *s, char *data, int i) {
    //Non multi-thread here, not thread-safe
    int n = STACK_ITEMS(stack);
    myfprintf("index: %d", i);
    item_block *t = (item_block *)stack->data;
    do {
        if(n < 1) {
            //memcpy(t, data, sizeof(item_block));  
            t[0] = *((item_block *)data);
            break;
        }
        myfprintf("src: %p, dest: %p, sz: %d", 
            CAST_PCHAR(t + i), CAST_PCHAR(t + i + 1), (n - i) * sizeof(item_block));
        //memmove(CAST_PCHAR(t + i + 1), CAST_PCHAR(t + i), (n - i) * sizeof(item_block));
        for(int j = n; j > i; j-- ) {
            t[j] = t[j -1];
        }
        //memcpy(CAST_PCHAR(t + i), data, sizeof(item_block)); 
        t[i] = *((item_block *)data);
    } while(0);
    stack->used += sizeof(item_block);
}
void dump_list() {
    int n = STACK_ITEMS(stack);
    int i = 0;
    item_block *t = (item_block *)stack->data; 
    myfprintf("\n\n\n");
    for(i = 0; i < n; ++i) {
        myfprintf("---->>>> i: %d, sz:%d, p: %p", i, t[i].n, t[i].p);
    }   
}
void try_clear() {
    item_block *t = (item_block *)stack->data; 
    int n = STACK_ITEMS(stack);
    while(n) {
        myfree(t[n/2].p);
        n = STACK_ITEMS(stack);
        dump_list();
    }
}