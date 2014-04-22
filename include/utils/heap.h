#ifndef __HEAP_H__
#define __HEAP_H__


#ifdef __cplusplus
extern "C" {
#endif


void heap_init();
void* heap_alloc(unsigned int size);
void heap_free(void* ptr);



#ifdef __cplusplus
}
#endif



#endif /* __HEAP_H__ */

