#include <stdio.h>
#include <error.h>
#include <errno.h>
#include <unistd.h>

#include "malloc.h"
#include "stdlib.h"

int main(void) {
    // char * str2 = malloc(10);
    // printf("malloc returned %p\n", str2);
    char * str = ft_malloc(10);
    if (!str) {
        error(EXIT_FAILURE, errno, "malloc failed");
    }
    printf("ft_malloc returned %p\n", str);
    // printf("ft_malloc returned %p\n", str2);
}
