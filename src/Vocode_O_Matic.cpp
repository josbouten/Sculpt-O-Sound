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

#include "Sculpt-O-Sound.hpp"
#include "std.hpp"
#include "matrix.hpp"
#include "Vocode_O_Matic.hpp"
#include "pan_and_level.hpp"

//#define PANNING

// Dimensions of matrix of buttons.
#define LED_WIDTH 10
#define LED_HEIGHT 10

#define VBASE NR_OF_BANDS * LED_HEIGHT + 40

#define ORIGIN_BOTTOM_LEFT

#define HBASE 140

void Vocode_O_Matic::onReset() {
  matrix_mode_button_pressed = false;
  matrix_hold_button_pressed = false;
  matrix_one_step_right_button_pressed = false;
  matrix_one_step_left_button_pressed = false;
  matrix_mode = INITIAL_MATRIX_MODE;
  matrix_shift_position = 0;

  // Initialize linear filter coupling.
  choose_matrix(4, button_value, p_cnt); // Initialize linear filter coupling.
  // Refresh LED matrix.
  refresh_led_matrix(lights_offset, p_cnt, button_value, led_state);

  // Set mute output buttons to not mute.
  for (int i = 0; i < NR_OF_BANDS; i++) {
    mute_modulator[i] = false;
    params[MUTE_MODULATOR_PARAM + i].setValue(0.0);
    lights[MUTE_MODULATOR_LIGHT + i].setBrightness(1.0);
  }
  // Show mute led values.
  refresh_mute_modulator_leds(MUTE_MODULATOR_LIGHT, mute_modulator);

  blinkPhase = -1.0f;
  oneStepBlinkPhase = 0.0f;

  // Reset toggle lights.
  lights[MATRIX_HOLD_TOGGLE_LIGHT].setBrightness(0.0);
  lights[BYPASS_LIGHT].setBrightness(0.0);

  // Set gain to initial value.
  //params[CARRIER_GAIN_PARAM].value = INITIAL_CARRIER_GAIN;
  //params[MODULATOR_GAIN_PARAM].value = INITIAL_MODULATOR_GAIN;
}

void Vocode_O_Matic::onRandomize() {
  int cnt = 3;
  clear_matrix(button_value, p_cnt);
  for (int i = 0; i < NR_OF_BANDS; i++) {
      for (int j = 0; j < cnt; j++) {
          int jj = i / 2 + (int) ((float) NR_OF_BANDS / 2.0 * rand() / (RAND_MAX + 1.0));
          button_value[i][p_cnt[i]++] = jj;
      }
  }
  refresh_led_matrix(lights_offset, p_cnt, button_value, led_state);

  // Set mute output buttons to not mute.
  for (int i = 0; i < NR_OF_BANDS; i++) {
    if ((rand() / (RAND_MAX + 1.0)) > 0.5) {
        mute_modulator[i] = false;
        lights[MUTE_MODULATOR_LIGHT + i].setBrightness(1.0);
    } else {
        mute_modulator[i] = true;
        lights[MUTE_MODULATOR_LIGHT + i].setBrightness(0.0);
    }
  }

  // Show mute led values.
  refresh_mute_modulator_leds(MUTE_MODULATOR_LIGHT, mute_modulator);

  // Set gain to initial value.
  params[CARRIER_GAIN_PARAM].setValue(INITIAL_CARRIER_GAIN);
  params[MODULATOR_GAIN_PARAM].setValue(INITIAL_MODULATOR_GAIN);
}

