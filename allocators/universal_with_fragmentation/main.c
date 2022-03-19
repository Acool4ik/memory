#include <stdlib.h>
#include <stdio.h>


typedef struct {
    unsigned blocks_size : 31;
    unsigned isFree : 1;
} META_BLOCK;

// ptr on root and size of buffer
META_BLOCK *BUFFER_BLOCKS = NULL;
unsigned BUFFER_BLOCKS_SIZE;

unsigned count_fragments;
unsigned count_free_fragments;
unsigned count_free_blocks;


// handlers functions
unsigned getFreeSizeInBlocks(META_BLOCK *mb) {
    return mb->blocks_size - 2;
}
void setNewFlag(META_BLOCK *mb, unsigned isFree) {
    mb->isFree = isFree;
    (mb + mb->blocks_size - 1)->isFree = isFree;
}
void setMetaBlock(META_BLOCK *mb, unsigned blocks_size, unsigned isFree) {
    mb->blocks_size = blocks_size;
    mb->isFree = isFree;
    (mb + blocks_size - 1)->blocks_size = blocks_size;
    (mb + blocks_size - 1)->isFree = isFree;
}
void defragmentationToRight(META_BLOCK *mb) {
    // base check
    if(mb >= BUFFER_BLOCKS + BUFFER_BLOCKS_SIZE) return;
    if(mb + mb->blocks_size >= BUFFER_BLOCKS + BUFFER_BLOCKS_SIZE) return;

    // reqursive case
    if((mb + mb->blocks_size)->isFree) {
        // statistics
        count_fragments--;
        count_free_fragments--;
        count_free_blocks+=2;

        // concatination fragment's
        setMetaBlock(
            mb,
            mb->blocks_size + (mb + mb->blocks_size)->blocks_size,
            1
        );

        return defragmentationToRight(mb);
    }
}
void defragmentationToLeft(META_BLOCK *mb) {
    // base check
    if(mb <= BUFFER_BLOCKS) return;
    if(mb - (mb - 1)->blocks_size < BUFFER_BLOCKS) return;

    // reqursive case
    if((mb - 1)->isFree) {
        // statistics
        count_fragments--;
        count_free_fragments--;
        count_free_blocks+=2;

        // concatination fragment's
        setMetaBlock(
            mb - (mb - 1)->blocks_size,
            mb->blocks_size + (mb - 1)->blocks_size,
            1
        );

        return defragmentationToRight(mb);
    }
}


// Функция инициализации
void mysetup(void *buf, size_t size)
{
    // define global variables
    BUFFER_BLOCKS = buf;
    BUFFER_BLOCKS_SIZE = size / 4;
    count_fragments++;
    count_free_fragments++;
    count_free_blocks = BUFFER_BLOCKS_SIZE - 2;

    // filling buffer by zero
    for(size_t i = 0; i < size; i++)
        *(unsigned char *)(buf + i) = 0;

    // create first block
    setMetaBlock(BUFFER_BLOCKS, size / 4, 1);
}


// Функция аллокации
void *myalloc(size_t size)
{
    // base check
    if(size == 0) return NULL;

    void *user_ptr = NULL;
    META_BLOCK *tmp_mb = BUFFER_BLOCKS;
    unsigned blocks_size = size / 4;
    if(size % 4) blocks_size++;

    while(tmp_mb < BUFFER_BLOCKS + BUFFER_BLOCKS_SIZE) {
        // all size_blocks in current fragments
        unsigned before = getFreeSizeInBlocks(tmp_mb) + 2;

        if( // if requested size_blocks == size blocks of fragment
            blocks_size == getFreeSizeInBlocks(tmp_mb)
            && tmp_mb->isFree
        ) {
            // alloc existed fragment
            setNewFlag(tmp_mb, 0);
            user_ptr = tmp_mb + 1;

            // statistics
            count_free_fragments--;
            count_free_blocks-=blocks_size;
            break;

        }   // if requested size_blocks - size blocks of fragment < 3, impossible create second fragment
        else if(
            blocks_size < getFreeSizeInBlocks(tmp_mb)
            && before - (blocks_size + 2) < 3
            && tmp_mb->isFree
        ) {
            // alloc existed fragment
            setMetaBlock(tmp_mb, before, 0);
            user_ptr = tmp_mb + 1;

            // statistics
            count_free_fragments--;
            count_free_blocks-=before - 2;
            break;

        }   // if requested size_blocks - size blocks of fragment > 3, possible create second fragment
        else if(
            blocks_size < getFreeSizeInBlocks(tmp_mb)
            && before - (blocks_size + 2) > 3
            && tmp_mb->isFree
        ) {
            // alloc existed + create new fragment
            setMetaBlock(tmp_mb, blocks_size + 2, 0);
            setMetaBlock(tmp_mb + tmp_mb->blocks_size, before - tmp_mb->blocks_size, 1);
            user_ptr = tmp_mb + 1;

            // statistics
            count_fragments++;
            count_free_blocks-=blocks_size + 2;
            break;
        }
        else {
            tmp_mb = tmp_mb + tmp_mb->blocks_size;
        }
    }

    return user_ptr;
}


