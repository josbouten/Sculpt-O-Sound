#include "Sculpt-O-Sound.hpp"
#include "std.hpp"
#include "matrix.hpp"
#include "Vocode_O_Matic_XL.hpp"
#include "lbutton.hpp"
#include "../deps/SynthDevKit/src/CV.hpp"
#include "pan_and_level.hpp"
#include "sliders.hpp"


// Dimensions of matrix of buttons.
#define LED_WIDTH 10
#define LED_HEIGHT 10

#define VBASE NR_OF_BANDS * LED_HEIGHT + 40
#define ORIGIN_BOTTOM_LEFT
#define HBASE 140
#define SLIDERS_X_OFFSET 480

void Vocode_O_Matic_XL::onReset() {
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
    mute_output[i] = false;
    params[MUTE_OUTPUT_PARAM_00 + i].value = 0.0;
    lights[MUTE_OUTPUT_LIGHT_00 + i].value = 1.0;
  }
  // Show mute led values.
  refresh_mute_output_leds(MUTE_OUTPUT_LIGHT_00, mute_output);

  blinkPhase = -1.0f;
  oneStepBlinkPhase = 0.0f;

  // Reset toggle lights.
  lights[MATRIX_HOLD_TOGGLE_LIGHT].value = 0.0;
  lights[BYPASS_LIGHT].value = 0.0;

  // Set gain to initial value. Note: this does not work!
  //params[CARRIER_GAIN_PARAM].value = INITIAL_CARRIER_GAIN;
  //params[MODULATOR_GAIN_PARAM].value = INITIAL_MODULATOR_GAIN;

  init_attack_times(envelope_attack_time);
  comp_attack_factors(envelope_attack_factor, envelope_attack_time);
  init_release_times(envelope_release_time);
  comp_release_factors(envelope_release_factor, envelope_release_time);
}

void Vocode_O_Matic_XL::onRandomize() {
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
        mute_output[i] = false;
        lights[MUTE_OUTPUT_LIGHT_00 + i].value = 1.0;
    } else {
        mute_output[i] = true;
        lights[MUTE_OUTPUT_LIGHT_00 + i].value = 0.0;
    }
  }

  // Show mute led values.
  refresh_mute_output_leds(MUTE_OUTPUT_LIGHT_00, mute_output);

  // Set gain to initial value.
  params[CARRIER_GAIN_PARAM].value = INITIAL_CARRIER_GAIN;
  params[MODULATOR_GAIN_PARAM].value = INITIAL_MODULATOR_GAIN;
}

