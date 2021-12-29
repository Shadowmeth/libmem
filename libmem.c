#include <string.h>
#include <unistd.h>

struct block_data {
    struct block_data* next;
    char free;
    size_t size;
};

#define BLOCK_SIZE (sizeof(struct block_data))

struct block_data* block_list = NULL;

struct block_data* find_first_fit(struct block_data** last, size_t size)
{
    struct block_data* current = block_list;
    while (current && !(current->free && current->size >= size)) {
        *last = current;
        current = current->next;
    }

    return current;
}

struct block_data* allocate_space(struct block_data* last, size_t size)
{
    struct block_data* block;
    block = sbrk(0); // store the current address of program break
    void* allocate = sbrk(size + BLOCK_SIZE);
    if (allocate == (void *)-1) {
        return NULL; // sbrk failed
    }

    if (last) {
        last->next = block;
    }

    block->size = size;
    block->free = 0;
    block->next = NULL;
    return block;
}

void* mymalloc(size_t size)
{
    if (size <= 0) {
        return NULL; // defensive check
    }

    struct block_data* block;
    if (!block_list) {
        // first call of mymalloc
        block = allocate_space(NULL, size);
        if (!block) {
            // unable to request more space
            return NULL;
        }
        block_list = block;
    } else {
        struct block_data* last = block_list;
        block = find_first_fit(&last, size);
        if (!block) {
            // failed to find a free block, request more space
            block = allocate_space(last, size);
            if (!block) {
                // unable to request more space
                return NULL;
            } else {
                // we can split blocks here to reduce fragmentation
                block->free = 0;
            }

        }

    }

    return block + 1;
}

struct block_data* get_block(void* ptr)
{
    return (struct block_data *)ptr - 1;
}

void myfree(void* ptr)
{
    if (!ptr) {
        return;
    }

    struct block_data* block = get_block(ptr);
    block->free = 1;
}

void* mycalloc(size_t count, size_t elesize)
{
    size_t size = count * elesize;
    void* ptr = mymalloc(size);
    memset(ptr, 0, size);
    return ptr;
}

void* realloc(void* ptr, size_t size)
{
    if (!ptr) {
        return mymalloc(size);
    }

    struct block_data* block = get_block(ptr);
    if (block->size >= size) {
        return ptr;
    }

    // we need to allocate memory now
    void* new_ptr = mymalloc(size);
    if (!new_ptr) {
        return NULL;
    }

    memcpy(new_ptr, ptr, block->size);
    myfree(ptr);
    return new_ptr;
}

int main()
{
    int* ptr = (int *)mymalloc(sizeof(int) * 5);
    for (int i = 0; i < 5; i++) {
        ptr[i] = i;
    }

    for (int i = 0; i < 5; i++) {
        printf("%d\n", ptr[i]);
    }

    myfree(ptr);
}

