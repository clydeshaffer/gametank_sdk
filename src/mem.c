#define HEAP_SIZE 512
static char heap[HEAP_SIZE];


typedef struct alloc_header_
{
    struct alloc_header_* prev;
    struct alloc_header_* next;
    int size; //not including header
    char free; //1 if free 0 if used
} alloc_header;

static alloc_header* cursor;

void mem_init() {
    cursor = (alloc_header*) heap;
    cursor->prev = 0;
    cursor->next = (alloc_header*) (heap+HEAP_SIZE-sizeof(alloc_header));
    cursor->size = HEAP_SIZE - sizeof(alloc_header)*2;
    cursor->free = 1;

    cursor->next->prev = cursor;
    cursor->next->next = 0;
    cursor->next->size = 0;
    cursor->next->free = 0;
}

void* mem_alloc(int size) {
    cursor = (alloc_header*) heap;
    
    do {
        if((cursor->free) && (cursor->size >= size)) {
            cursor->free = 0;
            if(cursor->size - size > sizeof(alloc_header)) {
                cursor->next->prev = (alloc_header*) (((char*)cursor) + sizeof(alloc_header) + size);
                cursor->next->prev->next = cursor->next;
                cursor->next = cursor->next->prev;
                cursor->next->size = cursor->size - (size + sizeof(alloc_header));
                cursor->next->prev = cursor;
                cursor->next->free = 1;
                cursor->size = size;
            }
            return (void*)(cursor+1);
        }
        cursor = cursor->next;
    } while (cursor->next != 0);

    return 0;
}

void mem_free(void* ptr) {
    cursor = ((alloc_header*)ptr)-1;
    cursor->free = 1;
    if(cursor->next) {
        if(cursor->next->free) {
            if(cursor->next->next) cursor->next->next->prev = cursor;
            cursor->size += cursor->next->size+sizeof(alloc_header);
            cursor->next = cursor->next->next;
        }
    }
    if(cursor->prev) {
        if(cursor->prev->free) {
            cursor = cursor->prev;
            if(cursor->next->next) cursor->next->next->prev = cursor;
            cursor->size += cursor->next->size+sizeof(alloc_header);
            cursor->next = cursor->next->next;
        }
    }
}