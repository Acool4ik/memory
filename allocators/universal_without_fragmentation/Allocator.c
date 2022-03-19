#include <cstdio>
#include <cstdint>
#include <cstdbool>
#include <cstdlib>


#define SIZE_HEADER (sizeof(size_t))
#define HEAP_SIZE (1 << 30)
#define FRAGMENT_SET_SIZE (1 << 15)


// struct discribe fragment-item of dinamic memory
typedef struct SFragment
{   
    uint8_t *top_ptr;
    uint8_t *base_ptr;
    size_t free_size;
} Fragment;

 
// init dinamic memory (heap)
static uint8_t HEAP[HEAP_SIZE];
// init all fragments of memory in heap
static Fragment FragmentSet[FRAGMENT_SET_SIZE];
static size_t USED = 0;


static Fragment *find_fragment(size_t size) {
    // first init heap
    if (USED == 0) {
        FragmentSet[0].base_ptr = HEAP;
        FragmentSet[0].top_ptr = HEAP;
        FragmentSet[0].free_size = HEAP_SIZE;
        USED++;
    }

    int i;
    Fragment* best_fragment = NULL;

    // find the best variant 
    for (i = 0; i < USED; i++) {
        if (best_fragment == NULL)
            if(FragmentSet[i].free_size >= size)
                best_fragment = FragmentSet + i;
        else 
            if (FragmentSet[i].free_size >= size && best_fragment->free_size >= FragmentSet[i].free_size)
                best_fragment = FragmentSet + i;
    }

    if (best_fragment == NULL) {
        printf("[%s:%d] Don't find fragment-item for allocation!\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
   
    return best_fragment;
}

void* cmalloc(size_t size) {
    if (size <= 0) return NULL;

    if (size + SIZE_HEADER > HEAP_SIZE) {
        printf("[%s:%d] Size allocation more then max heap size!\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    // get fragment with need memory
    Fragment *f = find_fragment(size + SIZE_HEADER);

    uint8_t *usr_ptr = f->top_ptr + SIZE_HEADER;

    // put size of block in header
    (*((size_t*)(f->top_ptr))) = size + SIZE_HEADER;

    // shift pointer and decreace free-size
    f->top_ptr = f->top_ptr + size + SIZE_HEADER;
    f->free_size = f->free_size - size - SIZE_HEADER;

    return usr_ptr;
}

void cfree(void* ptr) {
    if (ptr == NULL) return;

    if (FRAGMENT_SET_SIZE > USED) {
        // create new FragmentSet item
        FragmentSet[USED].base_ptr = (uint8_t*)ptr - SIZE_HEADER;
        FragmentSet[USED].top_ptr = (uint8_t*)ptr - SIZE_HEADER;
        FragmentSet[USED].free_size = *((size_t*)((uint8_t*)((uint8_t*)ptr - SIZE_HEADER)));
        USED++;
    }
    else {
        printf("[%s:%d] Owerflow FragmentSet size!\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
}

void heapLogger(void) {
    printf("\theapLogger():\n");
    unsigned i;
    for (i = 0; i < USED; i++) {
        printf("Fragment [%u] base/top ptrs [%p]/[%p] free-size [%llu] byte\n", i + 1,
            FragmentSet[i].base_ptr,
            FragmentSet[i].top_ptr,
            FragmentSet[i].free_size
        );
    }
}


int main(void)
{
    int* x;
    int* array;
    int size_array = (1 << 19);
    double *y;

 
    x = (int *)cmalloc(sizeof(int));
    heapLogger();
    array = (int*)cmalloc(size_array * sizeof(int));
    heapLogger();
    y = (double*)cmalloc(sizeof(double));
    heapLogger();

    
    cfree(x);
    heapLogger();
    cfree(array);
    heapLogger();
    cfree(y);
    heapLogger();

    return 0;
}

