#include <pebble.h>
#include "gauge_layer.h"
#include "../ui_helper.h"
#include "../../debug.h"
#include "../../api/settings.h"

extern AppState state;

#define CENTER(bounds) grect_center_point(&bounds)
#define RADIUS(bounds) (bounds.size.w / 2)

static void update_gauge(Layer *layer, GContext *ctx);

GaugeLayer *gauge_init(Layer *parent, GRect bounds) {
    if (bounds.size.w != bounds.size.h) {
        ERROR("Gauges should be symmetrical in height and width");
        return NULL;
    }
    TRACE("Gauge: %dx%d @ %dx%d", bounds.size.w, bounds.size.h, bounds.origin.x, bounds.origin.y);
    Layer *layer = layer_create_with_data(bounds, sizeof(GaugeLayer));
    GaugeLayer *gauge = layer_get_data(layer);
    gauge->layer  = layer; // self
    layer_add_child(parent, gauge->layer);

    gauge_set_min_max(gauge, 0.0f, 0.1f);
    gauge->value = 0.5f;
    gauge->start_top = true;
    gauge->show_value = true;
    gauge->show_band = true;
    gauge->show_marks = true;
    gauge->mark_interval = 0.1f;
    gauge->border = true;
    gauge->border_in = false;

    layer_set_update_proc(gauge->layer, update_gauge);
    gauge_draw(gauge);
    return gauge;
}

void gauge_deinit(GaugeLayer *gauge) {
    if (gauge->icon) free(gauge->icon);
    if (gauge->text) free(gauge->text);
    layer_destroy(gauge->layer);
    // pointer is invalid from this point
}

void gauge_set_value(GaugeLayer *gauge, float value, bool is_integer) {
    gauge->value = value;
    gauge->value_is_integer = is_integer;
}

void gauge_set_min_max(GaugeLayer *gauge, float min, float max) {
    gauge->min = min;
    gauge->max = max;
    gauge->mark_interval = (max - min) / 9; // 10 updates, min == max
}

void gauge_set_low(GaugeLayer *gauge, float min, float max, GColor colour) {
    gauge->low.min = min;
    gauge->low.max = max;
    gauge->low.colour = colour;
}

void gauge_set_good(GaugeLayer *gauge, float min, float max, GColor colour) {
    gauge->good.min = min;
    gauge->good.max = max;
    gauge->good.colour = colour;
}

void gauge_set_high(GaugeLayer *gauge, float min, float max, GColor colour) {
    gauge->high.min = min;
    gauge->high.max = max;
    gauge->high.colour = colour;
}

void gauge_set_normal(GaugeLayer *gauge, float min, float max, GColor colour) {
    gauge->normal.min = min;
    gauge->normal.max = max;
    gauge->normal.colour = colour;
}

void gauge_draw(GaugeLayer *gauge) {
    layer_mark_dirty(gauge->layer);
}

void gauge_set_icon_text(GaugeLayer *gauge, char *text) {
    if (gauge->icon != NULL) free(gauge->icon);

    int len = strlen(text) + 1;
    gauge->icon = calloc(1, len);
    memcpy(gauge->icon, text, len);
}


void gauge_set_text(GaugeLayer *gauge, char *text) {
    if (gauge->text != NULL) free(gauge->text);

    int len = strlen(text) + 1;
    gauge->text = calloc(1, len);
    memcpy(gauge->text, text, len);
}

static void update_border(GaugeLayer *gauge, GContext *ctx) {
    TRACE("Border: %d", gauge->border);
    if (!gauge->border) return;
    GRect bounds = layer_get_bounds(gauge->layer);
    graphics_context_set_antialiased(ctx, false);
    graphics_context_set_stroke_color(ctx, gauge->border_in ? GColorDarkGray : GColorLightGray);
    graphics_context_set_fill_color(ctx, gauge->border_in ? GColorDarkGray : GColorLightGray);
    graphics_context_set_stroke_width(ctx, 2);
    // draw outside
    /* graphics_draw_circle(ctx, CENTER(bounds), RADIUS(bounds) - 2); */
    graphics_fill_radial(ctx, bounds, GOvalScaleModeFillCircle, 2, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(360));
    graphics_context_set_antialiased(ctx, true);
    graphics_context_set_stroke_color(ctx, gauge->border_in ? GColorLightGray : GColorDarkGray);
    graphics_context_set_fill_color(ctx, gauge->border_in ? GColorLightGray : GColorDarkGray);
    graphics_context_set_stroke_width(ctx, 2);
    /* graphics_draw_circle(ctx, CENTER(bounds), RADIUS(bounds) - 3); */
    bounds.origin.x += 1; bounds.origin.y += 1;
    bounds.size.w -= 2; bounds.size.h -= 2;
    graphics_fill_radial(ctx, bounds, GOvalScaleModeFillCircle, 1, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(360));
}

