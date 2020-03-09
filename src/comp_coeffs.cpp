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
#include <stdio.h>

int freq[] = {0, 20, 25, 32, 40, 50, 63, 80, 100, 125, 160, 200, 250,
              315, 400, 500, 630, 800, 1000, 1250, 1600, 2000, 2500,
              3150, 4000, 5000, 6300, 8000, 10000,
              12500, 16000, 20000, 22025};

void comp_all_coeffs(int freq[NR_OF_BANDS], float bandwidth, double fsamp, float alpha1[NR_OF_BANDS], float alpha2[NR_OF_BANDS], float beta[NR_OF_BANDS]) {
   //
   // Compute all filter coefficients for NR_OF_BANDS frequency bands for a given bandwidth and sampling frequency.
   //
   for (int bandNr = 0; bandNr < NR_OF_BANDS; bandNr++) {
     double omega_1 = 2.0 * PI * (double) freq[bandNr] / fsamp;
     double omega_2 = 2.0 * PI * (double) freq[bandNr + 1] / fsamp;
     double delta_omega = (omega_2 - omega_1) * ((double) bandwidth / 4.0);
     double omega_c = (omega_1 + omega_2) / 2.0;
     double numerator   = 1.0 - tan((double)(delta_omega / 2.0));
     double denominator = 1.0 + tan((double)(delta_omega / 2.0));

     beta[bandNr] =  (float) (numerator / denominator);
     float gamma = (float) (-cos(omega_c));
     //
     alpha1[bandNr] = 0.5 - 0.5 * beta[bandNr];
     alpha2[bandNr] = gamma * (1 + beta[bandNr]);
     alpha2[bandNr] /= alpha1[bandNr];
     beta[bandNr] /= alpha1[bandNr];
#ifdef DEBUGMSG
     printf("fsamp: %f\n", fsamp);
     printf("bandwidth: %f\n", bandwidth);
     printf("omega_1: %f\n", omega_1);
     printf("omega_2: %f\n", omega_2);
     printf("denominator: %f\n", denominator);
     printf("comp_all_coeffs 2: %d %d %f %f %f\n", freq[bandNr], freq[bandNr + 1], alpha1[bandNr], alpha2[bandNr], beta[bandNr]);
#endif 
  }
}

void print_coeffs(float alpha1, float alpha2, float beta) {
    printf("%f %f %f\n", alpha1, alpha2, beta);
}

void print_all_coeffs(float alpha1[], float alpha2[], float beta[]) {
   for (int bandNr = 0; bandNr < NR_OF_BANDS; bandNr++) {
      print_coeffs(alpha1[bandNr], alpha2[bandNr], beta[bandNr]);
  }
}

void comp_times(float time[NR_OF_BANDS], float initial_envelope_temperature)
{ // Compute release times for the envelope followers for the respective frequency bands.
  for (int bandNr = 0; bandNr < NR_OF_BANDS; bandNr++) {
    float f_c = ((float) freq[bandNr + 1] + (float) freq[bandNr]) / 2.0;
    time[bandNr] = ((2.0 * PI) / f_c) * initial_envelope_temperature;
  }
}

void comp_release_times(float release_time[NR_OF_BANDS]) {
    comp_times(release_time, LOWER_ENVELOPE_RELEASE_TEMPERATURE);
}

void comp_attack_times(float attack_time[NR_OF_BANDS]) {
    comp_times(attack_time, LOWER_ENVELOPE_ATTACK_TEMPERATURE);
}

float init_time(int bandNr) {
  float f_c = ((float) freq[bandNr + 1] + (float) freq[bandNr]) / 2.0;
  return((2.0 * PI) / f_c);
}

float init_release_time(int bandNr) {
    return(init_time(bandNr) * INITIAL_ENVELOPE_RELEASE_TEMPERATURE);
}

void init_release_times(float release_time[NR_OF_BANDS]) 
{
  for (int bandNr = 0; bandNr < NR_OF_BANDS; bandNr++) {
      release_time[bandNr] = init_release_time(bandNr);
  }
}

float init_attack_time(int bandNr) {
    return(init_time(bandNr) * INITIAL_ENVELOPE_ATTACK_TEMPERATURE);
}

void init_attack_times(float attack_time[NR_OF_BANDS])
{
  for (int bandNr = 0; bandNr < NR_OF_BANDS; bandNr++) {
    attack_time[bandNr] = init_attack_time(bandNr);
   }
}

void print_array(float times[NR_OF_BANDS]) {
  for (int bandNr = 0; bandNr < NR_OF_BANDS; bandNr++) {
    printf("%f ", times[bandNr]);
  }
  printf("\n");
}

float comp_factor(float time) {
    return(exp(-1.0 / (FFSAMP * time)));
}

float comp_attack_factor(float envelope_time) {
    return(comp_factor(envelope_time));
}

void comp_attack_factors(float envelope_attack_factor[NR_OF_BANDS], float envelope_attack_time[NR_OF_BANDS])
{
  for (int bandNr = 0; bandNr < NR_OF_BANDS; bandNr++) {
    envelope_attack_factor[bandNr] = comp_attack_factor(envelope_attack_time[bandNr]);
  }
}

float comp_release_factor(float envelope_release_time) {
    return(comp_factor(envelope_release_time));
}

void comp_release_factors(float envelope_release_factor[NR_OF_BANDS], float envelope_release_time[NR_OF_BANDS])
{
  for (int bandNr = 0; bandNr < NR_OF_BANDS; bandNr++) {
    envelope_release_factor[bandNr]  = comp_release_factor(envelope_release_time[bandNr]);
  }
}

void comp_time_ranges(float time_lower_range[NR_OF_BANDS], float time_upper_range[NR_OF_BANDS], float lower_temperature, float upper_temperature)
{
  for (int bandNr = 0; bandNr < NR_OF_BANDS; bandNr++) {
    float f_c = ((float) freq[bandNr + 1] + (float) freq[bandNr]) / 2.0;
    time_lower_range[bandNr]  = ((2.0 * PI) / f_c) * lower_temperature;
    time_upper_range[bandNr]  = ((2.0 * PI) / f_c) * upper_temperature;
  }
}

void comp_attack_time_ranges(float attack_time_lower_range[NR_OF_BANDS], float attack_time_upper_range[NR_OF_BANDS]) {
  comp_time_ranges(attack_time_lower_range, attack_time_upper_range, LOWER_ENVELOPE_ATTACK_TEMPERATURE, UPPER_ENVELOPE_ATTACK_TEMPERATURE);
}

void comp_release_time_ranges(float release_time_lower_range[NR_OF_BANDS], float release_time_upper_range[NR_OF_BANDS]) {
  comp_time_ranges(release_time_lower_range, release_time_upper_range, LOWER_ENVELOPE_RELEASE_TEMPERATURE, UPPER_ENVELOPE_RELEASE_TEMPERATURE);
}

void comp_attack_and_release_time_ranges(float attack_time_lower_range[NR_OF_BANDS], float attack_time_upper_range[NR_OF_BANDS],
        float release_time_lower_range[NR_OF_BANDS], float release_time_upper_range[NR_OF_BANDS])
{
  comp_attack_time_ranges(attack_time_lower_range, attack_time_upper_range);
  comp_release_time_ranges(release_time_lower_range, release_time_upper_range);
}
