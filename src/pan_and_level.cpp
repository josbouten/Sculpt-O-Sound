#include "std.hpp"
#include <math.h>
#include "pan_and_level.hpp"

float equal_loudness_value(int _index)
{
  // These equal loudness values were derived from the 60 foon curve published in
  // Binas Informatieboek VWO-HAVO, Wolters Noordhoff,  ISBN 90 01 89354 6
  //
  float equal_loudness_curve_at_60_foon[]=
  {  97.14, 93.688, 89.2, 83.33, 74.428, 73.644, 70.004, 65.714, 63.928, 61.428,
//   20,    25,     31.5, 40,    50,     63,     80,    100,    125,    160,
     58.5714, 57.976, 57.202, 56.19, 55,   56.04, 57.4,  59,    58.892, 58.742,
//  200,     250,    315,    400,   500,  630,   800,  1000,  1250,   1600,
     58.57, 57.38,   55.832,   53.809,   51.428,   60,   65.71,   62.857,    59.881,   55.714,
// 2000,  2500, 3150,     4000,     5000,     6300, 8000,   10000,     12500,    16000,
        60};
//20000

// max = 97.14
// min = 51.428
  return(equal_loudness_curve_at_60_foon[_index]);
}

float min_equal_loudness_value(void)
{
  int i;
  float mi = MAX_SHORT;
  for (i = 0; i < NR_OF_BANDS; i++)
  {
     mi = fl_min(mi, equal_loudness_value(i));
  }
  return(mi);
}

void initialize_start_levels(float start_level[NR_OF_BANDS]) {
    for (int i = 0; i < NR_OF_BANDS; i++) {
        start_level[i] = INITIAL_LEVEL;
    }
}

void init_pan_and_level(float startLevel[NR_OF_BANDS], float left_pan[NR_OF_BANDS], float right_pan[NR_OF_BANDS], float left_level[NR_OF_BANDS], float right_level[NR_OF_BANDS]) {
    for (int i = 0; i < NR_OF_BANDS; i++) {
        if ((i % 2) != 0) {
            left_pan[i] = INITIAL_PAN + INITIAL_PAN_OFFSET;
            right_pan[i] = 0.0; 
        }
        else {
            left_pan[i] = 0.0;
            right_pan[i] = INITIAL_PAN + INITIAL_PAN_OFFSET;
        }
       float p = pow(10, startLevel[i] / 20);
       left_level[i] =  left_pan[i] * p;
       right_level[i] = right_pan[i] * p;
   }
}


void set_pan_and_level(float startLevel[NR_OF_BANDS], float left_pan[NR_OF_BANDS], float right_pan[NR_OF_BANDS], float left_level[NR_OF_BANDS], float right_level[NR_OF_BANDS], float width) {
    for (int i = 0; i < NR_OF_BANDS; i++) {
        if ((i % 2) != 0) {
            left_pan[i] = INITIAL_PAN + width * INITIAL_PAN_OFFSET;
            right_pan[i] = 0.0; 
        }
        else {
            left_pan[i] = 0.0;
            right_pan[i] = INITIAL_PAN + width * INITIAL_PAN_OFFSET;
        }
       float p = pow(10, startLevel[i] / 20);
       left_level[i] =  left_pan[i] * p;
       right_level[i] = right_pan[i] * p;
   }
}