static void update_band(GaugeLayer *gauge, GContext *ctx) {
    TRACE("Band: %d", gauge->show_band);
    if (!gauge->show_band) return;
    GRect bounds = layer_get_bounds(gauge->layer);
    bounds.origin.x += 2;
    bounds.origin.y += 2;
    bounds.size.w -= 4;
    bounds.size.h -= 4;
    graphics_context_set_antialiased(ctx, true);
    if (gauge->low.min != gauge->low.max && !gcolor_equal(gauge->low.colour, GColorClear)) {
        graphics_context_set_stroke_color(ctx, gauge->low.colour);
        graphics_context_set_fill_color(ctx, gauge->low.colour);
        graphics_fill_radial(ctx, bounds, GOvalScaleModeFillCircle,
                5,
                DEG_TO_TRIGANGLE((gauge->low.min - gauge->min) * (360 / (gauge->max - gauge->min))),
                DEG_TO_TRIGANGLE((gauge->low.max - gauge->min) * (360 / (gauge->max - gauge->min))));
    }

    if (gauge->normal.min != gauge->normal.max && !gcolor_equal(gauge->normal.colour, GColorClear)) {
        graphics_context_set_stroke_color(ctx, gauge->normal.colour);
        graphics_context_set_fill_color(ctx, gauge->normal.colour);
        graphics_fill_radial(ctx, bounds, GOvalScaleModeFillCircle,
                5,
                DEG_TO_TRIGANGLE((gauge->normal.min - gauge->min) * (360 / (gauge->max - gauge->min))),
                DEG_TO_TRIGANGLE((gauge->normal.max - gauge->min) * (360 / (gauge->max - gauge->min))));
    }

    if (gauge->good.min != gauge->good.max && !gcolor_equal(gauge->good.colour, GColorClear)) {
        graphics_context_set_stroke_color(ctx, gauge->good.colour);
        graphics_context_set_fill_color(ctx, gauge->good.colour);
        graphics_fill_radial(ctx, bounds, GOvalScaleModeFillCircle,
                5,
                DEG_TO_TRIGANGLE((gauge->good.min - gauge->min) * (360 / (gauge->max - gauge->min))),
                DEG_TO_TRIGANGLE((gauge->good.max - gauge->min) * (360 / (gauge->max - gauge->min))));
    }
    if (gauge->high.min != gauge->high.max && !gcolor_equal(gauge->high.colour, GColorClear)) {
        graphics_context_set_stroke_color(ctx, gauge->high.colour);
        graphics_context_set_fill_color(ctx, gauge->high.colour);
        graphics_fill_radial(ctx, bounds, GOvalScaleModeFillCircle,
                5,
                DEG_TO_TRIGANGLE((gauge->high.min - gauge->min) * (360 / (gauge->max - gauge->min))),
                DEG_TO_TRIGANGLE((gauge->high.max - gauge->min) * (360 / (gauge->max - gauge->min))));
    }
}

