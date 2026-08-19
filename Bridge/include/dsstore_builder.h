#ifndef DSSTORE_BUILDER_H
#define DSSTORE_BUILDER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t x;
    uint32_t y;
} dsstore_icon_pos_t;

typedef struct {
    const char* filename;
    dsstore_icon_pos_t pos;
} dsstore_icon_entry_t;

typedef struct {
    uint32_t window_x;
    uint32_t window_y;
    uint32_t window_w;
    uint32_t window_h;
    
    int icon_size;
    const char* background_image_path; 
    
    const dsstore_icon_entry_t* icons;
    size_t icon_count;
} dsstore_config_t;

uint8_t* dsstore_build(const dsstore_config_t* config, size_t* out_size);

#ifdef __cplusplus
}
#endif

#endif
