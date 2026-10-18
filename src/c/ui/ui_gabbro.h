#pragma once
#if defined(PBL_PLATFORM_EMERY) || defined(PBL_PLATFORM_GABBRO)
#define PHONE_ICON "\U0001F4F1"
#define STEPS_ICON "\U0001F9B6"
#define HRM_ICON "\U0001F493" 
#define WATCH_BATTERY_ICON "\U0000231A"
#define SENSOR_ICON "\U0001F319"
#else
// emojis are not available on older versions
#define PHONE_ICON " B:"
#define STEPS_ICON "s"
#define HRM_ICON "h" 
#define WATCH_BATTERY_ICON " W:"
#define SENSOR_ICON "S"
#endif

void ui_gabbro_init(void);
