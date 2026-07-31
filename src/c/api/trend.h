#ifndef __TREND_H__
#define __TREND_H__

#include <pebble.h>


#define TREND_LOG "\tTREND :: "

typedef int16_t bgl_value; 

typedef enum {
    BGL_TYPE_MMOL_L = 0,
    BGL_TYPE_MG_DL = 1
} bgl_type;

typedef enum {
    TREND_STYLE_DOTS = 0,
    TREND_STYLE_LINES = 1
} trend_style;

typedef struct {
    int16_t size;
    int16_t index;
    bgl_value *values;
} bgl_array;


typedef struct {
    bgl_type    bgl_type;
    GColor      good_color;
    GColor      average_color;
    GColor      high_color;
    GColor      low_color;
    GColor      critical_color;
    GColor      high_line_color;
    GColor      low_line_color;
    int8_t      line_width;
    int8_t      trend_width;
    int16_t     bgl_low;
    int16_t     bgl_average;
    int16_t     bgl_high;
    int16_t     bgl_high_line;
    int16_t     bgl_low_line;
    int16_t     bgl_high_limit;
    int16_t     bgl_low_limit;
    Layer       *layer;
    bgl_array   bgl;
    trend_style style;
} trend_config;

void trend_set_config(trend_config *cfg);
void trend_draw(void);

/**
 * convert bgl to y, respecting limits and bounds
 */
#define BGL_TO_Y(bgl, config, bounds) (\
            bounds.size.h - \
            ((int32_t) (bounds.size.h * (bgl - config->bgl_low_limit))) /\
            (config->bgl_high_limit - config->bgl_low_limit)\
        )

#endif