void Vocode_O_Matic::process(const ProcessArgs &args) {
  int x, y;
  float rc;
  static int x1 = -1, x2 = -1, y1,  y2;
  static int only_once = 1;
  if (only_once == 1) {
    init_attack_times(envelope_attack_time);
    init_release_times(envelope_release_time);
    comp_attack_factors(envelope_attack_factor, envelope_attack_time);
    comp_release_factors(envelope_release_factor, envelope_release_time);
    only_once = 0;
#ifdef DEBUGMSG
    printf("attack_time    : "); print_array(envelope_attack_time);
    printf("release_time   : "); print_array(envelope_release_time);
    printf("attack_factor  : "); print_array(envelope_attack_factor);
    printf("release_factor : "); print_array(envelope_release_factor);
    printf("left_pan       : "); print_array(left_pan);
    printf("right_pan      : "); print_array(right_pan);
    printf("left_level     : "); print_array(left_level);
    printf("right_level    : "); print_array(right_level);
    printf("mod_alpha1     : "); print_array(mod_alpha1);
    printf("mod_alpha2     : "); print_array(mod_alpha2);
    printf("mod_beta       : "); print_array(mod_beta);
    printf("carr_alpha1    : "); print_array(carr_alpha1);
    printf("carr_alpha2    : "); print_array(carr_alpha2);
    printf("carr_beta      : "); print_array(carr_beta);
#endif
  }
  // Do da vocoding thang.
  float deltaTime = args.sampleTime;
  float oneStepDeltaTime = APP->engine->getSampleTime();
#ifdef DEBUGMSG
  bool refresh = false;
#endif

  xc[0] = inputs[CARR_INPUT].getVoltage() * params[CARRIER_GAIN_PARAM].getValue();
  xm[0] = inputs[MOD_INPUT].getVoltage() * params[MODULATOR_GAIN_PARAM].getValue();
  float smoothing_factor = 1.0;

  for (int i = 0; i < NR_OF_BANDS; i++) {
    //
    // CARRIER
    //
    // Direct Form I topology is used for filtering. Direct Form II would in this case cost as many multiplications and shifts.
    yc[i][0] = carr_alpha1[i] * (xc[0] - xc[2] - carr_alpha2[i] * yc[i][1] - carr_beta[i] * yc[i][2]);
    //
    // Shift all carrier filter taps.
    yc[i][2] = yc[i][1]; yc[i][1] = yc[i][0];

    //
    // MODULATOR
    //
    ym[i][0] = mod_alpha1[i] * (xm[0] - xm[2] - mod_alpha2[i] * ym[i][1] - mod_beta[i] * ym[i][2]);
    //
    // Shift filter taps.
    ym[i][2] = ym[i][1]; ym[i][1] = ym[i][0];

    //
    // Compute input for envelope for this band.
    // Use only positive values so that energy levels are zero or positive.
    xm_env = fabs(ym[i][0]);
    //
    // Perform AR averager on this band's output.
    if (ym_env[i][1] < xm_env)
        smoothing_factor = envelope_attack_factor[i];
    else
        smoothing_factor = envelope_release_factor[i];
    ym_env[i][0] = (1.0 - smoothing_factor) * xm_env + smoothing_factor * ym_env[i][1];
    //
    // Shift low pass filter taps.
    ym_env[i][1] = ym_env[i][0];
  }

  // Shift modulator input taps.
  xm[2] = xm[1]; xm[1] = xm[0];
  //
  // Shift carrier input taps.
  xc[2] = xc[1]; xc[1] = xc[0];

  // Blink light at 2Hz.
  blinkPhase += deltaTime;
  if (blinkPhase >= 1.0f)
    blinkPhase -= 1.0f;
  oneStepBlinkPhase += oneStepDeltaTime;
  if (oneStepBlinkPhase >= 0.1f) { // light will be on for a very short time.
    lights[MATRIX_ONE_STEP_RIGHT_LIGHT].setBrightness(0.0f);
    lights[MATRIX_ONE_STEP_LEFT_LIGHT].setBrightness(0.0f);
  }

  // Process trigger signal on matrix shift input.
  float shiftRightTriggerIn = inputs[SHIFT_RIGHT_INPUT].getVoltage();
  cv_right->update(shiftRightTriggerIn);
  float shiftLeftTriggerIn = inputs[SHIFT_LEFT_INPUT].getVoltage();
  cv_left->update(shiftLeftTriggerIn);

  // Handling of buttons and / or lights.
  // If bypass button was pressed:
  if (bypass_button_trig.process(params[BYPASS_SWITCH].getValue())) {
    fx_bypass = !fx_bypass;
  }
  lights[BYPASS_LIGHT].setBrightness(fx_bypass ? 1.00 : 0.0);
  if (matrix_hold_button_pressed) { // We blink only if the button is toggled in the on position.
    lights[MATRIX_HOLD_TOGGLE_LIGHT].setBrightness((blinkPhase < 0.5f) ? 1.0f : 0.0f);
  }

  // Hold matrix movement if toggle is pressed.
  if (matrix_hold_button_trig.process(params[MATRIX_HOLD_TOGGLE_PARAM].getValue())) {
    matrix_hold_button_pressed = !matrix_hold_button_pressed;
    lights[MATRIX_HOLD_TOGGLE_LIGHT].setBrightness(matrix_hold_button_pressed ? 1.00 : 0.0);
  }

  // If one step right button was pressed:
  if (matrix_one_step_right_button_trig.process(params[MATRIX_ONE_STEP_RIGHT_PARAM].getValue())) {
    // Start a new blink period.
    oneStepBlinkPhase = 0.0f;
    // Light up the button;
    lights[MATRIX_ONE_STEP_RIGHT_LIGHT].setBrightness(1.00);
    // Shift the buttons one step.
    shift_buttons_right(button_value, p_cnt, led_state, &matrix_shift_position);
  }

  // Shift the matrix if there is a new trigger on the shift right input and the
  // hold button is not pressed and we are not in bypass mode.
  if (not fx_bypass && cv_right->newTrigger() and not matrix_hold_button_pressed) {
    // Shift the buttons one step.
    shift_buttons_right(button_value, p_cnt, led_state, &matrix_shift_position);
  }

  // If one step left button was pressed:
  if (matrix_one_step_left_button_trig.process(params[MATRIX_ONE_STEP_LEFT_PARAM].getValue())) {
    // Start a new blink period.
    oneStepBlinkPhase = 0.0f;
    // Light up the button;
    lights[MATRIX_ONE_STEP_LEFT_LIGHT].setBrightness(1.00);
    // Shift the buttons one step.
    shift_buttons_left(button_value, p_cnt, led_state, &matrix_shift_position);
  }

  // Shift the matrix if there is a new trigger on the shift left input and the
  // hold button is not pressed and we are not in bypass mode.
  if (not fx_bypass && cv_left->newTrigger() and not matrix_hold_button_pressed) {
    // Shift the buttons one step.
    shift_buttons_left(button_value, p_cnt, led_state, &matrix_shift_position);
  }

  // If toggle matrix preset button was pressed.
  if (matrix_mode_button_trig.process(params[MATRIX_MODE_TOGGLE_PARAM].getValue())) {
    matrix_mode_button_pressed = false;
    lights[MATRIX_MODE_TOGGLE_PARAM].setBrightness(matrix_mode_button_pressed ? 1.00 : 0.0);
    matrix_mode++;
    if (matrix_mode > NR_MATRIX_MODES - 1) { matrix_mode = 0; }
    choose_matrix(matrix_mode, button_value, p_cnt);

    // Refresh LED matrix.
    refresh_led_matrix(lights_offset, p_cnt, button_value, led_state);

    // Restart the shift counter at 0.
    matrix_shift_position = 0;
  }

  // Panning
#ifdef PANNING
  width = params[PANNING_PARAM].getValue();
  if (width != width_old) {
    set_pan_and_level(slider_level, slider_pan, left_pan, right_pan, left_level, right_level, width);
    width_old = width;
  }
#endif
  if (button_left_clicked_val > 0) {
    if (button_left_clicked_val >= MOD_MATRIX_PARAM && button_left_clicked_val < MOD_MATRIX_PARAM + NR_OF_BANDS * NR_OF_BANDS) { // There was a left click in the button matrix.
      int buttonNr = button_left_clicked_val - MOD_MATRIX_PARAM;
      led_state[buttonNr] = !led_state[buttonNr];
      int index = MOD_MATRIX_LIGHT + buttonNr;
      lights[index].setBrightness(1.0f - lights[index].getBrightness());
      int chosen_row = buttonNr / NR_OF_BANDS;
      int chosen_col = buttonNr % NR_OF_BANDS;
      if ((p_cnt[chosen_row] > 0) && (!led_state[buttonNr])) {
        for (int col = 0; col < NR_OF_BANDS; col++) { // Find the right column.
          if (button_value[chosen_row][col] == chosen_col) {
            button_value[chosen_row][col] = NOT_PRESSED;
             // Shift all values from unpressed button 1 position to the left.
            for (int shift_col = col; shift_col < p_cnt[chosen_row]; shift_col++) {
              button_value[chosen_row][shift_col] = button_value[chosen_row][shift_col + 1];
            }
            p_cnt[chosen_row]--;
            break; // As soon as one button is unpressed we are done.
          }
        }
      }
      else {
          button_value[chosen_row][p_cnt[chosen_row]] = chosen_col;
          p_cnt[chosen_row]++;
      }
    } else { // A mute button was left clicked.
      int buttonNr = button_left_clicked_val - MUTE_MODULATOR_PARAM;
      mute_modulator[buttonNr] = !mute_modulator[buttonNr];
      lights[MUTE_MODULATOR_LIGHT + buttonNr].setBrightness(1.0 - lights[MUTE_MODULATOR_LIGHT + buttonNr].getBrightness());
#ifdef DEBUGMSG
      refresh = true;
#endif
    }
    button_left_clicked_val = 0;
  }
  if (button_right_clicked_val > 0)
  {
    // Right button was clicked in the button matrix.
    if (button_right_clicked_val >= MOD_MATRIX_PARAM && button_right_clicked_val < MOD_MATRIX_PARAM + NR_OF_BANDS * NR_OF_BANDS)
    { // There was a richt click in the button matrix.
        right_click_state = !right_click_state;
        int buttonNr = button_right_clicked_val - MOD_MATRIX_PARAM;
        x = buttonNr / NR_OF_BANDS;
        y = buttonNr - x * NR_OF_BANDS;
        lights[MOD_MATRIX_LIGHT + buttonNr].value = true;
        handle_single_button(x, y, 1.0);
        if (x1 >= 0) { x2 = x; y2 = y; }
        else { x1 = x; y1 = y; }
        if ((x1 >= 0) && (x2 >= 0))
        {
            if (x1 != x2) { rc = ((float) y2 - (float) y1 ) / ((float) x2 - (float) x1); }
            else rc = -1.0;
            if (x1 == x2) // Vertical Path
            {
                if (y1 > y2) std::swap(y1, y2);
                for (y = y1 + 1; y < y2; y++) {
                    handle_single_button(x1, y, 1.0);
                }
            }
            else
            {
                if (x1 > x2) { std::swap(x1, x2); std::swap(y1, y2); }
                for (x = x1 + 1; x < x2; x++)
                {
                    y = (int) (y1 + rc * ((float) (x - x1)));
                    if (y >= NR_OF_BANDS) {
                        fprintf(stderr, "buttons_right_mouse::y-value too big: %d\n", y);
                        y = NR_OF_BANDS - 1;
                    }
                    handle_single_button(x, y, 1.0);
                }
            }
            x1 = x2 = -1;
        }
        refresh_led_matrix(lights_offset, p_cnt, button_value, led_state);
    }
    else
    {   // A mute button was right clicked.
        right_click_state = !right_click_state;
        int buttonNr = button_right_clicked_val - MUTE_MODULATOR_PARAM;
        if (right_click_state) { // This is solo mode, so we mute everything except the clicked button.
            for (int i = 0; i < NR_OF_BANDS; i++) {
                mute_modulator_old[i] = mute_modulator[i];
                lights[MUTE_MODULATOR_LIGHT + i].setBrightness(0.0);
                mute_modulator[i] = true;
            }
            mute_modulator[buttonNr] = false;
            lights[MUTE_MODULATOR_LIGHT + buttonNr].setBrightness(1.0);
        } else { // This is the unmute state, so we restore the pre mute knob settings.
            for (int i = 0; i < NR_OF_BANDS; i++) {
                mute_modulator[i] = mute_modulator_old[i];
                lights[MUTE_MODULATOR_LIGHT + i].setBrightness(mute_modulator[i] == true ? 0.0: 1.0);
            }
        }
    }
    button_right_clicked_val = 0;
  }
#ifdef DEBUGMSG
  if (refresh) {
      print_mute_buttons(mute_modulator);
      refresh_mute_modulator_leds(MUTE_MODULATOR_LIGHT, mute_modulator);
      refresh = false;
  }
#endif
  // Here we compute the output sample amplitude for both channels.
  if (fx_bypass) {
    outputs[LEFT_OUTPUT].setVoltage(inputs[CARR_INPUT].getVoltage() * params[CARRIER_GAIN_PARAM].getValue());
    outputs[RIGHT_OUTPUT].setVoltage(inputs[MOD_INPUT].getVoltage() * params[MODULATOR_GAIN_PARAM].getValue());
  }
  else
  {
    // Initialize output signal.
    outputs[RIGHT_OUTPUT].setVoltage(0.0);
    outputs[LEFT_OUTPUT].setVoltage(0.0);

    // The output is the sum of all carrier band signals multiplied by all
    // envelope outputs for that band (unless a carrier band is muted).
    for (int i = NR_OF_BANDS -1; i >= 0; i--)
    {
      for (int j = 0; j < p_cnt[i]; j++)
      {
        int ind = button_value[i][j];
        // Use fl_tmp0 and fl_tmp to speed things up a bit.
        float fl_tmp0 = yc[ind][0] * ym_env[i][0];
        //
        // Only the non muted channels are added to the output signal.
        if (mute_modulator[i] == false) {
          //
          // Left channel.
          float fl_tmp = fl_tmp0 * left_level[i];
          // Compute output value of signal ( superposition )
          outputs[LEFT_OUTPUT].setVoltage(outputs[LEFT_OUTPUT].getVoltage() + fl_tmp);
          //
          // Right channel.
          fl_tmp = fl_tmp0 * right_level[i];
          // Compute output value of signal ( superposition )
          outputs[RIGHT_OUTPUT].setVoltage(outputs[RIGHT_OUTPUT].getVoltage() + fl_tmp);
        }
      }
    }
  }
}

