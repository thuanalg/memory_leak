
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define BUFF_SIZE 32  //A segment
//#define MAX_BLOCK 20    //Maximum of number of pointers are allocated.
//#define CAST_PCHAR(p)   ((char*)(p))
//#define myfprintf(fm, ...) fprintf(stdout, "\nline: %d, "fm"\n", __LINE__, ##__VA_ARGS__)

typedef struct __tracker__ {
    int r;
    int c;
    int count;
} tracker;

typedef struct __island_list__ {
    int total;
    int used;
    char data[0];
} island_list;

typedef struct __matrix__ {
    //int sz_item;
    int col;
    int row;
    char data[0];
} matrix;

typedef struct __MYNODE__ {
    short v;
    char act;
} MYNODE;
#define MYTYPE MYNODE

#define MATRIX(mtx, t, r, c) { (mtx) = malloc( sizeof(t) * r * c + sizeof(matrix)); (mtx)->row = r; (mtx)->col = c;}
//#define setValMtx(mtx, t, i, j, val) { t *ppp = (t *) mtx->data; *((ppp) + ( i * (mtx->col) + j)) = val;}
#define setValMtx(mtx, t, i, j, val) { (((t *) mtx->data) + ( i * (mtx->col) + j))->v = val;}
#define setActMtx(mtx, t, i, j, val) { (((t *) mtx->data) + ( i * (mtx->col) + j))->act = val;}
//#define getValMtx(mtx, t, i, j, val) { t *ppp = (t *) mtx->data;  val = *((ppp) + ( i * (mtx->col) + j));}
#define getValMtx(mtx, t, i, j, val) { val = (((t *) mtx->data) + ( i * (mtx->col) + j))->v;}
#define getActMtx(mtx, t, i, j, val) { val = (((t *) mtx->data) + ( i * (mtx->col) + j))->act;}



// Call recursive
void doact(matrix *mtx, tracker *tr);
void do_recursive(matrix *mtx, int i, int j, tracker *tr);

//  Run command ./LargestIsland 8 7
int main(int argc, char *argv[]) {
    matrix *mtx = 0;
    short val = 0;
    char act = 0;
    int m = 1, n = 1;
    tracker t0 = {0};

    if(argc < 3) {
        return EXIT_FAILURE;
    }

    sscanf(argv[1], "%d", &m);
    sscanf(argv[2], "%d", &n);
    
    if(m < 1) {
        return EXIT_FAILURE;
    }  
    if(n < 1) {
        return EXIT_FAILURE;
    } 

    MATRIX(mtx, MYTYPE, m, n);


    for(int i = 0; i <  mtx->row; ++i)
        for(int j = 0; j <  mtx->col; ++j) {
            setValMtx(mtx, MYTYPE, i, j, ( ((random()%10) > 2) ? 0 : 1));
            setActMtx(mtx, MYTYPE, i, j, 0);
        }
    for(int i = 0; i <  mtx->row; ++i) {
        fprintf(stdout, "\n\n");
        for(int j = 0; j <  mtx->col; ++j) {
            getValMtx(mtx, MYTYPE, i, j, val);
            getActMtx(mtx, MYTYPE, i, j, act);
            fprintf(stdout, "%ld\t", val, act);
        }

    }
    fprintf(stdout, "\n\n(row, col) = (%d, %d)\n\n", mtx->row, mtx->col);
    //doact(mtx, mtx->row, mtx->col);
    doact(mtx, &t0);
    for(int i = 0; i <  mtx->row; ++i) {
        fprintf(stdout, "\n\n");
        for(int j = 0; j <  mtx->col; ++j) {
            getValMtx(mtx, MYTYPE, i, j, val);
            getActMtx(mtx, MYTYPE, i, j, act);
            fprintf(stdout, "%ld\t", val, act);
        }

    }   
    free(mtx);
    fprintf(stdout, "\n\n\n-------------- Largestcount: (%d, %d) = %d\n\n\n", 
        t0.c, t0.r, t0.count);
    return 0;
}

void doact(matrix *mtx, tracker *t0)
{
    int i = 0;
    int j = 0;
    short val;
    char act;
    int row, col;
    //tracker t0 = {0};
    tracker t = {0};

    row = mtx->row;
    col = mtx->col;

    for(i = 0; i < row; ++i) {
        for(j = 0; j < col; ++j) {
            getValMtx(mtx, MYTYPE, i, j, val);
            if(!val) {
                continue;
            }
            getActMtx(mtx, MYTYPE, i, j, act);
            if(act) {
                continue;
            }
            t.c = i;
            t.r = j;
            t.count = 1;
            setActMtx(mtx, MYTYPE, i, j, 1);
            do_recursive(mtx, i, j, &t);
            fprintf(stdout, "\n\ncount: %d\n\n", t.count);
            if(t.count > t0->count) {
                (*t0) = t;
            }
        }
    }
    fprintf(stdout, "\n\n\n-------------- Largestcount: (%d, %d) = %d\n\n\n", 
        t0->c, t0->r, t0->count);
}

