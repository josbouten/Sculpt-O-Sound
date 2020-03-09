#include "Sculpt-O-Sound.hpp"
#include "std.hpp"
#include "matrix.hpp"
#include "Vocode_O_Matic_XL.hpp"
#include "../deps/SynthDevKit/src/CV.hpp"
#include "pan_and_level.hpp"

// Vocode_O_Matic_XL

// Dimensions of matrix of buttons.
#define LED_WIDTH 10
#define LED_HEIGHT 10

#define VBASE NR_OF_BANDS * LED_HEIGHT + 40
#define ORIGIN_BOTTOM_LEFT
#define HBASE 140
#define SLIDERS_X_OFFSET 487

void Vocode_O_Matic_XL::onReset() {
  // Initialize essential variables and show leds.
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
    mute_modulator_old[i] = false;
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

  // Set gain to initial value. Note: this does not work!
  //params[CARRIER_GAIN_PARAM].value = INITIAL_CARRIER_GAIN;
  //params[MODULATOR_GAIN_PARAM].value = INITIAL_MODULATOR_GAIN;

  init_attack_times(envelope_attack_time);
  init_release_times(envelope_release_time);
  for (int i = 0; i < NR_OF_BANDS; i++) {
    params[ATTACK_TIME_PARAM + i].value = envelope_attack_time[i];
    params[RELEASE_TIME_PARAM + i].value = envelope_release_time[i];
  }
  comp_attack_factors(envelope_attack_factor, envelope_attack_time);
  comp_release_factors(envelope_release_factor, envelope_release_time);
}

void Vocode_O_Matic_XL::onRandomize() {
  // All buttons are of type params[] and will be randomly pressed.
  // Therefore copy their value to button_values and their number
  // for each band to p_cnt.
  int cnt = 3;
  clear_matrix(button_value, p_cnt);
  for (int i = 0; i < NR_OF_BANDS; i++) {
      for (int j = 0; j < cnt; j++) {
          int jj = i / 2 + (int) ((float) NR_OF_BANDS / 2.0 * rand() / (RAND_MAX + 1.0));
          button_value[i][p_cnt[i]++] = jj;
      }
  }
  // Show the lights corresponding to the matrix
  refresh_led_matrix(lights_offset, p_cnt, button_value, led_state);

  // Set some mute output buttons to mute.
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

  // Set gain to initial value. This will not work. The param values are set
  // in the library where they can not be reached?
  params[CARRIER_GAIN_PARAM].setValue(INITIAL_CARRIER_GAIN);
  params[MODULATOR_GAIN_PARAM].setValue(INITIAL_MODULATOR_GAIN);
  params[PAN_WIDTH_PARAM].setValue(1.0f);
}

