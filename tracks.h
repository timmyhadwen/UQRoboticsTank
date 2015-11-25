#include "board.h"
#include "stm32f4xx_hal_conf.h"
#include "debug_printf.h"

#define TRACK_FORWARD 1
#define TRACK_BACKWARD 0

void tracks_init();
void set_left_direction(int percent);
void set_right_direction(int percent);

enum TrackSpeed {
    SPEED_FAST,
    SPEED_SLOW,
    SPEED_STOP
};
