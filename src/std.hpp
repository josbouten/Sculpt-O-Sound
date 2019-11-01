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

#ifndef _STD_H
#define _STD_H

#define INITIAL_START_LEVEL 10.0

#define INITIAL_BW_IN_SEMITONES 4.0

#define INITIAL_CARR_BW_IN_SEMITONES 4.0
#define INITIAL_MOD_BW_IN_SEMITONES 4.0
#define INITIAL_GAIN 1.0

#define LOWER_ENVELOPE_ATTACK_TEMPERATURE 0.01
#define INITIAL_ENVELOPE_ATTACK_TEMPERATURE 1
#define UPPER_ENVELOPE_ATTACK_TEMPERATURE 100
#define LOWER_ENVELOPE_RELEASE_TEMPERATURE 0.01
#define INITIAL_ENVELOPE_RELEASE_TEMPERATURE 5
#define UPPER_ENVELOPE_RELEASE_TEMPERATURE 100
#define ENVELOPE_TEMPERATURE 10

#define MIN_PAN         -0.999 // Right
#define MAX_PAN          0.999 // Left

#define CENTER_PAN_VALUE 0.0
#define INITIAL_PAN   CENTER_PAN_VALUE
#define INITIAL_PAN_OFFSET ((MAX_PAN - MIN_PAN) / 5)

#define UPPER_SMOOTHING_BAND 20
#define SMOOTHING_FACTOR 10
#define PI 3.1415828
#define FFSAMP 44100.0
#define NR_OF_BANDS 31
#define FALSE 0
#define TRUE 1

#endif