void Vocode_O_Matic_XL::process(const ProcessArgs &args) {
  int x, y;
  float rc;
  static int x1 = -1, x2 = -1, y1,  y2;
  static int only_once = 1;
  if (only_once == 1) {
    init_attack_times(envelope_attack_time);
    init_release_times(envelope_release_time);
    for (int i = 0; i < NR_OF_BANDS; i++) {
        params[ATTACK_TIME_PARAM + i].setValue(envelope_attack_time[i]);
        params[RELEASE_TIME_PARAM + i].setValue(envelope_attack_time[i]);
    }
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

#ifdef DEBUGMSG
  bool refresh = false;
#endif

  // Do da vocoding thang,
  // while checking all buttons, dials and sliders.
  float deltaTime = APP->engine->getSampleTime();
  float oneStepDeltaTime = APP->engine->getSampleTime();

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
    // Shift modulator filter taps.
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

  if (pan_width_increase_button_trig.process(params[PAN_WIDTH_INCREASE_PARAM].getValue())) {
      pan_width_increase();
  }

  if (pan_width_decrease_button_trig.process(params[PAN_WIDTH_DECREASE_PARAM].getValue())) {
      pan_width_decrease();
  }

  if (pan_left_button_trig.process(params[PAN_LEFT_PARAM].getValue())) {
      pan_left();
  }

  if (pan_right_button_trig.process(params[PAN_RIGHT_PARAM].getValue())) {
      pan_right();
  }


  if (pan_center_button_trig.process(params[PAN_CENTER_PARAM].getValue())) {
      pan_center();
  }

  if (level_increase_button_trig.process(params[LEVEL_INCREASE_PARAM].getValue())) {
      level_increase();
  }

  if (level_decrease_button_trig.process(params[LEVEL_DECREASE_PARAM].getValue())) {
      level_decrease();
  }

  if (envelope_attack_time_increase_button_trig.process(params[ATTACK_TIME_INCREASE_PARAM].getValue())) {
      increase_attack_time();
  }

  if (envelope_attack_time_decrease_button_trig.process(params[ATTACK_TIME_DECREASE_PARAM].getValue())) {
      decrease_attack_time();
  }

  if (envelope_release_time_increase_button_trig.process(params[RELEASE_TIME_INCREASE_PARAM].getValue())) {
      increase_release_time();
  }

  if (envelope_release_time_decrease_button_trig.process(params[RELEASE_TIME_DECREASE_PARAM].getValue())) {
      decrease_release_time();
  }
 
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
        lights[MOD_MATRIX_LIGHT + buttonNr].setBrightness(1.0);
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


  // Process changes in sliders for envelope attack time, envelope release time, pan and level values.
  if (wait_all_sliders == 0) {
    // Handle envelope attack time sliders changes.
    wait_all_sliders = 20000;
    bool change = false;
    for (int i = 0; i < NR_OF_BANDS; i++) {
        if (params[ATTACK_TIME_PARAM + i].value != envelope_attack_time[i]) {
            envelope_attack_time[i] = params[ATTACK_TIME_PARAM + i].value;
            change = true;
        }
    }
    if (change) {
        // compute factors here.
        comp_attack_factors(envelope_attack_factor, envelope_attack_time);
#ifdef DEBUGMSG
        printf("attack time:");
        print_array(envelope_attack_time);
        printf("attack factor:");
        print_array(envelope_attack_factor);
#endif
    }

    // Handle envelope release time sliders changes.
    change = false;
    for (int i = 0; i < NR_OF_BANDS; i++) {
        if (params[RELEASE_TIME_PARAM + i].value != envelope_release_time[i]) {
            envelope_release_time[i] = params[RELEASE_TIME_PARAM + i].value;
            change = true;
        }
    }
    if (change) {
        // compute factors here.
        comp_release_factors(envelope_release_factor, envelope_release_time);
#ifdef DEBUGMSG
        printf("release time:");
        print_array(envelope_release_time);
        printf("release factor:");
        print_array(envelope_release_factor);
#endif
    }

    // Handle pan and level slider changes here.
    // First process pan slider changes.
    change = false;
    for (int i = 0; i < NR_OF_BANDS; i++) {
        if (params[PAN_PARAM + i].value != slider_pan[i]) {
            slider_pan[i] = params[PAN_PARAM + i].value;
            change = true;
        }
    }
    if (change) {
        for (int i = 0; i < NR_OF_BANDS; i++) {
            left_pan[i] = left_pan_factor(slider_pan[i]);
            right_pan[i] = right_pan_factor(slider_pan[i]);
        }
    }

    // Handle level slider changes here.
    for (int i = 0; i < NR_OF_BANDS; i++) {
        if (params[LEVEL_PARAM + i].value != slider_level[i]) {
            slider_level[i] = params[LEVEL_PARAM + i].value;
            change = true;
        }
    }
    if (change) {
        int width = 1;
        set_pan_and_level(slider_level, slider_pan, left_pan, right_pan, left_level, right_level, width);
    }
  } else {
    wait_all_sliders -= 1;
  }

  // Now we compute the output sample amplitude for both channels.
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

struct Vocode_O_Matic_XL_Widget : ModuleWidget,  Vocode_O_Matic_XL {
  Vocode_O_Matic_XL_Widget(Vocode_O_Matic_XL *module) {
    setModule(module);

    // Set background.
    setPanel(APP->window->loadSvg(asset::plugin(thePlugin, "res/Sculpt-O-Sound-_-Vocode_O_Matic_XL_v0.1.svg")));

    // Add some screws.
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    // Dial for carrier gain.
    // Dial for modulator gain.
    // Note: format is Vec(x-pos, y-pos)
    addParam(createParam<RoundSmallBlackKnob>(Vec(10,  25), module, Vocode_O_Matic_XL::CARRIER_GAIN_PARAM));
    addParam(createParam<RoundSmallBlackKnob>(Vec(40,  25), module, Vocode_O_Matic_XL::MODULATOR_GAIN_PARAM));

    // INTPUTS (SIGNAL AND PARAMS)
    // Carrier, modulator and shift left and right Input signals.
    addInput(createInput<PJ301MPort>(Vec(10, 180), module, Vocode_O_Matic_XL::CARR_INPUT));
    addInput(createInput<PJ301MPort>(Vec(42, 180), module, Vocode_O_Matic_XL::MOD_INPUT));
    addInput(createInput<PJ301MPort>(Vec(105, 140), module, Vocode_O_Matic_XL::SHIFT_RIGHT_INPUT));
    addInput(createInput<PJ301MPort>(Vec(105, 103), module, Vocode_O_Matic_XL::SHIFT_LEFT_INPUT));

    // Bypass switch.
    addParam(createParam<LEDBezel>(Vec(12,  66), module, Vocode_O_Matic_XL::BYPASS_SWITCH));
    addChild(createLight<LedLight<RedLight>>(Vec(14.2, 68), module, Vocode_O_Matic_XL::BYPASS_LIGHT));

    // Matrix type switch: linear, inverse + 4 * log
    addParam(createParam<LEDBezel>(Vec(12, 104), module, Vocode_O_Matic_XL::MATRIX_MODE_TOGGLE_PARAM));
    addChild(createLight<LedLight<GreenLight>>(Vec(14.2, 106), module, Vocode_O_Matic_XL::MATRIX_MODE_TOGGLE_LIGHT));

    // Push button which shifts the matrix to the right one step at a time.
    addParam(createParam<LEDBezel>(Vec(76, 142), module, Vocode_O_Matic_XL::MATRIX_ONE_STEP_RIGHT_PARAM));
    addChild(createLight<LedLight<GreenLight>>(Vec(78.2, 144), module, Vocode_O_Matic_XL::MATRIX_ONE_STEP_RIGHT_LIGHT));

    // Push button which shifts the matrix to the left one step at a time.
    addParam(createParam<LEDBezel>(Vec(76, 104), module, Vocode_O_Matic_XL::MATRIX_ONE_STEP_LEFT_PARAM));
    addChild(createLight<LedLight<GreenLight>>(Vec(78.2, 106), module, Vocode_O_Matic_XL::MATRIX_ONE_STEP_LEFT_LIGHT));

    // Matrix shift hold toggle.
    addParam(createParam<LEDBezel>(Vec(12, 142), module, Vocode_O_Matic_XL::MATRIX_HOLD_TOGGLE_PARAM));
    addChild(createLight<LedLight<RedLight>>(Vec(14.2, 144), module, Vocode_O_Matic_XL::MATRIX_HOLD_TOGGLE_LIGHT));

    // Matrix Mode Display.
    MsDisplayWidget *matrix_mode_display = new MsDisplayWidget(); 
    matrix_mode_display->box.pos = Vec(38, 105); 
    matrix_mode_display->box.size = Vec(30, 20); 
    if (module) {
        matrix_mode_display->value = &module->matrix_mode;
    }
    addChild(matrix_mode_display); 

    // Matrix Shift Position Display.
    MsDisplayWidget *matrix_shift_position_display = new MsDisplayWidget();
    matrix_shift_position_display->box.pos = Vec(38, 143);
    matrix_shift_position_display->box.size = Vec(30, 20); 
    if (module) {
        matrix_shift_position_display->value = &module->matrix_shift_position;
    }
    addChild(matrix_shift_position_display);

    // Output of vocoded signal.
    addOutput(createOutput<PJ301MPort>(Vec(10, 219), module, Vocode_O_Matic_XL::LEFT_OUTPUT));
    addOutput(createOutput<PJ301MPort>(Vec(42, 219), module, Vocode_O_Matic_XL::RIGHT_OUTPUT));

    // Matrix, origin is bottom left.
    for (int i = 0; i < NR_OF_BANDS; i++) {
        for (int j = 0; j < NR_OF_BANDS; j++) {
            int x = HBASE + j * LED_WIDTH - 0.20 * LED_WIDTH;
            int y = VBASE - i * (LED_HEIGHT + 1);
            int offset = i * NR_OF_BANDS + j;
            {
                LButton_XL *lb = new LButton_XL();
                lb->module = module;
                lb->box.pos = Vec(x, y);
                if (module) {
                    lb->paramQuantity = module->paramQuantities[Vocode_O_Matic_XL::MOD_MATRIX_PARAM + offset];
                }
                addChild(lb);
            }
            addChild(createLight<MediumLight<BlueLight>>(Vec(x, y), module, Vocode_O_Matic_XL::MOD_MATRIX_LIGHT + offset));
        }
    }
    // Add mute output buttons on the RHS of the matrix.
    int x = HBASE + 0.25 * LED_WIDTH + NR_OF_BANDS * LED_WIDTH;
    for (int i = 0; i < NR_OF_BANDS; i++) {
            int y = VBASE - i * (LED_HEIGHT + 1);
            int offset = i; // * NR_OF_BANDS;
            {
                LButton_XL *lb = new LButton_XL();
                lb->module = module;
                lb->box.pos = Vec(x, y);
                if (module) {
                    lb->paramQuantity = module->paramQuantities[Vocode_O_Matic_XL::MUTE_MODULATOR_PARAM + offset];
                }
                addChild(lb);
            }
            addChild(createLight<MediumLight<GreenLight>>(Vec(x, y), module, Vocode_O_Matic_XL::MUTE_MODULATOR_LIGHT + offset));
    }

    // Add 4 rows of sliders (from bottom to top of module) for
    for (int i = 0; i < NR_OF_BANDS; i++) {
        // panning,
        pan_slider[i] = createParam<SliderWithId>(Vec(SLIDERS_X_OFFSET + i * 12, 280), module, Vocode_O_Matic_XL::PAN_PARAM + i);
        pan_slider[i]->id = i;
        pan_slider[i]->type = SliderWithId::PAN;
        addParam(pan_slider[i]);

        // level,
        level_slider[i] = createParam<SliderWithId>(Vec(SLIDERS_X_OFFSET + i * 12, 190), module, Vocode_O_Matic_XL::LEVEL_PARAM + i);
        level_slider[i]->id = i;
        level_slider[i]->type = SliderWithId::LEVEL;
        addParam(level_slider[i]);

        // envelope attack time and
        attack_time_slider[i] = createParam<SliderWithId>(Vec(SLIDERS_X_OFFSET + i * 12,  100), module, Vocode_O_Matic_XL::ATTACK_TIME_PARAM + i);
        attack_time_slider[i]->id = i;
        attack_time_slider[i]->type = SliderWithId::ATTACK_TIME;
        addParam(attack_time_slider[i]);

        // envelope release time.
        release_time_slider[i] = createParam<SliderWithId>(Vec(SLIDERS_X_OFFSET + i * 12,  10), module, Vocode_O_Matic_XL::RELEASE_TIME_PARAM + i);
        release_time_slider[i]->id = i;
        release_time_slider[i]->type = SliderWithId::RELEASE_TIME;
        addParam(release_time_slider[i]);

    }

    // Push buttons for panning. 
    addParam(createParam<ButtonUp>(Vec(863, 285), module, Vocode_O_Matic_XL::PAN_LEFT_PARAM));
    addParam(createParam<ButtonCenter>(Vec(863, 310), module, Vocode_O_Matic_XL::PAN_CENTER_PARAM));
    addParam(createParam<ButtonDown>(Vec(863, 335), module, Vocode_O_Matic_XL::PAN_RIGHT_PARAM));

    addParam(createParam<ButtonUp>(Vec(SLIDERS_X_OFFSET - 16, 285), module, Vocode_O_Matic_XL::PAN_WIDTH_INCREASE_PARAM));
    addParam(createParam<ButtonCenter>(Vec(SLIDERS_X_OFFSET - 16, 310), module, Vocode_O_Matic_XL::PAN_CENTER_PARAM));
    addParam(createParam<ButtonDown>(Vec(SLIDERS_X_OFFSET - 16, 335), module, Vocode_O_Matic_XL::PAN_WIDTH_DECREASE_PARAM));

    // Push buttons for level sliders increase / decrease.
    addParam(createParam<ButtonUp>(Vec(863, 195), module, Vocode_O_Matic_XL::LEVEL_INCREASE_PARAM));
    addParam(createParam<ButtonDown>(Vec(863, 245), module, Vocode_O_Matic_XL::LEVEL_DECREASE_PARAM));

    // Push buttons for attack time sliders increase / decrease.
    addParam(createParam<ButtonUp>(Vec(863, 105), module, Vocode_O_Matic_XL::ATTACK_TIME_INCREASE_PARAM));
    addParam(createParam<ButtonDown>(Vec(863, 160), module, Vocode_O_Matic_XL::ATTACK_TIME_DECREASE_PARAM));

    // Push buttons for release time sliders increase / decrease.
    addParam(createParam<ButtonUp>(Vec(863,  15), module, Vocode_O_Matic_XL::RELEASE_TIME_INCREASE_PARAM));
    addParam(createParam<ButtonDown>(Vec(863,  70), module, Vocode_O_Matic_XL::RELEASE_TIME_DECREASE_PARAM));
  };
};

Model *modelVocode_O_Matic_XL = createModel<Vocode_O_Matic_XL, Vocode_O_Matic_XL_Widget>("Vocode_O_Matic_XL");
