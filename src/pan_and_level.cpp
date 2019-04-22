#include "std.hpp"
#include <math.h>

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
