#include <pebble.h>

typedef struct Range {
    GColor colour;
    float min;
    float max;
} Range;

typedef struct GaugeLayer {
    Layer *layer;
    
    bool show_band;
    bool show_marks;
    bool show_value;
    bool value_is_integer;
    bool start_top;
    bool border;
    bool border_in;

    float min;
    float max;
    float mark_interval;
    float value;

    Range low;
    Range normal;
    Range good;
    Range high;

    char *icon;
    char *text;

} GaugeLayer;

void gauge_draw(GaugeLayer *gauge);

void gauge_set_min_max(GaugeLayer *gauge, float min, float max);
void gauge_set_value(GaugeLayer *gauge, float value, bool is_integer);
GaugeLayer *gauge_init(Layer *parent, GRect bounds);

void gauge_set_high(GaugeLayer *gauge, float min, float max, GColor colour);
void gauge_set_low(GaugeLayer *gauge, float min, float max, GColor colour);
void gauge_set_normal(GaugeLayer *gauge, float min, float max, GColor colour);
void gauge_set_good(GaugeLayer *gauge, float min, float max, GColor colour);

void gauge_set_icon_text(GaugeLayer *gauge, char *text);
