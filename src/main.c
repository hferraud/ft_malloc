#include <stdio.h>
#include <stdlib.h>

#include "malloc.h"

int main(void) {
    char *data1 = ft_malloc(0);
    show_alloc_mem();
    data1 = ft_realloc(data1, 64);
    show_alloc_mem();
    data1 = ft_realloc(data1, 256);
    show_alloc_mem();
    data1 = ft_realloc(data1, 2048);
    show_alloc_mem();
    data1 = ft_realloc(data1, 256);
    show_alloc_mem();
    // data1 = ft_realloc(data1, 4096);
    // show_alloc_mem();
    // data1 = ft_realloc(data1, 8192);
    // show_alloc_mem();

    // char *data1 = ft_malloc(8192);
    // char *data2 = ft_malloc(4100);
    // data1[0] = 0xff;
    // show_alloc_mem();
    // ft_free(data1);
    // show_alloc_mem();
    // ft_realloc(data2, 4242);
    // ft_free(data2);
    // char *data3 = ft_malloc(4500);
    // show_alloc_mem();
    // ft_free(data3);
    // show_alloc_mem();
}