void do_recursive(matrix *mtx, int i, int j, tracker *tr) {
    short val;
    char act;    
    int m,n;
    int row = mtx->row;
    int col = mtx->col;
    //left
    do {
        if(j < 1) break;
         //(i, j - 1)
        m = i;
        n = j - 1;
        
        getValMtx(mtx, MYTYPE, m, n, val);
        if(!val) {
            break;
        } 
        
        getActMtx(mtx, MYTYPE, m, n, act);
        if(act) {
            break;
        }
        
        (tr->count)++;
        //fprintf(stdout, "\n left: %d, count: %d\n\n", __LINE__ , tr->count);
        setActMtx(mtx, MYTYPE, m, n, 1);
        do_recursive(mtx, m, n, tr);

    } while(0);

    //right
    do {
        if(j >= col - 1) break;
        //(i, j + 1)
        
        m = i;
        n = j + 1;
        getValMtx(mtx, MYTYPE, m, n, val);
        if(!val) {
            break;
        } 
        
        getActMtx(mtx, MYTYPE, m, n, act);
        if(act) {
            break;
        }
        //fprintf(stdout, "\n right: %d\n\n", __LINE__ );
        (tr->count)++;
        //fprintf(stdout, "\n right: %d, count: %d\n\n", __LINE__ , tr->count);
        setActMtx(mtx, MYTYPE, m, n, 1);
        do_recursive(mtx, m, n,  tr);
    } while(0);

    //top
    do {
        if(i < 1) break;
        //(i - 1, j)
        m = i - 1;
        n = j;
        
        getValMtx(mtx, MYTYPE, m, n, val);
        if(!val) {
            break;
        } 
        
        getActMtx(mtx, MYTYPE, m, n, act);
        if(act) {
            break;
        }
        //fprintf(stdout, "\n top: %d\n\n", __LINE__ );
        (tr->count)++;
        //fprintf(stdout, "\n top: %d, count: %d\n\n", __LINE__ , tr->count);
        setActMtx(mtx, MYTYPE, m, n, 1);
        do_recursive(mtx, m, n,  tr);
    } while(0);

    //bottom
    do {
        if(i >= row - 1) break;
        //(i + 1, j)
        
        m = i + 1;
        n = j;    
        getValMtx(mtx, MYTYPE, m, n, val);
        if(!val) {
            break;
        } 
        
        getActMtx(mtx, MYTYPE, m, n, act);
        if(act) {
            break;
        }
        //fprintf(stdout, "\n bottom: %d\n\n", __LINE__ );
        (tr->count)++;
        //fprintf(stdout, "\n bottom: %d, count: %d\n\n", __LINE__ , tr->count);
        setActMtx(mtx, MYTYPE, m, n, 1);
        do_recursive(mtx, m, n,  tr);            
    } while(0);

    //top-left
    do {
        if( i < 1) break;
        if( j < 1) break;
        m = i - 1;
        n = j - 1;    
        getValMtx(mtx, MYTYPE, m, n, val);
        if(!val) {
            break;
        } 
        getActMtx(mtx, MYTYPE, m, n, act);
        if(act) {
            break;
        }
        (tr->count)++;
        setActMtx(mtx, MYTYPE, m, n, 1);
        do_recursive(mtx, m, n,  tr);            
    } while(0);

    //top-right
    do {
        if( i < 1) break;
        if( j >= col - 1) break;
        m = i - 1;
        n = j + 1;       
        getValMtx(mtx, MYTYPE, m, n, val);
        if(!val) {
            break;
        } 
        getActMtx(mtx, MYTYPE, m, n, act);
        if(act) {
            break;
        }
        (tr->count)++;
        setActMtx(mtx, MYTYPE, m, n, 1);
        do_recursive(mtx, m, n,  tr);         
    } while(0);

    //bottom-left
    do {
        if( i >= row - 1) break;
        if( j < 1) break;
        m = i + 1;
        n = j - 1;   
        getValMtx(mtx, MYTYPE, m, n, val);
        if(!val) {
            break;
        } 
        getActMtx(mtx, MYTYPE, m, n, act);
        if(act) {
            break;
        }
        (tr->count)++;
        setActMtx(mtx, MYTYPE, m, n, 1);
        do_recursive(mtx, m, n,  tr);             
    } while(0);

    //bottom-right
    do {
        if( i >= row - 1) break;
        if( j >= col - 1) break;
        m = i + 1;
        n = j + 1;      
        getValMtx(mtx, MYTYPE, m, n, val);
        if(!val) {
            break;
        } 
        getActMtx(mtx, MYTYPE, m, n, act);
        if(act) {
            break;
        }
        (tr->count)++;
        setActMtx(mtx, MYTYPE, m, n, 1);
        do_recursive(mtx, m, n,  tr);
    } while(0);    
}