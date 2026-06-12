#include "display.h"
#include "malloc.h"

int main(void) {
    malloc(64);
    display_memory();
    display_memory_ex();
    return 0;
}