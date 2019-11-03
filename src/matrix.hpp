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

#define PRESSED 1
#define NOT_PRESSED 0

#define NR_MATRIX_MODES 6 // linear, inverse, 4 * log
#define INITIAL_MATRIX_MODE 4

void initialize_start_levels(float start_level[NR_OF_BANDS]);

void initialize_matrix(int button_value[NR_OF_BANDS][NR_OF_BANDS], int p_cnt[NR_OF_BANDS]);

void clear_matrix(int button_value[NR_OF_BANDS][NR_OF_BANDS], int p_cnt[NR_OF_BANDS]);

void print_matrix(int button_value[NR_OF_BANDS][NR_OF_BANDS]);
void print_matrix(int button_value[NR_OF_BANDS][NR_OF_BANDS], int p_cnt[NR_OF_BANDS]);

void print_p_cnt(int p_cnt[NR_OF_BANDS]);

void choose_matrix(int filter_coupling_type, int button_value[NR_OF_BANDS][NR_OF_BANDS], int p_cnt[NR_OF_BANDS]);

void matrix_shift_buttons_right(int button_value[NR_OF_BANDS][NR_OF_BANDS], int p_cnt[NR_OF_BANDS]);

void matrix_shift_buttons_left(int button_value[NR_OF_BANDS][NR_OF_BANDS], int p_cnt[NR_OF_BANDS]);

void matrix_shift_buttons_up(int button_value[NR_OF_BANDS][NR_OF_BANDS], int p_cnt[NR_OF_BANDS]);

void matrix_shift_buttons_down(int button_value[NR_OF_BANDS][NR_OF_BANDS], int p_cnt[NR_OF_BANDS]);
