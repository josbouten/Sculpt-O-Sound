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
