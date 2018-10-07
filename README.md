# VCV vocoder plugin
# Copyright: Zaphod B. 2018

![alt text](res/pic_logo_sos.gif)

Sculpt-O-Sound presents: 

Vocode-O-Matic, v0.2 is a 31 terts band vocoder with editable frequency matrix usable for 44100 Hz sampling frequency only (at the moment).

The vocoder has 31 terts band filters for the carrier and modulator inputs.
The carrier input is the top left of the connectors, the modulator the top right.
The L and R outputs are immediately below the inputs.
To get an interesting result, start by using a pad like sound with a big bandwidth as a carrier and
a rhythm loop as a modulator. You should hear the pad play the rhythm.

Example

Try for example the files fl1_std.wav (string from a Kurzweil K2000) and jung05.wav (a jungle loop from some sample kit).
You can play them via a sample player like complex simpler from the NYSTHI plugin set.
For your convenience Vocode-O-Matic-example_01.vcv will load the appropriate files.
Make sure the top simpler loads the fl2.wav file and the bottom one the jung05.wav file.
Have a look at the screen shot below if you need some help.

For reasons unclear to me when using the example setup file often the same file is read into both simplers which does not make much sense. As a remedy read in the audio files explicitely and you'll be fine.

![alt text](res/Screenshot_2018-10-04_at_19.52.28.png)

Knobs and Toggle switches

The knobs on top of the rack element are attenuators for the carrier and modulator input signals.
The filter matrix consists of toggle switches, so you can make your own coupling between modulator and carrier filters.
Mind you, no attenuation is done, so if you combine a lot of modulator bands with one carrier filter, the output of the filter
may exceed the maximum sample value (voltage) allowed resulting in distortion. 

Buttons

The top toggle button when pressed will turn red, this will put the vocode in bypass mode. 
This will send the modulator and carrier input signals to the left and right outputs respectively.

The second toggle button from the top lets you choose between 5 filter mappings. Number 4 is a linear mapping of the modulator filters
against the carrier filters. This is the default at startup. Number 5 is a reverse mapping. 
Mapping 0, 1, 2, 3 and log mappings. All sound differently. And by toggling switches in the matrix you can change them.

The third button does some matrix magic. It will shift the filter mapping to the right in a slow pace. 
If you press the button again the matrix will stop moving.

Known bugs

Since I'm relatively new to C++ programming and the Rack VCV platform I probably have not initialized all elements properly.
What I've found is that the vocoder on occasion will only output a DC value, and not produce any other sound at all.
Restarting the Rack or opening the module once more will often solve this problem. If anyone knows how to handle this properly,
please let me know.

All filters are based on the series of tutorial papers about Effect Design by Jon Dattorro published in the Journal of the Audio Engineering Society.
