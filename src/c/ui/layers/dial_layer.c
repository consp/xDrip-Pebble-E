#include <pebble.h>
#include "dial_layer.h"
#include "../ui_helper.h"
#include "../../debug.h"
#include "../../api/settings.h"

#define HAND_MARGIN 5

/**
 * For now ther can only be one dial
 */

extern AppState state;

static DialLayer dial;
uint8_t minute_radius = 0, hour_radius = 0, second_radius = 0, mark_start = 0, mark_end = 0, mark_small_end = 0;

void update_time(struct tm *tick) {
  dial.hour = tick->tm_hour % 12;
  dial.minute = tick->tm_min;
  dial.second = tick->tm_sec;
  layer_mark_dirty(dial.layer);
}

static void update_marks(Layer *layer, GContext *ctx) {
    if (!dial.marks_enabled) return;
    GRect bounds = layer_get_unobstructed_bounds(layer);
    GPoint center = grect_center_point(&bounds);

    GColor8 t = INVERT_COLOUR(state.background_colour);
    GColor c = state.background_colour;
    
    graphics_context_set_stroke_color(ctx, BW_BY_LUMINANCE(state.background_colour));
    graphics_context_set_stroke_width(ctx, 2);
    graphics_context_set_antialiased(ctx, true);
    graphics_context_set_compositing_mode(ctx, GCompOpAssignInverted);

    for (int i = 0; i < 60; i++) {
        GPoint start = (GPoint) {
            .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * (float) i / 60) * (int32_t)(mark_start) / TRIG_MAX_RATIO) + center.x,
            .y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * ((float) i) / 60) * (int32_t)(mark_start) / TRIG_MAX_RATIO) + center.y,
        };
        GPoint stop = (GPoint) {
            .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * (float) i / 60) * (int32_t)(mark_end) / TRIG_MAX_RATIO) + center.x,
            .y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * (float) i / 60) * (int32_t)(mark_end) / TRIG_MAX_RATIO) + center.y,
        };
        GPoint small_stop = (GPoint) {
            .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * (float) i / 60) * (int32_t)(mark_small_end) / TRIG_MAX_RATIO) + center.x,
            .y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * (float) i / 60) * (int32_t)(mark_small_end) / TRIG_MAX_RATIO) + center.y,
        };
        if (i % 5 == 0) graphics_draw_line(ctx, start, stop);
        if (dial.marks_small_enabled && i % 5 != 0) {
            graphics_draw_line(ctx, start, small_stop);
        }
    }
}

static void update_dial(Layer *layer, GContext *ctx) {
    GRect bounds = layer_get_unobstructed_bounds(layer);
    GPoint center = grect_center_point(&bounds);

    /* graphics_context_set_fill_color(ctx, state.background_colour); */
    /* graphics_fill_rect(ctx, full_bounds, 0, GCornerNone); */

    graphics_context_set_antialiased(ctx, true);

    update_marks(layer, ctx);

    graphics_context_set_stroke_color(ctx, state.foreground_colour);
    graphics_context_set_stroke_width(ctx, 4);

    // Adjust for minutes through the hour
    float minute_angle = TRIG_MAX_ANGLE * (float)dial.minute / 60.0;
    float second_angle = TRIG_MAX_ANGLE * (float)dial.second / 60.0;
    float hour_angle;
    hour_angle = TRIG_MAX_ANGLE * (float)dial.hour / 12.0;
    hour_angle += (minute_angle / TRIG_MAX_ANGLE) * (TRIG_MAX_ANGLE / 12.0);

    // Plot hands
    GPoint minute_hand = (GPoint) {
        .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * dial.minute / 60) * (int32_t)(minute_radius) / TRIG_MAX_RATIO) + center.x,
        .y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * dial.minute / 60) * (int32_t)(minute_radius) / TRIG_MAX_RATIO) + center.y,
    };
    GPoint hour_hand = (GPoint) {
        .x = (int16_t)(sin_lookup(hour_angle) * (int32_t)(hour_radius) / TRIG_MAX_RATIO) + center.x,
        .y = (int16_t)(-cos_lookup(hour_angle) * (int32_t)(hour_radius) / TRIG_MAX_RATIO) + center.y,
    };

    GPoint second_hand = (GPoint) {
        .x = (int16_t)(sin_lookup(second_angle) * (int32_t)(second_radius) / TRIG_MAX_RATIO) + center.x,
        .y = (int16_t)(-cos_lookup(second_angle) * (int32_t)(second_radius) / TRIG_MAX_RATIO) + center.y,
    };

    // Draw hands with positive length only
    graphics_context_set_stroke_width(ctx, 6);
    graphics_draw_line(ctx, center, hour_hand);
    graphics_context_set_stroke_width(ctx, 4);
    graphics_draw_line(ctx, center, minute_hand);

    if (dial.seconds_enabled) {
        graphics_context_set_stroke_width(ctx, 2);
        graphics_draw_line(ctx, center, second_hand);
    }
}

DialLayer *dial_init(Layer *parent, bool seconds) {
    GRect bounds = layer_get_unobstructed_bounds(parent);

    minute_radius = (bounds.size.w * 86) / 200;
    hour_radius = (bounds.size.w * 50) / 200;
    second_radius = (bounds.size.w * 74) / 200; 
    mark_start = (bounds.size.w) / 2;
    mark_end = (bounds.size.w - 18) / 2;
    mark_small_end = (bounds.size.w - 8) / 2;

    dial.seconds_enabled = seconds;
    dial.marks_enabled = true;
    dial.marks_small_enabled = true;

    dial.layer = layer_create(bounds);
    layer_set_update_proc(dial.layer, update_dial);
    layer_add_child(parent, dial.layer);

    const time_t ctime = time(NULL);
    struct tm *ltime = localtime(&ctime);
    update_time(ltime);
    return &dial;
}
