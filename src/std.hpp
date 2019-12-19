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

#ifndef _STD_H
#define _STD_H

#define INITIAL_START_LEVEL 10.0

#define INITIAL_BW_IN_SEMITONES 4.0

#define INITIAL_CARR_BW_IN_SEMITONES 4.0
#define INITIAL_MOD_BW_IN_SEMITONES 4.0
#define INITIAL_GAIN 1.0

// ENVELOPE FOLLOWER

#define LOWER_ENVELOPE_ATTACK_TEMPERATURE 0.01
#define INITIAL_ENVELOPE_ATTACK_TEMPERATURE 1
#define UPPER_ENVELOPE_ATTACK_TEMPERATURE 100
#define LOWER_ENVELOPE_RELEASE_TEMPERATURE 0.01
#define INITIAL_ENVELOPE_RELEASE_TEMPERATURE 5
#define UPPER_ENVELOPE_RELEASE_TEMPERATURE 100
#define ENVELOPE_TEMPERATURE 10

#define MIN_ATTACK_TIME 0.0001
#define MAX_ATTACK_TIME 100.0000
#define ATTACK_TIME_STEPS 50
#define MIN_RELEASE_TIME 0.0001
#define MAX_RELEASE_TIME 100.0000
#define RELEASE_TIME_STEPS 50

// LEVELS

#define INITIAL_CARRIER_GAIN 1.0
#define MIN_CARRIER_GAIN 1.0
#define MAX_CARRIER_GAIN 10.0
#define INITIAL_MODULATOR_GAIN 1.0
#define MIN_MODULATOR_GAIN 1.0
#define MAX_MODULATOR_GAIN 10.0

#define MIN_LEVEL 1.0
#define MAX_LEVEL 24.0
#define INITIAL_LEVEL 1.0
#define LEVEL_STEPS 50

// PANNING

#define MIN_PAN         -0.999 // Right
#define MAX_PAN          0.999 // Left
#define PAN_STEPS 50

#define CENTER_PAN 0.0
#define INITIAL_PAN   CENTER_PAN
#define INITIAL_PAN_OFFSET ((MAX_PAN - MIN_PAN) / 5)

#define UPPER_SMOOTHING_BAND 20
#define SMOOTHING_FACTOR 10

// MISC

#define PI 3.1415828
#define HALF_PI PI / 2.0
#define QUATER_PI PI / 4.0
#define FFSAMP 44100.0
#define NR_OF_BANDS 31

// LOGIC
#define FALSE 0
#define TRUE 1

// NUMBERS

#define MAX_SHORT 32767

float fl_max(float a, float b);
float fl_min(float a, float b);

#define PRESSES 1
#define NOT_PRESSED 0

#endif
