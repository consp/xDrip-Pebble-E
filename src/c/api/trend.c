#include <pebble.h>

#include "../xdrip.h"
#include "../debug.h"

#include "trend.h"

/**
 * File contains functions for drawing the trend image from a list of BGL values vs time
 *
 * Options/constraints:
 * - Draw only lates <n>, e.g. if you have 500 items draw only <pixel width of draw box>
 * - Draw all and aliase
 * - Color/gray(scale)
 * - Enable/disable high/low line
 * 
 * tbd:
 * - Set colors
 * - Set line colors
 */

static void trend_layer_callback(Layer *layer, GContext *ctx);

trend_config *config = NULL;

void trend_set_config(trend_config *cfg) {
    TRACE(TREND_LOG "Loading config");
    config = cfg;
    TRACE(TREND_LOG "Setting callback");
    layer_set_update_proc((Layer *) config->layer, trend_layer_callback);
}


inline void draw_bgl_point(bgl_value value, int16_t x, trend_config *config, GRect bounds, GContext *ctx) {
    /**
     * Since the trend image is just a graph, we do not need to know the 
     * actual type of data
     */

    GColor color = config->good_color;

    if (value > config->bgl_high) color = config->high_color;
    else if (value > config->bgl_average) color = config->average_color;
    else if (value < config->bgl_low) color = config->low_color;

    graphics_context_set_stroke_color(ctx, color);
    
    GPoint point = {x, BGL_TO_Y(value, config, bounds)};

    graphics_draw_circle(ctx, point, 0);

}

static bool draw_trend(trend_config *config, Layer *layer, GContext *ctx) {
    graphics_context_set_stroke_width(ctx, config->trend_width); // constant

    GRect bounds = layer_get_bounds(layer);

    for (int i = 0; i < bounds.size.w; i++) {
        draw_bgl_point(config->bgl.values[(config->bgl.index + i) % config->bgl.size], i, config, bounds, ctx); 
    }
    return true;
}

static bool draw_trend_lines(trend_config  *config, Layer *layer, GContext *ctx) {
    // top is 0,0
    GRect bounds = layer_get_bounds(layer);
    const int16_t h = BGL_TO_Y(config->bgl_high_line, config, bounds); 
    const int16_t l = BGL_TO_Y(config->bgl_low_line, config, bounds); 
    TRACE(TREND_LOG " Draw lines, high: %d, low %d", h, l);

    // drawing
    graphics_context_set_stroke_color(ctx, config->high_line_color);
    graphics_context_set_stroke_width(ctx, config->line_width);
    graphics_draw_line(ctx, (GPoint) { 0, h }, (GPoint) { bounds.size.w, h});
    graphics_context_set_stroke_color(ctx, config->low_line_color);
    graphics_draw_line(ctx, (GPoint) { 0, l }, (GPoint) { bounds.size.w, l});
    
    return true;
}

 void trend_layer_callback(Layer *layer, GContext *ctx) {
   TRACE(TREND_LOG "Layer callback"); 
   TRACE(TREND_LOG "Drawing trend line");
   draw_trend(config, layer, ctx);
   TRACE(TREND_LOG "Drawing high/low lines");
   draw_trend_lines(config, layer, ctx);
}

void trend_draw(void) {
    if (config == NULL) {
        INFO(TREND_LOG "No trend configuration set");
        return;
    }
    DEBUG(TREND_LOG "Marking trend layer dirty");
    layer_mark_dirty(config->layer);
}
