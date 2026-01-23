#include <stdio.h>
#include <stdlib.h>

#include "malloc.h"

int main(void) {
    // for (int i = 0; i < 128; i++) {
    //     printf("%d\n", i);
    //     ft_malloc(128);
    // }
    char *data1 = ft_malloc(64);
    char *data2 = ft_malloc(64);
    char *data3 = ft_malloc(64);
    data3[0] = 0xff;
    ft_free(data1);
    ft_free(data2);

    show_alloc_mem();
}
