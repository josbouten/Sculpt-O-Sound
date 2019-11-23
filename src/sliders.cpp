#include "std.hpp"
#include <stdio.h>
 
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
  float mi = MAX_SHORT;
  for (int i = 0; i < NR_OF_BANDS; i++)
  {
     mi = fl_min(mi, equal_loudness_value(i));
  }
  return(mi);
}

float max_equal_loudness_value(void)
{
  float ma = 0;
  for (int i = 0; i < NR_OF_BANDS; i++)
  {
     ma = fl_max(ma, equal_loudness_value(i));
  }
  return(ma);
}

void initialize_slider_levels(float start_level[NR_OF_BANDS]) 
{
  float minimum_equal_loudness_level = min_equal_loudness_value();
  for (int i = 0; i < NR_OF_BANDS; i++) { 
    //start_level[i] = equal_loudness_value(i);
    start_level[i] = INITIAL_START_LEVEL * equal_loudness_value(i) / minimum_equal_loudness_level;
    printf("start_level[%d] = %f\n", i, start_level[i]);
  }
}