void Vocode_O_Matic_XL::step() {
  // Do da vocoding thang.
  float deltaTime = engineGetSampleTime();
  float oneStepDeltaTime = engineGetSampleTime();
  bool refresh = false;
             
  xc[0] = inputs[CARR_INPUT].value * params[CARRIER_GAIN_PARAM].value;
  xm[0] = inputs[MOD_INPUT].value * params[MODULATOR_GAIN_PARAM].value;
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
    lights[MATRIX_ONE_STEP_RIGHT_LIGHT].value = 0.0f;
    lights[MATRIX_ONE_STEP_LEFT_LIGHT].value = 0.0f;
  }

  // Process trigger signal on matrix shift input. 
  float shiftRightTriggerIn = inputs[SHIFT_RIGHT_INPUT].value;
  cv_right->update(shiftRightTriggerIn);
  float shiftLeftTriggerIn = inputs[SHIFT_LEFT_INPUT].value;
  cv_left->update(shiftLeftTriggerIn);

  // Handling of buttons and / or lights.
  // If bypass button was pressed:
  if (bypass_button_trig.process(params[BYPASS_SWITCH].value)) {
    fx_bypass = !fx_bypass;
  }
  lights[BYPASS_LIGHT].value = fx_bypass ? 1.00 : 0.0;
  if (matrix_hold_button_pressed) { // We blink only if the button is toggled in the on position.
    lights[MATRIX_HOLD_TOGGLE_LIGHT].value = (blinkPhase < 0.5f) ? 1.0f : 0.0f;
  }

  // Hold matrix movement if toggle is pressed.
  if (matrix_hold_button_trig.process(params[MATRIX_HOLD_TOGGLE_PARAM].value)) {
    matrix_hold_button_pressed = !matrix_hold_button_pressed;
    lights[MATRIX_HOLD_TOGGLE_LIGHT].value = matrix_hold_button_pressed ? 1.00 : 0.0;
  }

  // If one step right button was pressed:
  if (matrix_one_step_right_button_trig.process(params[MATRIX_ONE_STEP_RIGHT_PARAM].value)) {
    // Start a new blink period.
    oneStepBlinkPhase = 0.0f;
    // Light up the button;
    lights[MATRIX_ONE_STEP_RIGHT_LIGHT].value = 1.00;
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
  if (matrix_one_step_left_button_trig.process(params[MATRIX_ONE_STEP_LEFT_PARAM].value)) {
    // Start a new blink period.
    oneStepBlinkPhase = 0.0f;
    // Light up the button;
    lights[MATRIX_ONE_STEP_LEFT_LIGHT].value = 1.00;
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
  if (matrix_mode_button_trig.process(params[MATRIX_MODE_TOGGLE_PARAM].value)) {
    matrix_mode_button_pressed = false;
    lights[MATRIX_MODE_TOGGLE_PARAM].value = matrix_mode_button_pressed ? 1.00 : 0.0;
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
  width = params[PANNING_PARAM].value;
  if (width != width_old) {
    set_pan_and_level(slider_level, left_pan, right_pan, left_level, right_level, width);
    width_old = width;
  }
#endif

  // Handle the matrix button presses every half second ( assumption Fsamp = 44100 Hz ).
  if (wait == 0) {
    for (int i = 0; i < NR_OF_BANDS * NR_OF_BANDS; i++) {
        if (params[MOD_MATRIX_PARAM + i].value) {
            wait = 20000;
            led_state[i] = !led_state[i]; 
            int index = MOD_MATRIX + i;
            lights[index].value = !lights[index].value;
            int chosen_row = i / NR_OF_BANDS;
            int chosen_col = i % NR_OF_BANDS;
            if ((p_cnt[chosen_row] > 0) && (!led_state[i])) {
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
        }
    }
  } else wait -= 1;

  // Handle mute buttons every half second.
  for (int i = 0; i <NR_OF_BANDS; i++) {
      if (mute_output_trig.process(params[MUTE_OUTPUT_PARAM_00 + i].value)) {
          if (wait2 == 0) {
              wait2 = 20000;
              mute_output[i] = !mute_output[i];
              lights[MUTE_OUTPUT_LIGHT_00 + i].value = 1.0 - lights[MUTE_OUTPUT_LIGHT_00 + i].value;
              refresh = true;
          } else {
              wait2 -= 1;
          }
      }
  }
#ifdef DEBUG
  if (refresh) {
      print_mute_buttons(mute_output);
      refresh_mute_output_leds(MUTE_OUTPUT_LIGHT_00, mute_output);
      refresh = false;
  }
#endif

  // Process changes in sliders for envelope attack time, envelope release time, pan and level values
  if (wait_all_sliders == 0) {
    // Handle envelope attack time sliders changes.
    wait_all_sliders = 20000;
    bool change = false;
    for (int i = 0; i < NR_OF_BANDS; i++) {
        if (params[ATTACK_TIME_PARAM_00 + i].value != envelope_attack_time[i]) {
            envelope_attack_time[i] = params[ATTACK_TIME_PARAM_00 + i].value;
            change = true;
        }
    }
    if (change) {
        // compute factors here.
        comp_attack_factors(envelope_attack_factor, envelope_attack_time);
    }

    // Handle envelope release time sliders changes.
    change = false;
    for (int i = 0; i < NR_OF_BANDS; i++) {
        if (params[RELEASE_TIME_PARAM_00 + i].value != envelope_release_time[i]) {
            envelope_release_time[i] = params[RELEASE_TIME_PARAM_00 + i].value;
            change = true;
        }
    }
    if (change) {
        // compute factors here.
        comp_release_factors(envelope_release_factor, envelope_release_time);
    }
 
    // Handle pan slider changes here.
    change = false;
    for (int i = 0; i < NR_OF_BANDS; i++) {
        if (params[PAN_PARAM_00 + i].value != slider_pan[i]) {
            slider_pan[i] = params[PAN_PARAM_00 + i].value;
            change = true;
        }
    }

    // ToDo: how to implement the pan slider values ???

    // Handle level slider changes here.
    for (int i = 0; i < NR_OF_BANDS; i++) {
        if (params[LEVEL_PARAM_00 + i].value != slider_level[i]) {
            slider_level[i] = params[LEVEL_PARAM_00 + i].value;
            change = true;
        }
    }
    if (change) {
        int width = 1;
        set_pan_and_level(slider_level, left_pan, right_pan, left_level, right_level, width);
    }
    
  } else {
    wait_all_sliders -= 1;
  }
    
  if (fx_bypass) {
    outputs[LEFT_OUTPUT].value = inputs[CARR_INPUT].value * params[CARRIER_GAIN_PARAM].value;
    outputs[RIGHT_OUTPUT].value = inputs[MOD_INPUT].value * params[MODULATOR_GAIN_PARAM].value;
  } 
  else 
  {
    // Initialize output signal.
    outputs[RIGHT_OUTPUT].value = 0.0;
    outputs[LEFT_OUTPUT].value = 0.0;
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
        if (mute_output[i] == false) {
            //
            // Left channel.
            float fl_tmp = fl_tmp0 * left_level[i];
            // Compute output value of signal ( superposition )
            outputs[LEFT_OUTPUT].value += fl_tmp;
            //
            // Right channel.
            fl_tmp = fl_tmp0 * right_level[i];
            // Compute output value of signal ( superposition )
            outputs[RIGHT_OUTPUT].value += fl_tmp;
        }
      }
    }
  }
}

struct Vocode_O_Matic_XL_Widget : ModuleWidget, Vocode_O_Matic_XL {
  Vocode_O_Matic_XL_Widget(Vocode_O_Matic_XL *module) : ModuleWidget(module) {

    // Set background.
    setPanel(SVG::load(assetPlugin(plugin, "res/Sculpt-O-Sound-_-Vocode_O_Matic_v0.5.svg")));

    // Add some screws.
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    // Dial for carrier gain.
    // Dial for modulator gain.
    // Dial for panning.
    // Note: format is Vec(x-pos, y-pos)
    addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(10,  25), module, Vocode_O_Matic_XL::CARRIER_GAIN_PARAM, 1.0, MAX_CARRIER_GAIN, INITIAL_CARRIER_GAIN));
    addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(40,  25), module, Vocode_O_Matic_XL::MODULATOR_GAIN_PARAM, 1.0, MAX_MODULATOR_GAIN, INITIAL_MODULATOR_GAIN));
