/*
Copyright (C) 2018, Jos Bouten aka Zaphod B.

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
   for (int i = 0; i < NR_OF_BANDS; i++) {
     double omega_1 = 2.0 * 3.14159 * (double) freq[i] / fsamp;
     double omega_2 = 2.0 * 3.14159 * (double) freq[i + 1] / fsamp;
     double delta_omega = (omega_2 - omega_1) * ((double) bandwidth / 4.0);
     double omega_c = (omega_1 + omega_2) / 2.0;
     double numerator   = 1.0 - tan((double)(delta_omega / 2.0));
     double denominator = 1.0 + tan((double)(delta_omega / 2.0));

     beta[i] =  (float) (numerator / denominator);
     float gamma = (float) (-cos(omega_c));
     //
     alpha1[i] = 0.5 - 0.5 * beta[i];
     alpha2[i] = gamma * (1 + beta[i]);
     alpha2[i] /= alpha1[i];
     beta[i] /= alpha1[i];
#ifdef DEBUGMSG
     printf("fsamp: %f\n", fsamp);
     printf("bandwidth: %f\n", bandwidth);
     printf("omega_1: %f\n", omega_1);
     printf("omega_2: %f\n", omega_2);
     printf("denominator: %f\n", denominator);
     printf("comp_all_coeffs 2: %d %d %f %f %f\n", freq[i], freq[i + 1], alpha1[i], alpha2[i], beta[i]);
#endif 
  }
}

void print_coeffs(float alpha1, float alpha2, float beta) {
    printf("%f %f %f\n", alpha1, alpha2, beta);
}

void print_all_coeffs(float alpha1[], float alpha2[], float beta[]) {
   for (int i = 0; i < NR_OF_BANDS; i++) {
      print_coeffs(alpha1[i], alpha2[i], beta[i]);
  }
}

void comp_release_times(float release_time[NR_OF_BANDS])
{ // Compute release times for the envelope followers for the respective frequency bands.
  int i;
  double f_c;
  for (i = 0; i < NR_OF_BANDS; i++)
  {
    f_c = ((float) freq[i+1] + (float) freq[i]) / 2.0;
    // Make bands above band UPPER_SMOOTHING_BAND sound less harsh.
    if (i > UPPER_SMOOTHING_BAND)
      release_time[i] = SMOOTHING_FACTOR * (2 * PI / f_c) * INITIAL_ENVELOPE_RELEASE_TEMPERATURE;
    else
      release_time[i] = (2 * PI / f_c) * INITIAL_ENVELOPE_RELEASE_TEMPERATURE;
  }
}

void print_array(float times[NR_OF_BANDS]) {
  for (int i = 0; i < NR_OF_BANDS; i++) {
    printf("%f ", times[i]);
  }
  printf("\n");
}

void comp_attack_times(float attack_time[NR_OF_BANDS])
{ // Compute attack times for the envelope followers for the respective frequency bands.
  int i;
  double f_c;
  for (i = 0; i < NR_OF_BANDS; i++)
  {
    f_c = ((float) freq[i+1] + (float) freq[i]) / 2.0;
    attack_time[i]  = (2 * PI / f_c) * INITIAL_ENVELOPE_ATTACK_TEMPERATURE;
  }
}

void comp_attack_factors(float envelope_attack_factor[NR_OF_BANDS], float envelope_attack_time[NR_OF_BANDS])
{
  int i;
  for (i = 0; i < NR_OF_BANDS; i++)
  {
    envelope_attack_factor[i]  = exp(-1.0 / (FFSAMP * envelope_attack_time[i]));
  }
}

void comp_release_factors(float envelope_release_factor[NR_OF_BANDS], float envelope_release_time[NR_OF_BANDS])
{     
  int i;
  for (i = 0; i < NR_OF_BANDS; i++)
  {   
    envelope_release_factor[i]  = exp(-1.0 / (FFSAMP * envelope_release_time[i]));
  }            
}  
