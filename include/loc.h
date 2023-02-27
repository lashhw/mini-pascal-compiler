#ifndef LOC_H
#define LOC_H

#include <cstdint>

typedef struct {
    uint32_t first_line;
    uint32_t first_column;
    uint32_t last_line;
    uint32_t last_column;
} LocType;

#endif
