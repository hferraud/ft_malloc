#include <stdio.h>
#include <stdlib.h>

#include "malloc.h"
#include "free.h"

int main(void) {
    // char *data1 = ft_malloc(0);
    // show_alloc_mem();
    // data1 = ft_realloc(data1, 64);
    // show_alloc_mem();
    // data1 = ft_realloc(data1, 256);
    // show_alloc_mem();
    // data1 = ft_realloc(data1, 2048);
    // show_alloc_mem();
    // data1 = ft_realloc(data1, 256);
    // show_alloc_mem();
    // data1 = ft_realloc(data1, 4096);
    // show_alloc_mem();
    // data1 = ft_realloc(data1, 8192);
    // show_alloc_mem();

    char *data1 = ft_malloc(48);
    char *data2 = ft_malloc(64);
    display_memory();
    ft_free(data1);
    display_memory();
    ft_free(data2);
    char *data3 = ft_malloc(12);
    display_memory();
    ft_free(data3);
    display_memory();
}
