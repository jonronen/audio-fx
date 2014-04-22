#include <stdio.h>
#include "heap.h"


int main(int argc, char* argv[])
{
    unsigned char *p, *q, *r, *s, *t, *u, *v;

    heap_init();

    p = (unsigned char*)heap_alloc(1);
    printf("p: %p\n", p);

    q = (unsigned char*)heap_alloc(0xffffffff);
    printf("q: %p\n", q);

    q = (unsigned char*)heap_alloc(4);
    printf("q: %p\n", q);

    r = (unsigned char*)heap_alloc(4);
    printf("r: %p\n", r);

    heap_free(q);

    q = (unsigned char*)heap_alloc(4);
    printf("q: %p\n", q);

    heap_free(p);

    p = (unsigned char*)heap_alloc(4);
    printf("p: %p\n", p);

    heap_free(r);

    r = (unsigned char*)heap_alloc(16);
    printf("r: %p\n", r);

    s = (unsigned char*)heap_alloc(16);
    printf("s: %p\n", s);

    t = (unsigned char*)heap_alloc(16);
    printf("t: %p\n", t);

    heap_free(r);
    heap_free(s);

    r = (unsigned char*)heap_alloc(32);
    printf("r: %p\n", r);

    heap_free(q);

    u = (unsigned char*)heap_alloc(32);
    printf("u: %p\n", u);

    v = (unsigned char*)heap_alloc(100);
    printf("v: %p\n", v);

    heap_free(p);
}

