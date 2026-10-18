#if defined(PBL_PLATFORM_GABBRO)
#include <pebble.h>
#include "../xdrip.h"
#include <stdint.h>
#include "../debug.h"
#include "../api/settings.h"
#include "../api/trend.h"
#ifdef ENABLE_TOUCH
#include "ui_treatment.h"
#endif
#include "../api/callbacks.h"
#include "layers/dial_layer.h"
#include "layers/gauge_layer.h"


#ifdef ENABLE_COMM_FRAMEWORK
#include "../api/communication.h"
#endif

#include "ui_gabbro.h"

#define T_WIDTH 60
#define T_HEIGHT 30

/* variables */
/* fonts */
GFont gb_time_font_large;
GFont gb_time_font_medium;
GFont gb_time_font_normal;
GFont gb_time_font_small;

/* component layers */
DialLayer *dial = NULL;
Layer *trend_canvas = NULL;
Layer *trend_layer = NULL;
GaugeLayer *g1 = NULL;
GaugeLayer *g2 = NULL;
GaugeLayer *g3 = NULL;

Window *gb_window = NULL;

/* app state */
extern AppState state;

/* Set functions */
void ui_gb_set_message(char *message, size_t length) {}
void ui_gb_set_icon(uint8_t icon) {}
void ui_gb_set_delta(char *message, size_t length) {}
void ui_gb_set_delta_colour(GColor colour) {}
void ui_gb_set_cgmtime(char *message, size_t length) {}
/* comms callbacks */

void ui_gb_comm_set_sensor_info(comm_sensor_info *value) {}
void ui_gb_comm_set_bgl_series(comm_bgl_series *series) {}
void ui_gb_comm_set_message(comm_message message) {
    ui_gb_set_message(message.message, message.length);
}
void ui_gb_comm_set_png(comm_png_data *data) {}
void ui_gb_comm_set_bgl_data(comm_bgl_data *value) {}
void ui_gb_comm_set_bgl_value(comm_bgl_value value) {}
void ui_gb_comm_set_bgl_timestamp(uint32_t timestamp) {}
void ui_gb_comm_set_bwp(comm_bwp_value value) {}
void ui_gb_comm_set_phone_battery(comm_phonebat value) {}
void ui_gb_comm_set_icon(comm_slopeval value) {}
void ui_gb_comm_set_bgl_delta(comm_bgl_delta value) {}
void ui_gb_comm_set_vibrate(comm_vibe value) {}

void ui_gb_update_message(void) {}
void ui_gb_update_collect_health(void) {}
void ui_gb_update_message_timeout(uint32_t timeout) {}
void ui_gb_update_timeago(void) {}

void ui_gb_update_trend(void) {}

GRect ui_gabbro_trend_bounds(void) { return GRect(0, 0, 1, 1); } 

void gb_minutes_tick(struct tm *tick, TimeUnits units_changed) {
    update_time(tick);
}
void gb_seconds_tick(struct tm* tick, TimeUnits units_changed) {
    update_time(tick);
}

void gb_trend_canvas_update(Layer *layer, GContext *ctx) {
    GRect bounds = layer_get_unobstructed_bounds(layer);

    graphics_context_set_stroke_color(ctx, GColorDarkGray);
    graphics_context_set_stroke_width(ctx, 1);
    graphics_draw_rect(ctx, GRect(0, 0, bounds.size.w, bounds.size.h));
}

void gb_load_window(Window *window) {
    DEBUG("Load gb window");

    Layer *window_layer = window_get_root_layer(window);


    GRect bounds = layer_get_unobstructed_bounds(window_layer);
    GPoint center = grect_center_point(&bounds);

    trend_canvas = layer_create(GRect(center.x - (bounds.size.w * T_WIDTH / 200), center.y - (bounds.size.h * T_HEIGHT / 100), (bounds.size.w * T_WIDTH) / 100, (bounds.size.h * T_HEIGHT) / 100));
    trend_layer = layer_create(GRect(1, 1, (bounds.size.w * T_WIDTH / 100) - 2, (bounds.size.h * T_HEIGHT / 100) - 2));

    layer_add_child(window_layer, trend_canvas);
    layer_add_child(trend_canvas, trend_layer);
    layer_set_update_proc(trend_canvas, gb_trend_canvas_update);

    trend_init(trend_layer);

    // gauges
    int gauge_width = (PBL_DISPLAY_WIDTH * 20) / 100;
    g1 = gauge_init(window_layer, GRect(((PBL_DISPLAY_WIDTH * 25) / 100) - (gauge_width / 2), (PBL_DISPLAY_HEIGHT / 2) + 10, gauge_width, gauge_width));
    g2 = gauge_init(window_layer, GRect(((PBL_DISPLAY_WIDTH * 50) / 100) - (gauge_width / 2), (PBL_DISPLAY_HEIGHT / 2) + 10, gauge_width, gauge_width));
    g3 = gauge_init(window_layer, GRect(((PBL_DISPLAY_WIDTH * 75) / 100) - (gauge_width / 2), (PBL_DISPLAY_HEIGHT / 2) + 10, gauge_width, gauge_width));

    gauge_set_min_max(g1, 0.0, 100.0);
    gauge_set_low(g1, 0.0, 20.0, GColorSunsetOrange);
    gauge_set_normal(g1, 20.0, 40.0, GColorChromeYellow); 
    gauge_set_good(g1, 40.0, 75.0, GColorClear);
    gauge_set_high(g1, 75.0, 100.0, GColorIslamicGreen);
    gauge_set_value(g1, 65.0, true);
    gauge_set_icon_text(g1, WATCH_BATTERY_ICON);

    gauge_set_min_max(g2, 0.0, 16.0);
    gauge_set_low(g2, 0.0, 4.0, GColorVividCerulean);
    gauge_set_good(g2, 4.5, 7.5, GColorSpringBud);
    gauge_set_normal(g2, 10.0, 14.0, GColorChromeYellow); 
    gauge_set_high(g2, 14.0, 16.0, GColorRed);
    gauge_set_value(g2, 6.5, false);
    gauge_set_icon_text(g2, "bg");

    gauge_set_min_max(g3, 0.0, 10.5);
    gauge_set_low(g3, 0.0, 1.0, GColorIcterine);
    gauge_set_value(g3, 5.66, false);
    gauge_set_icon_text(g3, SENSOR_ICON);

    dial = dial_init(window_layer, state.enable_seconds);
}
void gb_unload_window(Window *window) {
    DEBUG("Unload gb window");
}

