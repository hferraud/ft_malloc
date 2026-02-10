#ifndef MALLOC_H
#define MALLOC_H

#include <stddef.h>

void *ft_malloc(size_t size);
void ft_free(void *ptr);
void *ft_realloc(void *ptr, size_t size);
void show_alloc_mem(void);

#endif //MALLOC_H
