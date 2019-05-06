#ifndef PAN_AND_LEVEL_HPP
#define PAN_AND_LEVEL_HPP

#define MAX_LEVEL       24  // all levels in dB
#define INITIAL_LEVEL   72  // all levels in dB
#define MIN_LEVEL      -24  // all levels in dB
#define LEVEL_REL_INCR  15.0

#define MIN_PAN         -0.999 // Right
#define MAX_PAN          0.999 // Left
#define PAN_REL_INCR     15.0

#define CENTER_PAN_VALUE 0.0
#define INITIAL_PAN   CENTER_PAN_VALUE
#define INITIAL_PAN_OFFSET ((MAX_PAN - MIN_PAN) / 5)
#define PAN_STEPS 50 // increment = (MAX_PAN - MIN_PAN) / PAN_STEPS

float equal_loudness_value(int _index);
float min_equal_loudness_value(void);

void init_pan_and_level(float level[NR_OF_BANDS], float left_pan[NR_OF_BANDS], float right_pan[NR_OF_BANDS], float left_level[NR_OF_BANDS], float right_level[NR_OF_BANDS]);

void set_pan_and_level(float level[NR_OF_BANDS], float left_pan[NR_OF_BANDS], float right_pan[NR_OF_BANDS], float left_level[NR_OF_BANDS], float right_level[NR_OF_BANDS], float width);

void initialize_levels(float level[NR_OF_BANDS]);
#endif
