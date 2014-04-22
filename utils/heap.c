#include <stdint.h>
#include "utils/heap.h"


typedef struct _heap_header_t {
    struct _heap_header_t* p_prev;
    struct _heap_header_t* p_next;
    unsigned int chunk_size;
    unsigned int flags;
} heap_header_t;

/* THE heap */
#define HEAP_SIZE (128*1024)
static unsigned char g_heap[HEAP_SIZE];


#define HEAP_ALIGNMENT 4
#define HEADER_FLAGS_IN_USE 0x00000001


void heap_init()
{
    heap_header_t* p_first = (heap_header_t*)(void*)g_heap;

    p_first->p_prev = (heap_header_t*)NULL;
    p_first->p_next = (heap_header_t*)NULL;
    p_first->chunk_size = HEAP_SIZE - sizeof(heap_header_t);
    p_first->flags = 0;
}

void* heap_alloc(unsigned int size)
{
    heap_header_t* p_new_hdr;
    heap_header_t* p_hdr = (heap_header_t*)(void*)g_heap;

    /* align the size */
    size = (size + (HEAP_ALIGNMENT-1)) & (~(HEAP_ALIGNMENT-1));

    /* integer overflow? */
    if (size < HEAP_ALIGNMENT) return NULL;

    /* see if we can find an available slot */
    for (; p_hdr != NULL; p_hdr = p_hdr->p_next) {
        if (((p_hdr->flags & HEADER_FLAGS_IN_USE) == 0) &&
            (p_hdr->chunk_size >= size))
        {
            break;
        }
    }

    /* did we find one? */
    if (p_hdr == NULL) return NULL;

    /* yes! */
    p_hdr->flags = HEADER_FLAGS_IN_USE;

    /* is there enough room for a new chunk? */
    if (p_hdr->chunk_size > size + sizeof(heap_header_t)) {
        p_new_hdr = (heap_header_t*)(void*)
            ((unsigned char*)p_hdr + sizeof(heap_header_t) + size);

        /* the new header defines an empty chunk - set its size */
        p_new_hdr->chunk_size =
            p_hdr->chunk_size - size - sizeof(heap_header_t);
        p_new_hdr->flags = 0;

        /* and adjust the current chunk size */
        p_hdr->chunk_size = size;

        /* fix the pointers with the new header */
        p_new_hdr->p_prev = p_hdr;
        p_new_hdr->p_next = p_hdr->p_next;
        if (p_hdr->p_next) p_hdr->p_next->p_prev = p_new_hdr;
        p_hdr->p_next = p_new_hdr;
    }

    return (void*)((unsigned char*)p_hdr + sizeof(heap_header_t));
}

void heap_free(void* p_obj)
{
    heap_header_t* p_hdr;

    if (p_obj == NULL) return;

    p_hdr =
        (heap_header_t*)((unsigned char*)p_obj - sizeof(heap_header_t));

    /* this chunk is no longer in use */
    p_hdr->flags = 0;

    /* can we unite this chunk with the next chunk? */
    if ((p_hdr->p_next != NULL) && (p_hdr->p_next->flags == 0)) {
        p_hdr->chunk_size += sizeof(heap_header_t) + p_hdr->p_next->chunk_size;
        p_hdr->p_next = p_hdr->p_next->p_next;
        if (p_hdr->p_next) p_hdr->p_next->p_prev = p_hdr;
    }

    /* can we unite this chunk with the previous chunk? */
    if ((p_hdr->p_prev != NULL) && (p_hdr->p_prev->flags == 0)) {
        p_hdr->p_prev->chunk_size += sizeof(heap_header_t) + p_hdr->chunk_size;
        p_hdr->p_prev->p_next = p_hdr->p_next;
        if (p_hdr->p_next) p_hdr->p_next->p_prev = p_hdr->p_prev;
    }
}