#ifdef PANNING
    //addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(70,  47), module, Vocode_O_Matic_XL::PANNING_PARAM, 0.0, MAX_PAN, 1.0 / INITIAL_PAN_OFFSET));
    addParam(ParamWidget::create<RoundSmallBlackKnob>(Vec(70,  25), module, Vocode_O_Matic_XL::PANNING_PARAM, 0.5, MAX_PAN, 1.0 / INITIAL_PAN_OFFSET));
#endif

    // INTPUTS (SIGNAL AND PARAMS)
    // Carrier, modulator and shift left and right Input signals.
    addInput(Port::create<PJ301MPort>(Vec(10, 180), Port::INPUT, module, Vocode_O_Matic_XL::CARR_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(42, 180), Port::INPUT, module, Vocode_O_Matic_XL::MOD_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(105, 140), Port::INPUT, module, Vocode_O_Matic_XL::SHIFT_RIGHT_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(105, 103), Port::INPUT, module, Vocode_O_Matic_XL::SHIFT_LEFT_INPUT));

    // Bypass switch.
    addParam(ParamWidget::create<LEDBezel>(Vec(12,  66), module, Vocode_O_Matic_XL::BYPASS_SWITCH , 0.0f, 1.0f, 0.0f));
    addChild(ModuleLightWidget::create<LedLight<RedLight>>(Vec(14.2, 68), module, Vocode_O_Matic_XL::BYPASS_LIGHT));

    // Matrix type switch: linear, inverse + 4 * log
    addParam(ParamWidget::create<LEDBezel>(Vec(12, 104), module, Vocode_O_Matic_XL::MATRIX_MODE_TOGGLE_PARAM, 0.0f, 1.0f, 0.0f));
    addChild(ModuleLightWidget::create<LedLight<GreenLight>>(Vec(14.2, 106), module, Vocode_O_Matic_XL::MATRIX_MODE_TOGGLE_LIGHT));

    // Push button which shifts the matrix to the right one step at a time.
    addParam(ParamWidget::create<LEDBezel>(Vec(76, 142), module, Vocode_O_Matic_XL::MATRIX_ONE_STEP_RIGHT_PARAM, 0.0f, 1.0f, 0.0f));
    addChild(ModuleLightWidget::create<LedLight<GreenLight>>(Vec(78.2, 144), module, Vocode_O_Matic_XL::MATRIX_ONE_STEP_RIGHT_LIGHT));

    // Push button which shifts the matrix to the left one step at a time.
    addParam(ParamWidget::create<LEDBezel>(Vec(76, 104), module, Vocode_O_Matic_XL::MATRIX_ONE_STEP_LEFT_PARAM, 0.0f, 1.0f, 0.0f));
    addChild(ModuleLightWidget::create<LedLight<GreenLight>>(Vec(78.2, 106), module, Vocode_O_Matic_XL::MATRIX_ONE_STEP_LEFT_LIGHT));

    // Matrix shift hold toggle.
    addParam(ParamWidget::create<LEDBezel>(Vec(12, 142), module, Vocode_O_Matic_XL::MATRIX_HOLD_TOGGLE_PARAM, 0.0f, 1.0f, 0.0f));
    addChild(ModuleLightWidget::create<LedLight<RedLight>>(Vec(14.2, 144), module, Vocode_O_Matic_XL::MATRIX_HOLD_TOGGLE_LIGHT));

    // Matrix Mode Display
    MsDisplayWidget2 *matrix_mode_display = new MsDisplayWidget2();                            
    matrix_mode_display->box.pos = Vec(38, 105);                                               
    matrix_mode_display->box.size = Vec(30, 20);                                             
    matrix_mode_display->value = &module->matrix_mode;
    addChild(matrix_mode_display);                                                           

    // Matrix Shift Position Display 
    MsDisplayWidget2 *matrix_shift_position_display = new MsDisplayWidget2();           
    matrix_shift_position_display->box.pos = Vec(38, 143);
    matrix_shift_position_display->box.size = Vec(30, 20);                                             
    matrix_shift_position_display->value = &module->matrix_shift_position;
    addChild(matrix_shift_position_display);                                                           

    // Output of vocoded signal.
    addOutput(Port::create<PJ301MPort>(Vec(10, 219), Port::OUTPUT, module, Vocode_O_Matic_XL::LEFT_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(42, 219), Port::OUTPUT, module, Vocode_O_Matic_XL::RIGHT_OUTPUT));

    // Matrix, origin is bottom left.
    for (int i = 0; i < NR_OF_BANDS; i++) {
        for (int j = 0; j < NR_OF_BANDS; j++) {
            int x = HBASE + j * LED_WIDTH - 0.20 * LED_WIDTH;
            int y = VBASE - i * (LED_HEIGHT + 1);
            int offset = i * NR_OF_BANDS + j;
            addParam(ParamWidget::create<LButton>(Vec(x, y), module, Vocode_O_Matic_XL::MOD_MATRIX_PARAM + offset, 0.0, 1.0f, 0.0f));
            addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(x, y), module, Vocode_O_Matic_XL::MOD_MATRIX + offset));
        }
    }
    // Mute output buttons to the left of the matrix.
    int x = HBASE + 0.25 * LED_WIDTH + NR_OF_BANDS * LED_WIDTH;
    for (int i = 0; i < NR_OF_BANDS; i++) {
            int y = VBASE - i * (LED_HEIGHT + 1);
            int offset = i; // * NR_OF_BANDS;
            addParam(ParamWidget::create<LButton>(Vec(x, y), module, Vocode_O_Matic_XL::MUTE_OUTPUT_PARAM_00 + offset, 0.0, 1.0f, 0.0f));
            addChild(ModuleLightWidget::create<MediumLight<GreenLight>>(Vec(x, y), module, Vocode_O_Matic_XL::MUTE_OUTPUT_LIGHT_00 + offset));
    }

    // Add 4 rows of sliders for 
    for (int i = 0; i < NR_OF_BANDS; i++) {
        // envelope attack time,
        attack_time_slider[i] = ParamWidget::create<SliderWithId>(Vec(SLIDERS_X_OFFSET + i * 12,  10), module, Vocode_O_Matic_XL::ATTACK_TIME_PARAM_00 + i, min_envelope_attack_time[i], max_envelope_attack_time[i], INITIAL_ATTACK_TIME);
        attack_time_slider[i] ->id = i;
        attack_time_slider[i] ->type = SliderWithId::ATTACK_TIME;
        addParam(attack_time_slider[i]);

        // envelope release time,
        release_time_slider[i] = ParamWidget::create<SliderWithId>(Vec(SLIDERS_X_OFFSET + i * 12,  100), module, Vocode_O_Matic_XL::RELEASE_TIME_PARAM_00 + i, min_envelope_release_time[i], max_envelope_release_time[i], INITIAL_RELEASE_TIME);
        release_time_slider[i]->id = i;
        release_time_slider[i]->type = SliderWithId::RELEASE_TIME;
        addParam(release_time_slider[i]);

        // level and
        level_slider[i] = ParamWidget::create<SliderWithId>(Vec(SLIDERS_X_OFFSET + i * 12, 190), module, Vocode_O_Matic_XL::LEVEL_PARAM_00 + i, MIN_LEVEL, MAX_LEVEL, INITIAL_LEVEL);
        level_slider[i]->id = i;
        level_slider[i]->type = SliderWithId::LEVEL;
        addParam(level_slider[i]);

        // panning.
        pan_slider[i] = ParamWidget::create<SliderWithId>(Vec(SLIDERS_X_OFFSET + i * 12, 280), module, Vocode_O_Matic_XL::PAN_PARAM_00 + i, MIN_PAN, MAX_PAN, INITIAL_PAN);
        pan_slider[i]->id = i;
        pan_slider[i]->type = SliderWithId::PAN;
        addParam(pan_slider[i]);
    }
  //printf("param: %f %d %d\n", params[LEVEL_PARAM_00 + 5].value, params[LEVEL_PARAM_00 + 5]->id, params[LEVEL_PARAM_00 + 5]->type);
  };
};

// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelVocode_O_Matic_XL = Model::create<Vocode_O_Matic_XL, Vocode_O_Matic_XL_Widget>("Sculpt-O-Sound", "Vocode_O_Matic_XL", "Vocode_O_Matic_XL", VOCODER_TAG);
