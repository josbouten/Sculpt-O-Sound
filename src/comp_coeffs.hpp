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

#ifndef _COMP_COEFFS_HPP
#define _COMP_COEFFS_HPP

#include "std.hpp"
#include <string.h>

void comp_release_times(float release_time[NR_OF_BANDS]);
void comp_attack_times(float attack_time[NR_OF_BANDS]);

void comp_attack_factors(float envelope_attack_factor[NR_OF_BANDS], float envelope_attack_time[NR_OF_BANDS]);
void comp_release_factors(float envelope_release_factor[NR_OF_BANDS], float envelope_release_time[NR_OF_BANDS]);

void init_release_times(float release_time[NR_OF_BANDS]);
void init_attack_times(float release_time[NR_OF_BANDS]);

void comp_coeffs_for_freq_and_bandwidth(float bandwidth, float center_frequency, double fsamp, float *alpha1, float *alpha2, float *beta);
void comp_coeffs_for_freqs(float lower_freq, float upper_freq, double fsamp, float *alpha1, float *alpha2, float *beta);
void comp_coeffs(int freqIndex, int freq[NR_OF_BANDS], float bandwidth, double fsamp, float *alpha1, float *alpha2, float *beta);
void print_coeffs(float alpha1, float alpha2, float beta);

void comp_all_coeffs(int freq[NR_OF_BANDS], float bandwidth, double fsamp, float alpha1[NR_OF_BANDS], float alpha2[NR_OF_BANDS], float beta[NR_OF_BANDS]);
void print_all_coeffs(float alpha1[], float alpha2[], float beta[]);

void print_times(const std::string s, float times[NR_OF_BANDS]);
void print_array(float elements[NR_OF_BANDS]);

void comp_attack_and_release_time_ranges(float min_attack_time[NR_OF_BANDS], float max_attack_time[NR_OF_BANDS], float min_release_time[NR_OF_BANDS], float max_release_time[NR_OF_BANDS]);

#endif
