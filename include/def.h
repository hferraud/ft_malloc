#ifndef CEEDLING_DEF_H
#define CEEDLING_DEF_H

#include <stdalign.h>

#define ALIGN_MEM(x)    ((x + ALIGN_SIZE - 1) & ~(ALIGN_SIZE - 1))
#define ALIGN_SIZE      alignof(max_align_t)

#endif //CEEDLING_DEF_H