static void update_marks(GaugeLayer *gauge, GContext *ctx) {
    TRACE("Marks: %d", gauge->show_marks);
    if (!gauge->show_marks) return;

    GRect bounds = layer_get_unobstructed_bounds(gauge->layer);
    GPoint center = grect_center_point(&bounds);

    graphics_context_set_stroke_color(ctx, BW_BY_LUMINANCE(state.background_colour));
    graphics_context_set_stroke_width(ctx, 1);

    int radius = bounds.size.w / 2;
    if (gauge->border) radius -= 1;
    if (gauge->border_in) radius -= 1;

    for (int i = 0; i < 10; i++) {
        GPoint start = (GPoint) {
            .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * (float) i / 10) * (int32_t)(radius) / TRIG_MAX_RATIO) + center.x,
            .y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * ((float) i) / 10) * (int32_t)(radius) / TRIG_MAX_RATIO) + center.y,
        };
        GPoint stop = (GPoint) {
            .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * (float) i / 10) * (int32_t)(radius - 5) / TRIG_MAX_RATIO) + center.x,
            .y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * (float) i / 10) * (int32_t)(radius - 5) / TRIG_MAX_RATIO) + center.y,
        };
        graphics_draw_line(ctx, start, stop);
    }
}

static void update_dial(GaugeLayer *gauge, GContext *ctx) {
    GRect bounds = layer_get_unobstructed_bounds(gauge->layer);
    GPoint center = grect_center_point(&bounds);

    float angle = TRIG_MAX_ANGLE * (float)gauge->value / (gauge->max - gauge->min);

    int radius = (bounds.size.w / 2) - 1;
    if (gauge->border) radius -= 1;
    if (gauge->border_in) radius -= 1;

    GPoint end = (GPoint) {
        .x = (int16_t)(sin_lookup(angle) * (int32_t)(radius) / TRIG_MAX_RATIO) + center.x,
        .y = (int16_t)(-cos_lookup(angle) * (int32_t)(radius) / TRIG_MAX_RATIO) + center.y,
    };

    // draw text
    GFont icon_font = fonts_get_system_font(FONT_KEY_GOTHIC_18);
    GFont text_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);


    char text[6]; // max 999.9
    uint32_t stride = snprintf(text, sizeof(text), "%d", (int) gauge->value);
    if (!gauge->value_is_integer && stride < sizeof(text) - 1) {
        // cannot printf float, do in parts
        snprintf(&text[stride], sizeof(text) - stride, ".%d", (int)((float)gauge->value * 10.0) % 10);
    }
    gauge_set_text(gauge, text);

    WARNING("%s", gauge->icon);
    WARNING("%s", gauge->text);

    GRect icon_bounds = GRect(
            (bounds.size.w * 2) / 10, (bounds.size.h / 2) - 5,
            (bounds.size.w * 6) / 10, (bounds.size.h * 35) / 100);
    GRect text_bounds = GRect(
            (bounds.size.w * 2) / 10, 6,
            (bounds.size.w * 6) / 10, (bounds.size.h * 3) / 10);

    GColor colour = BW_BY_LUMINANCE(state.background_colour);
    // alpha only works on bitmaps need some magic
    // text is inverse b or w, other is inverse gray
    graphics_context_set_text_color(ctx, colour);
    if (gauge->text != NULL) graphics_draw_text(ctx, gauge->text, text_font, text_bounds, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);

    if (gcolor_equal(colour, GColorWhite)) colour = GColorDarkGray;
    if (gcolor_equal(colour, GColorBlack)) colour = GColorLightGray;
    graphics_context_set_text_color(ctx, colour);
    if (gauge->icon != NULL) graphics_draw_text(ctx, gauge->icon, icon_font, icon_bounds, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);

    // draw arm
    graphics_context_set_stroke_color(ctx, BW_BY_LUMINANCE(state.background_colour));
    graphics_context_set_stroke_width(ctx, 2);

    graphics_draw_line(ctx, center, end);
}

static void update_gauge(Layer *layer, GContext *ctx) {
    GaugeLayer *gauge = layer_get_data(layer);
    GRect bounds = layer_get_bounds(layer);
    // Wipe
    graphics_context_set_stroke_color(ctx, state.background_colour);
    graphics_context_set_fill_color(ctx, state.background_colour);
    graphics_fill_circle(ctx, CENTER(bounds), RADIUS(bounds));

    // Draw background
    update_border(gauge, ctx);
    update_band(gauge, ctx);
    update_marks(gauge, ctx);

    // Draw gauge & text
    update_dial(gauge, ctx); // also draws text
}