void ui_gabbro_init(void) {
    DEBUG("Creating GB window");
/*     state.wf_cb.set_message = ui_gb_set_message; */
/*     state.wf_cb.set_icon = ui_gb_set_icon; */
/*     state.wf_cb.set_delta = ui_gb_set_delta; */
/*     state.wf_cb.set_delta_colour = ui_gb_set_delta_colour; */
/*     state.wf_cb.set_cgmtime = ui_gb_set_cgmtime; */
/*     state.wf_cb.update_battery_state = NULL; */
/*     state.wf_cb.update_phone_battery_state = NULL; */
/*     state.wf_cb.update_seconds_timer = NULL; */
/*     state.wf_cb.update_timeago = ui_gb_update_timeago; */
/*     state.wf_cb.update_message_timeout = ui_gb_update_message_timeout; */
/*     state.wf_cb.update_timeago = ui_gb_update_timeago; */
/*     state.wf_cb.update_left_field = NULL; */
/*     state.wf_cb.update_right_field = NULL; */
/*     state.wf_cb.update_trend = ui_gb_update_trend; */
/*     state.wf_cb.update_collect_health = ui_gb_update_collect_health; */
/*     state.wf_cb.update_message = ui_gb_update_message; */
/* #ifdef ENABLE_TOUCH */
/*     state.wf_cb.update_touch = update_touch; */
/*     state.wf_cb.update_touch_methods = update_touch_method; */
/* #endif */
/* #ifdef PBL_COLOR */
/*     state.wf_cb.update_colours = NULL; */
/* #endif */
/*     state.wf_cb.trend_bounds = ui_gabbro_trend_bounds; */

    // timers
    state.wf_cb.minutes_tick = gb_minutes_tick;
    state.wf_cb.seconds_tick = gb_seconds_tick;

/* #ifdef ENABLE_TREND_RENDERER */
/*     state.comm_callbacks.low_limit = trend_set_low_line; */
/*     state.comm_callbacks.high_limit = trend_set_high_line; */
/* #endif */
/*     state.comm_callbacks.message = ui_gb_comm_set_message; */
/*     state.comm_callbacks.phonebat = ui_gb_comm_set_phone_battery; */
/*     state.comm_callbacks.slopeval = ui_gb_comm_set_icon; */
/*     state.comm_callbacks.vibe = ui_gb_comm_set_vibrate; */
/*     state.comm_callbacks.bgl_delta = ui_gb_comm_set_bgl_delta; */
/*     state.comm_callbacks.bgl_series = ui_gb_comm_set_bgl_series;  */
/*     state.comm_callbacks.bgl_data = ui_gb_comm_set_bgl_data; */
/*     state.comm_callbacks.bgl_timestamp = ui_gb_comm_set_bgl_timestamp; */
/*     state.comm_callbacks.bgl_value = ui_gb_comm_set_bgl_value; */
/*     state.comm_callbacks.png = ui_gb_comm_set_png; */
/*     state.comm_callbacks.sensor_info = ui_gb_comm_set_sensor_info; */
    // the watch is the health data source, so nothing to receive
    state.comm_callbacks.health = NULL;

    
    gb_time_font_large = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_GOTHAM_BOLD_BG_64));
    gb_time_font_medium = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_GOTHAM_BOLD_54));
    gb_time_font_normal = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_GOTHAM_BOLD_40));
    gb_time_font_small = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_GOTHAM_BOLD_30));

    state.background_colour = GColorBlack;
    state.foreground_colour = GColorWhite;

    /* state.background_colour = GColorWhite; */
    /* state.foreground_colour = GColorBlack; */

    gb_window = window_create();
    window_set_background_color(gb_window, state.background_colour);
    window_set_window_handlers(gb_window, (WindowHandlers)
    {
        .load = gb_load_window,
        .unload = gb_unload_window
    });

    window_stack_push(gb_window, true);

    if (!state.show_trend) {
        layer_set_hidden(trend_canvas, true);
        layer_set_hidden(trend_layer, true);
    }


#ifdef ENABLE_TOUCH
    /* update_touch(); */
#endif
}
#endif
