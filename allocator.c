#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>

/*
TODO:
- implement a linked list of free blocks
- implement mmap version
- implement algoithms such as:
    - choosing sbrk or mmap optimally
    - first fit
    - best fit
    - worst fit
    - next fit
    - merging adjacent free blocks
    - splitting blocks
    - coalescing free blocks
    - buddy system (?)
*/

// metadata for each block
struct Block
{
    size_t size;
    struct Block *next;
};

// inital break point
static void *heap_start = NULL;

// linked list of free blocks
struct Block *free_head = NULL;

/* The `void heap_init()` function is responsible for initializing the heap. It sets the `heap_start`
variable to the current break point (end of the heap) using the `sbrk(0)` system call. This function
is called before any allocations are made to ensure that the heap is properly set up. */
void heap_init()
{
    heap_start = sbrk(0);
};

struct Block* request_space(size_t size)
{
    void* block;
    // increment break point by size and return a pointer to the start of the new block (or previous break point)
    block = sbrk(size);

    // if sbrk returns -1, there is no space left
    if (block == (void *)-1)
    {
        return NULL;
    }
    return block;
}

void* brk_alloc(size_t size)
{

    if (size == 0)
    {
        return NULL;
    }

    struct Block* new_block;

    // if the heap has not been initialized, initialize it
    if (heap_start == NULL)
    {
        heap_init();
    }

    // increment size to include the size of the block struct
    size += sizeof(struct Block);

    // request space from the heap
    new_block = request_space(size);

    // if there is no space left, return NULL
    if (new_block == NULL)
    {
        return NULL;
    }

    new_block->size = size;
    // not included in the linked list of free blocks
    new_block->next = NULL;

    // return a pointer to the start of the new block (+1 to skip metadata)
    return (void *)(new_block + 1);
}

void brk_dealloc(void *ptr)
{
    struct Block* block;
    // get the block struct from the pointer
    block = (struct Block *)ptr - 1;

    block->next = free_head;
    free_head = block;
    brk(block);
}

void* mmap_alloc(size_t size){

struct Block* block;

size_t pageSize= getpagesize();
size_t sizeNeeded = size + sizeof(struct Block);
size_t roundedSize = ((sizeNeeded - 1) / pageSize + 1) * pageSize;

block = (struct Block*) mmap(NULL, roundedSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

if(block == MAP_FAILED){
    return NULL;
}
return (void*)(block + 1);

}

void mmap_dealloc(void* ptr){
    if (ptr == NULL){
        return;
    }

    struct Block *block = (struct Block*)ptr - 1;
    munmap(block, block->size);;

}
int main()
{
    int* data = (int *)brk_alloc(sizeof(int));
    *data = 42;
    printf("%d \n", *data);
    brk_dealloc(data);
    char* data2 = (char *)brk_alloc(sizeof(char));
    *data2 = 'a';
    printf("%c \n", *data2);
    brk_dealloc(data2);

    int* data3 = (int *)mmap_alloc(sizeof(int));
    *data3 = 42;
    printf("%d \n", *data3);
    mmap_dealloc(data3);

    return 0;
}