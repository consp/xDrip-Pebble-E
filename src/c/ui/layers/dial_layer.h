#pragma once
#include <pebble.h>

/**
 * Due to pebble not exporting the Layer struct, we have to do some shenanigans
 */
typedef struct DialLayer {
    Layer *layer;
    bool marks_enabled;
    bool marks_small_enabled;
    bool seconds_enabled;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} DialLayer;

DialLayer *dial_init(Layer *parent, bool seconds);
void update_time(struct tm *tick);