// Функция освобождения
void myfree(void *p)
{
    // base check
    if(p == NULL) return;

    // cast type pointer's
    META_BLOCK *tmp_mb = (META_BLOCK *)p - 1;

    // statistics
    count_free_fragments++;
    count_free_blocks+=tmp_mb->blocks_size - 2;

    // free fragment
    setNewFlag(tmp_mb, 1);

    // reqursion defragmentation
    defragmentationToRight(tmp_mb);
    defragmentationToLeft(tmp_mb);
}


void printMemoryLayout(META_BLOCK *mb) {
    static unsigned index = 0;

    // not resursive case
    if(mb >= BUFFER_BLOCKS + BUFFER_BLOCKS_SIZE) {
        index = 0;
        return;
    }

    // output in console about state memory's
    if(index == 0) {
        printf("STATISTICS:\n");
        printf("Exist fragments: %u\n", count_fragments);
        printf("Free/busy fragments: %u/%u\n", count_free_fragments, count_fragments - count_free_fragments);
        printf("Exist blocks: %u\n", BUFFER_BLOCKS_SIZE);
        printf("Free/busy blocks: %u/%u\n", count_free_blocks, BUFFER_BLOCKS_SIZE - count_free_blocks);
        printf("Free/busy bytes: %u/%u\n", count_free_blocks * 4, (BUFFER_BLOCKS_SIZE - count_free_blocks) * 4);
        printf("Used memory: %.1f%%\n",
            (BUFFER_BLOCKS_SIZE - count_free_blocks) / (float)BUFFER_BLOCKS_SIZE * 100.0
        );
        printf("\n");
    }

    // output in console data about fragment's
    printf("Frugment number: %u\n", index);
    printf("HEADER\t[%p; %p)\t{blocks_size=%u isFree=%u} %lu byte\n",
        mb, mb + 1,
        mb->blocks_size, mb->isFree,
        sizeof(META_BLOCK)
    );
    printf("DATA\t[%p; %p)\t{...payload...} %u byte, %u KByte\n",
        mb + 1, mb + mb->blocks_size - 1,
        getFreeSizeInBlocks(mb) * 4,
        (getFreeSizeInBlocks(mb) * 4) / 1024
    );
    printf("TAIL\t[%p; %p)\t{blocks_size=%u isFree=%u} %lu byte\n",
        mb + mb->blocks_size - 1, mb + mb->blocks_size,
        (mb + mb->blocks_size - 1)->blocks_size, (mb + mb->blocks_size - 1)->isFree,
        sizeof(META_BLOCK)
    );

    // call reqursive
    index++;
    return printMemoryLayout(mb + mb->blocks_size);
}


// unit test
int main(void) {
    #define MByte 1024 * 1024

    // initialization castom allocator
    void *bufferKByte = malloc(MByte);
    mysetup(bufferKByte, MByte);

    // container of pointers
    void *arrayPtrs[2048];
    unsigned top = 0;

    // alloc random size
    for(int i = 0; i < 42; i++)
        arrayPtrs[top++] = myalloc((rand() % (MByte / 2)));
    // print current state of memory
    printMemoryLayout(BUFFER_BLOCKS);

    // free all fragment
    for(int i = 0; i < top; i++)
        myfree(arrayPtrs[i]);
    // print state after free
    printf("\n");
    printMemoryLayout(BUFFER_BLOCKS);

    return 0;
}