struct Vocode_O_MaticWidget : ModuleWidget {
  Vocode_O_MaticWidget(Vocode_O_Matic *module) {
	setModule(module);

    // Set background.
    setPanel(APP->window->loadSvg(asset::plugin(thePlugin, "res/Sculpt-O-Sound-_-Vocode_O_Matic_v0.4.svg")));

    // Add some screws.
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    // Dial for carrier gain.
    // Dial for modulator gain.
    // Dial for panning.
    // Note: format is Vec(x-pos, y-pos)
    addParam(createParam<RoundSmallBlackKnob>(Vec(10,  25), module, Vocode_O_Matic::CARRIER_GAIN_PARAM));
    addParam(createParam<RoundSmallBlackKnob>(Vec(40,  25), module, Vocode_O_Matic::MODULATOR_GAIN_PARAM));
#ifdef PANNING
    addParam(createParam<RoundSmallBlackKnob>(Vec(70,  25), module, Vocode_O_Matic::PANNING_PARAM));
#endif

    // INTPUTS (SIGNAL AND PARAMS)
    // Carrier, modulator and shift left and right Input signals.
    addInput(createInput<PJ301MPort>(Vec(10, 180), module, Vocode_O_Matic::CARR_INPUT));
    addInput(createInput<PJ301MPort>(Vec(42, 180), module, Vocode_O_Matic::MOD_INPUT));
    addInput(createInput<PJ301MPort>(Vec(105, 140), module, Vocode_O_Matic::SHIFT_RIGHT_INPUT));
    addInput(createInput<PJ301MPort>(Vec(105, 103), module, Vocode_O_Matic::SHIFT_LEFT_INPUT));

    // Bypass switch.
    addParam(createParam<LEDBezel>(Vec(12,  66), module, Vocode_O_Matic::BYPASS_SWITCH));
    addChild(createLight<LedLight<RedLight>>(Vec(14.2, 68), module, Vocode_O_Matic::BYPASS_LIGHT));

    // Matrix type switch: linear, inverse + 4 * log
    addParam(createParam<LEDBezel>(Vec(12, 104), module, Vocode_O_Matic::MATRIX_MODE_TOGGLE_PARAM));
    addChild(createLight<LedLight<GreenLight>>(Vec(14.2, 106), module, Vocode_O_Matic::MATRIX_MODE_TOGGLE_LIGHT));

    // Push button which shifts the matrix to the right one step at a time.
    addParam(createParam<LEDBezel>(Vec(76, 142), module, Vocode_O_Matic::MATRIX_ONE_STEP_RIGHT_PARAM));
    addChild(createLight<LedLight<GreenLight>>(Vec(78.2, 144), module, Vocode_O_Matic::MATRIX_ONE_STEP_RIGHT_LIGHT));

    // Push button which shifts the matrix to the left one step at a time.
    addParam(createParam<LEDBezel>(Vec(76, 104), module, Vocode_O_Matic::MATRIX_ONE_STEP_LEFT_PARAM));
    addChild(createLight<LedLight<GreenLight>>(Vec(78.2, 106), module, Vocode_O_Matic::MATRIX_ONE_STEP_LEFT_LIGHT));

    // Matrix shift hold toggle.
    addParam(createParam<LEDBezel>(Vec(12, 142), module, Vocode_O_Matic::MATRIX_HOLD_TOGGLE_PARAM));
    addChild(createLight<LedLight<RedLight>>(Vec(14.2, 144), module, Vocode_O_Matic::MATRIX_HOLD_TOGGLE_LIGHT));

    // Matrix Mode Display
    MsDisplayWidget *matrix_mode_display = new MsDisplayWidget();
    matrix_mode_display->box.pos = Vec(38, 105);
    matrix_mode_display->box.size = Vec(30, 20);
    if (module) {
      matrix_mode_display->value = &module->matrix_mode;
    }
    addChild(matrix_mode_display);

    // Matrix Shift Position Display
    MsDisplayWidget *matrix_shift_position_display = new MsDisplayWidget();
    matrix_shift_position_display->box.pos = Vec(38, 143);
    matrix_shift_position_display->box.size = Vec(30, 20);
    if (module) {
      matrix_shift_position_display->value = &module->matrix_shift_position;
    }
    addChild(matrix_shift_position_display);

    // Output of vocoded signal.
    addOutput(createOutput<PJ301MPort>(Vec(10, 219), module, Vocode_O_Matic::LEFT_OUTPUT));
    addOutput(createOutput<PJ301MPort>(Vec(42, 219), module, Vocode_O_Matic::RIGHT_OUTPUT));

    // Add the button matrix. Note its origin is on the bottom left.
    for (int i = 0; i < NR_OF_BANDS; i++) {
      for (int j = 0; j < NR_OF_BANDS; j++) {
        int x = HBASE + j * LED_WIDTH - 0.20 * LED_WIDTH;
        int y = VBASE - i * (LED_HEIGHT + 1);
        int offset = i * NR_OF_BANDS + j;
        {
          LButton *lb = new LButton();
          lb->module = module;
          lb->box.pos = Vec(x, y);
          lb->paramId = Vocode_O_Matic::MOD_MATRIX_PARAM + offset;
          lb->initParamQuantity();
          addChild(lb);
        }
        addChild(createLight<MediumLight<BlueLight>>(Vec(x, y), module, Vocode_O_Matic::MOD_MATRIX_LIGHT + offset));
      }
    }

    // Add mute output buttons on the RHS of the matrix.
    int x = HBASE + 0.25 * LED_WIDTH + NR_OF_BANDS * LED_WIDTH;
    for (int i = 0; i < NR_OF_BANDS; i++) {
      int y = VBASE - i * (LED_HEIGHT + 1);
      int offset = i; // * NR_OF_BANDS;
      {
        LButton *lb = new LButton();
        lb->module = module;
        lb->box.pos = Vec(x, y);
        lb->paramId = Vocode_O_Matic::MUTE_MODULATOR_PARAM + offset;
        lb->initParamQuantity();
        addChild(lb);
      }
      addChild(createLight<MediumLight<GreenLight>>(Vec(x, y), module, Vocode_O_Matic::MUTE_MODULATOR_LIGHT + offset));
    }
  };
};

Model *modelVocode_O_Matic = createModel<Vocode_O_Matic, Vocode_O_MaticWidget>("Vocode_O_Matic");
