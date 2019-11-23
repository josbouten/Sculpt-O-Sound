/*
This is Vocode-O-Matic, a vocoder plugin for VCV Rack v1.x
Copyright (C) 2018, Jos Bouten aka Zaphod B.
You can contact me here: josbouten at gmail dot com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "std.hpp"
#include <math.h>

// Constant Power Panning
// Literature: https://www.cs.cmu.edu/~music/icm-online/readings/panlaws

float left_pan_factor(float slider_value) {
    // If slider value < 0 then use cosine else use sine value of angle.
    float angle = (slider_value - MIN_PAN) * 0.5 * HALF_PI;
    return(cos(angle));
}

float right_pan_factor(float slider_value) {
    // If slider value < 0 then use cosine else use sine value of angle.
    float angle = (slider_value - MIN_PAN) * 0.5 * HALF_PI;
    return(sin(angle));
}

void init_pan_and_level(float slider_level[NR_OF_BANDS], float left_pan[NR_OF_BANDS], float right_pan[NR_OF_BANDS], float left_level[NR_OF_BANDS], float right_level[NR_OF_BANDS]) {
    for (int i = 0; i < NR_OF_BANDS; i++) {
        if ((i % 2) != 0) {
            left_pan[i] = left_pan_factor(INITIAL_PAN + INITIAL_PAN_OFFSET);
            right_pan[i] = right_pan_factor(0.0);
        }
        else {
            left_pan[i] = left_pan_factor(0.0);
            right_pan[i] = right_pan_factor(INITIAL_PAN + INITIAL_PAN_OFFSET);
        }
       float p = pow(10, slider_level[i] / 20);
       left_level[i] =  left_pan[i] * p;
       right_level[i] = right_pan[i] * p;
#ifdef DEBUGMSG
        printf("pan_and_level: %f %f %f %f\n", left_pan[i], right_pan[i], left_level[i], right_level[i]);
#endif
   }
}

void set_pan_and_level(float slider_level[NR_OF_BANDS], float slider_pan[NR_OF_BANDS], float left_pan[NR_OF_BANDS], float right_pan[NR_OF_BANDS], float left_level[NR_OF_BANDS], float right_level[NR_OF_BANDS], float width) {
    for (int i = 0; i < NR_OF_BANDS; i++) {
       float p = pow(10, slider_level[i] / 20);
       left_pan[i] = left_pan_factor(slider_pan[i]) * width;
       right_pan[i] = right_pan_factor(slider_pan[i]) * width;
       left_level[i] =  left_pan[i] * p;
       right_level[i] = right_pan[i] * p;
#ifdef DEBUGMSG
        printf("pan_and_level: %f %f %f %f\n", left_pan[i], right_pan[i], left_level[i], right_level[i]);
#endif
   }
}
