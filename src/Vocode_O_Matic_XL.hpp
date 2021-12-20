#include "std.hpp"
#include "../deps/SynthDevKit/src/CV.hpp"
#include "dsp/digital.hpp"
#include "Sculpt-O-Sound.hpp"
#include "comp_coeffs.hpp"
#include "level_sliders.hpp"
#include "sliders.hpp"
#include "pan_and_level.hpp"
#include "matrix.hpp"
#include "buttons.hpp"

struct Vocode_O_Matic_XL : Module {

  // Define CV trigger a la synthkit for shifting the matrix.
  SynthDevKit::CV *cv_right = new SynthDevKit::CV(0.1f);
  SynthDevKit::CV *cv_left =  new SynthDevKit::CV(0.1f);

  void refresh_led_matrix(int lights_offset, int p_cnt[NR_OF_BANDS], int button_value[NR_OF_BANDS][NR_OF_BANDS], bool led_state[1024])
  {
     for (int i = 0; i < NR_OF_BANDS; i++)     
     {
        for (int j = 0; j < NR_OF_BANDS; j++)
        {
            led_state[i * NR_OF_BANDS + j] = false;
            lights[lights_offset + i * NR_OF_BANDS + j].value = false;
        }
        for (int j = 0; j < p_cnt[i]; j++)
        {
            led_state[i * NR_OF_BANDS + button_value[i][j]] = true;
            lights[lights_offset + i * NR_OF_BANDS + button_value[i][j]].value = true;
        }
     }
  }

  void refresh_mute_modulator_leds(int offset, bool mute_modulator[NR_OF_BANDS])
  {
    for (int i = 0; i < NR_OF_BANDS; i++) {
        // Switch led off if mute_modulator is true.
        lights[offset + i].value = (mute_modulator[i] == true) ? 0.0: 1.0;
    }
  }
 
  void refresh_pan_sliders() {
    set_pan_and_level(slider_level, slider_pan, left_pan, right_pan, left_level, right_level, width);
  }

  void shift_buttons_right(int button_value[NR_OF_BANDS][NR_OF_BANDS], int p_cnt[NR_OF_BANDS], bool led_state[1024], int *matrix_shift_position) {
    matrix_shift_buttons_right(button_value, p_cnt);
#ifdef DEBUGMSG
    print_matrix(button_value, p_cnt);
#endif
    // Refresh the visible matrix.
    refresh_led_matrix(lights_offset, p_cnt, button_value, led_state);
    *matrix_shift_position += 1;
    if (*matrix_shift_position >= NR_OF_BANDS) 
        *matrix_shift_position = 0;
  }

  void shift_buttons_left(int button_value[NR_OF_BANDS][NR_OF_BANDS], int p_cnt[NR_OF_BANDS], bool led_state[1024], int *matrix_shift_position) {
    matrix_shift_buttons_left(button_value, p_cnt);
#ifdef DEBUGMSG
    print_matrix(button_value, p_cnt);
#endif
    // Refresh the visible matrix.
    refresh_led_matrix(lights_offset, p_cnt, button_value, led_state);
    *matrix_shift_position -= 1;
    if (*matrix_shift_position < 0) 
        *matrix_shift_position = NR_OF_BANDS - 1;
  }

  void initialize_mute_modulator(bool mute_modulator[NR_OF_BANDS]) {
    for (int i = 0; i < NR_OF_BANDS; i++) {
        mute_modulator[i] = false;
    }
  }
 
  void print_mute_buttons(bool mute_modulator[NR_OF_BANDS]) {
    for (int i = 0; i <  NR_OF_BANDS - 1; i++) {
        printf("%02d: %d ", i, mute_modulator[i] == true ? 1: 0);
    }
    printf("%02d %d\n", NR_OF_BANDS - 1, mute_modulator[NR_OF_BANDS - 1] == true ? 1: 0);
  }

  void handle_single_button(int x, int y, int set) {
    int p, k;
    bool found = false;
    if (set == PRESSED) {
        for (k = 0; k < p_cnt[x]; k++) {
            if (button_value[x][k] == y) {
                found = true;
                break;
            }
        }
        if (found == false) button_value[x][p_cnt[x]++] = y;
    } else {
        for (k = 0; k < p_cnt[x]; k++) {
            if (button_value[x][k] == y) {
                button_value[x][k] = NOT_PRESSED;
                // Move rest on position back.
                for (p = k; p < p_cnt[x]; p++) {
                    button_value[x][p] = button_value[x][p + 1];
                }
            }
        }
        button_value[x][p_cnt[x]--] = NOT_PRESSED;
    }
  }

  void pan_left() {
      // Decrease pan and limit to MIN_PAN and MAX_PAN.
      float increment = (MAX_PAN - MIN_PAN) / PAN_STEPS;
      for (int i = 0; i < NR_OF_BANDS; i += 1) {
          slider_pan[i] += increment;
          if (slider_pan[i] > MAX_PAN) slider_pan[i] = MAX_PAN;
          params[PAN_PARAM + i].value = slider_pan[i];
      }
      set_pan_and_level(slider_level, slider_pan, left_pan, right_pan, left_level, right_level, width);
  }
  
  void pan_right() {
      // Increase pan and limit to MIN_PAN and MAX_PAN.
      float increment = (MAX_PAN - MIN_PAN) / PAN_STEPS;
      for (int i = 0; i < NR_OF_BANDS; i += 1) {
          slider_pan[i] -= increment;
          if (slider_pan[i] < MIN_PAN) slider_pan[i] = MIN_PAN;
          params[PAN_PARAM + i].value = slider_pan[i];
      }
      set_pan_and_level(slider_level, slider_pan, left_pan, right_pan, left_level, right_level, width);
  }

  // Decrease pan and limit to MIN_PAN and MAX_PAN.
  // Because decreasing implies that PAN can cross 0, we need to 
  // check against MIN_PAN and MAX_PAN.
  void pan_width_decrease() {
      float increment = (MAX_PAN - MIN_PAN) / PAN_STEPS;
      for (int i = 0; i < NR_OF_BANDS; i += 2) {
          slider_pan[i] += increment;
          if (slider_pan[i] < MIN_PAN) slider_pan[i] = MIN_PAN;
          if (slider_pan[i] > MAX_PAN) slider_pan[i] = MAX_PAN;
          params[PAN_PARAM + i].value = slider_pan[i];
          if (i < (NR_OF_BANDS - 1)) {
            slider_pan[i + 1] -= increment;
            if (slider_pan[i + 1] < MIN_PAN) slider_pan[i + 1] = MIN_PAN;
            if (slider_pan[i + 1] > MAX_PAN) slider_pan[i + 1] = MAX_PAN;
            params[PAN_PARAM + i + 1].value = slider_pan[i + 1];
          }
      }
      set_pan_and_level(slider_level, slider_pan, left_pan, right_pan, left_level, right_level, width);
  }
  
  // Increase pan and limit to MIN_PAN and MAX_PAN.
  void pan_width_increase() {
      float increment = (MAX_PAN - MIN_PAN) / PAN_STEPS;
      for (int i = 0; i < NR_OF_BANDS; i += 2) {
          slider_pan[i] -= increment;
          if (slider_pan[i] > MAX_PAN) slider_pan[i] = MAX_PAN;
          if (slider_pan[i] < MIN_PAN) slider_pan[i] = MIN_PAN;
          params[PAN_PARAM + i].value = slider_pan[i];
          if (i < (NR_OF_BANDS - 1)) {
            slider_pan[i + 1] += increment;
            if (slider_pan[i + 1] > MAX_PAN) slider_pan[i + 1] = MAX_PAN;
            if (slider_pan[i + 1] < MIN_PAN) slider_pan[i + 1] = MIN_PAN;
            params[PAN_PARAM + i + 1].value = slider_pan[i + 1];
          }
      }
      set_pan_and_level(slider_level, slider_pan, left_pan, right_pan, left_level, right_level, width);
  }
 
  // Set pan to center position. 
  void pan_center() {
      for (int i = 0; i < NR_OF_BANDS; i += 1) {
          slider_pan[i] = 0;
          params[PAN_PARAM + i].value = 0.0;
      }
      set_pan_and_level(slider_level, slider_pan, left_pan, right_pan, left_level, right_level, width);
  }
 
  // Increase levels of all bands with a small value. 
  void level_increase() {
    float increment = (MAX_LEVEL - MIN_LEVEL) / LEVEL_STEPS;
    for (int i = 0; i < NR_OF_BANDS; i += 1) {
        slider_level[i] += increment;
        if (slider_level[i] > MAX_LEVEL) slider_level[i] = MAX_LEVEL;
        params[LEVEL_PARAM + i].value = slider_level[i];
    }
    set_pan_and_level(slider_level, slider_pan, left_pan, right_pan, left_level, right_level, width);
  }

  // Decrease levels of all bands with a small value. 
  void level_decrease() {
    float increment = (MAX_LEVEL - MIN_LEVEL) / LEVEL_STEPS;
    for (int i = 0; i < NR_OF_BANDS; i += 1) {
        slider_level[i] -= increment;
        if (slider_level[i] < MIN_LEVEL) slider_level[i] = MIN_LEVEL;
        params[LEVEL_PARAM + i].value = slider_level[i];
    }
    set_pan_and_level(slider_level, slider_pan, left_pan, right_pan, left_level, right_level, width);
  }


  enum ParamIds {
    // Amplification factor
    DEBUG_PARAM,
    // Bypass switch
    BYPASS_SWITCH,
    // Toggle button to choose matrix type
    MATRIX_MODE_TOGGLE_PARAM,
    // Switch to start shift of matrix (to the right)
    MATRIX_HOLD_TOGGLE_PARAM,
    MATRIX_ONE_STEP_RIGHT_PARAM,
    MATRIX_ONE_STEP_LEFT_PARAM,
    CARRIER_GAIN_PARAM,
    MODULATOR_GAIN_PARAM,
    ENUMS(MUTE_MODULATOR_PARAM, NR_OF_BANDS),
    ENUMS(MOD_MATRIX_PARAM, NR_OF_BANDS * NR_OF_BANDS),
    ENUMS(ATTACK_TIME_PARAM, NR_OF_BANDS),
    ENUMS(RELEASE_TIME_PARAM, NR_OF_BANDS),
    ENUMS(LEVEL_PARAM, NR_OF_BANDS),
    ENUMS(PAN_PARAM, NR_OF_BANDS),
    PAN_WIDTH_PARAM,
    PAN_WIDTH_INCREASE_PARAM,
    PAN_WIDTH_DECREASE_PARAM,
    PAN_CENTER_PARAM,
    PAN_LEFT_PARAM,
    PAN_RIGHT_PARAM,
    LEVEL_INCREASE_PARAM,
    LEVEL_DECREASE_PARAM,
    ATTACK_TIME_INCREASE_PARAM,
    ATTACK_TIME_DECREASE_PARAM,
    RELEASE_TIME_INCREASE_PARAM,
    RELEASE_TIME_DECREASE_PARAM,
    NUM_PARAMS
  };

  enum InputIds {
    // Input signal
    CARR_INPUT,
    MOD_INPUT, 
    SHIFT_RIGHT_INPUT,
    SHIFT_LEFT_INPUT,
    NUM_INPUTS
  };

  enum OutputIds {
    // Output signal
    LEFT_OUTPUT,
    RIGHT_OUTPUT,
    NUM_OUTPUTS
  };

  enum LightIds {
    // Bypass light.
    BYPASS_LIGHT,
    // Matrix type light (toggles when pressed)
    MATRIX_MODE_TOGGLE_LIGHT, 
    // Matrix shift indicator light
    MATRIX_HOLD_TOGGLE_LIGHT,
    // Step toggle to set shift by hand
    MATRIX_ONE_STEP_RIGHT_LIGHT,
    MATRIX_ONE_STEP_LEFT_LIGHT,
    ENUMS(MUTE_MODULATOR_LIGHT, NR_OF_BANDS),
    ENUMS(MOD_MATRIX_LIGHT, NR_OF_BANDS * NR_OF_BANDS),
    NUM_LIGHTS 
  };

  float blinkPhase = -1.0f;
  float oneStepBlinkPhase = 0.0f;

  void process(const ProcessArgs &args) override;
  void onReset() override;
  void onRandomize() override;

  float smoothing_factor,   
        ym[NR_OF_BANDS][3],  // filter taps (y = output, x = input )
        xm[3] = {0.0,  0.0,  0.0};
  float yc[NR_OF_BANDS][3],  // filter taps (y = output, x = input )
        xc[3] = {0.0,  0.0,  0.0};
  float xm_env = 0.0, 
        ym_env[NR_OF_BANDS][2];

  int freq[33] = {0, 20, 25, 32, 40, 50, 63, 80, 100, 125, 160, 200, 250,
               315, 400, 500, 630, 800, 1000, 1250, 1600, 2000, 2500,
               3150, 4000, 5000, 6300, 8000, 10000,      
               12500, 16000, 20000, 22025};
  int i = 0;
  double carr_bandwidth = INITIAL_CARR_BW_IN_SEMITONES;
  double mod_bandwidth = INITIAL_MOD_BW_IN_SEMITONES;
  double fsamp = FFSAMP;

  // Button for bypass on and off.
  dsp::SchmittTrigger bypass_button_trig;
  bool fx_bypass = false;
  // Button to toggle the filter band coupling type (4 * log).
  dsp::SchmittTrigger matrix_mode_button_trig;
  bool matrix_mode_button_pressed = false;
  // Start with linear coupling of filters.
  int matrix_mode_selector = INITIAL_MATRIX_MODE;
  int matrix_mode = matrix_mode_selector;

  // Keep track of the shift position of the matrix.
  int matrix_shift_position = 1;

  // Button to control triggering of matrix movement.
  dsp::SchmittTrigger matrix_hold_button_trig;
  bool matrix_hold_button_pressed = false;
  // Button to step matrix on step to the right.
  dsp::SchmittTrigger matrix_one_step_right_button_trig;
  bool matrix_one_step_right_button_pressed = false;
  // Button to step matrix on step to the left.
  dsp::SchmittTrigger matrix_one_step_left_button_trig;
  bool matrix_one_step_left_button_pressed = false;

  // Push Buttons to control pan width.
  dsp::SchmittTrigger pan_width_increase_button_trig;
  dsp::SchmittTrigger pan_width_decrease_button_trig;
  dsp::SchmittTrigger pan_center_button_trig;
  dsp::SchmittTrigger pan_left_button_trig;
  dsp::SchmittTrigger pan_right_button_trig;

  // Push Buttons to control level.
  dsp::SchmittTrigger level_increase_button_trig;
  dsp::SchmittTrigger level_decrease_button_trig;

  // Push Buttons to envelope attack time.
  dsp::SchmittTrigger envelope_attack_time_increase_button_trig;
  dsp::SchmittTrigger envelope_attack_time_decrease_button_trig;
  
  // Push Buttons to envelope  time.
  dsp::SchmittTrigger envelope_release_time_increase_button_trig;
  dsp::SchmittTrigger envelope_release_time_decrease_button_trig;

  int wait = 1;
  int wait2 = 1;
  int wait_all_sliders = 1;
  int p_cnt[NR_OF_BANDS];
  int button_value[NR_OF_BANDS][NR_OF_BANDS];
  bool mute_modulator[NR_OF_BANDS]; 
  bool mute_modulator_old[NR_OF_BANDS]; 
  float mod_alpha1[NR_OF_BANDS];
  float mod_alpha2[NR_OF_BANDS];
  float mod_beta[NR_OF_BANDS];
  float carr_alpha1[NR_OF_BANDS];
  float carr_alpha2[NR_OF_BANDS];
  float carr_beta[NR_OF_BANDS];
  float left_pan[NR_OF_BANDS];
  float right_pan[NR_OF_BANDS];
  float left_level[NR_OF_BANDS]; // output
  float right_level[NR_OF_BANDS]; // output
  float slider_level[NR_OF_BANDS]; // slider level value
  float slider_pan[NR_OF_BANDS]; // slider pan value
  float envelope_attack_time[NR_OF_BANDS];
  float envelope_attack_time_lower_range[NR_OF_BANDS];
  float envelope_attack_time_upper_range[NR_OF_BANDS];
  float envelope_release_time[NR_OF_BANDS]; 
  float envelope_release_time_lower_range[NR_OF_BANDS]; 
  float envelope_release_time_upper_range[NR_OF_BANDS]; 
  float envelope_attack_factor[NR_OF_BANDS];
  float envelope_release_factor[NR_OF_BANDS]; 
  float min_envelope_attack_time[NR_OF_BANDS];
  float max_envelope_attack_time[NR_OF_BANDS];
  float min_envelope_release_time[NR_OF_BANDS];
  float max_envelope_release_time[NR_OF_BANDS];
  float width = 1.0;
  float width_old = width;
  bool led_state[NR_OF_BANDS * NR_OF_BANDS] = {};
  bool mute_modulator_led_state[NR_OF_BANDS] = {};
  bool matrix_mode_read_from_settings = false;
  int lights_offset = MOD_MATRIX_LIGHT;
  int mute_modulator_lights_offset = MUTE_MODULATOR_LIGHT;

  int  button_left_clicked_val = 0;
  int  button_right_clicked_val = 0;
  bool right_click_state = false;

  // Sliders.
  SliderWithId *release_time_slider[NR_OF_BANDS]; 
  SliderWithId *attack_time_slider[NR_OF_BANDS]; 
  SliderWithId *pan_slider[NR_OF_BANDS]; 
  SliderWithId *level_slider[NR_OF_BANDS]; 

  // Some code to read/save state of bypass button.
  json_t *dataToJson() override {
    json_t *rootJm = json_object();

    // Store bypass setting.
    json_t *bypassJ = json_boolean(fx_bypass);
    json_object_set_new(rootJm, "fx_bypass", bypassJ);

    // Store setting of matrix_shift_position.
    json_t *matrix_shift_positionJ = json_real(matrix_shift_position);
    json_object_set_new(rootJm, "matrix_shift_position", matrix_shift_positionJ);

    // Store setting of matrix_mode.
    json_t *matrix_modeJ = json_real(matrix_mode);
    json_object_set_new(rootJm, "matrix_mode", matrix_modeJ);
    
    // Store matrix hold button status.
    json_t *matrix_hold_button_pressedJ = json_boolean(matrix_hold_button_pressed);
    json_object_set_new(rootJm, "matrix_hold_button_pressed", matrix_hold_button_pressedJ);

    // Store p_cnt.
    json_t *p_cntJ = json_array();
    for (int i = 0; i < NR_OF_BANDS; i++) {
       json_array_append_new(p_cntJ, json_real(p_cnt[i]));
    }
    json_object_set_new(rootJm, "p_cnt", p_cntJ);

    // Store matrix button values to patch settings.
    int cnt = 0;
   	json_t *button_valuesJ = json_array();
	for (int i = 0; i < NR_OF_BANDS; i++) {
		for (int j = 0; j < p_cnt[i]; j++) {
			json_array_append_new(button_valuesJ, json_real(button_value[i][j]));
            cnt++;
		}
    }
	json_object_set_new(rootJm, "button_values", button_valuesJ);

    // Store mute_modulator button values.
    json_t *mute_modulatorJ = json_array();
    for (int i = 0; i < NR_OF_BANDS; i++) {
       json_array_append_new(mute_modulatorJ, json_boolean(mute_modulator[i]));
    }
    json_object_set_new(rootJm, "mute_modulator", mute_modulatorJ);

    // Store envelope release time slider values.
    json_t *envelope_release_timeJ = json_array();
    for (int i = 0; i < NR_OF_BANDS; i++) {
       json_array_append_new(envelope_release_timeJ, json_real(envelope_release_time[i]));
    }
    json_object_set_new(rootJm, "envelope_", envelope_release_timeJ);

    // Store envelope attack time slider values.
    json_t *envelope_attack_timeJ = json_array();
    for (int i = 0; i < NR_OF_BANDS; i++) {
       json_array_append_new(envelope_attack_timeJ, json_real(envelope_attack_time[i]));
    }
    json_object_set_new(rootJm, "envelope_attack", envelope_attack_timeJ);

    // Store level slider values.
    json_t *levelJ = json_array();
    for (int i = 0; i < NR_OF_BANDS; i++) {
       json_array_append_new(levelJ, json_real(slider_level[i]));
    }
    json_object_set_new(rootJm, "level", levelJ);

    // Store pan slider values.
    json_t *panJ = json_array();
    for (int i = 0; i < NR_OF_BANDS; i++) {
       json_array_append_new(panJ, json_real(slider_pan[i]));
    }
    json_object_set_new(rootJm, "pan", panJ);
    return rootJm;
  }       
          
  void dataFromJson(json_t *rootJm) override {
    // Restore bypass state.
    json_t *bypassJ = json_object_get(rootJm, "fx_bypass");
    if (bypassJ) {
        fx_bypass = json_boolean_value(bypassJ);
    }

    // Restore matrix shift position.
    json_t *matrix_shift_positionJ = json_object_get(rootJm, "matrix_shift_position");
    if (matrix_shift_positionJ) {
        matrix_shift_position = (int) json_number_value(matrix_shift_positionJ);
    }

    // Restore matrix type.
    json_t *matrix_modeJ = json_object_get(rootJm, "matrix_mode");
    if (matrix_modeJ) {
        matrix_mode = (int) json_number_value(matrix_modeJ);
    }

    // Restore matrix_hold_button_pressed button status.
    json_t *matrix_hold_button_pressedJ = json_object_get(rootJm, "matrix_hold_button_pressed");
    if (matrix_hold_button_pressedJ) {
        matrix_hold_button_pressed = json_boolean_value(matrix_hold_button_pressedJ);
    }

    // Restore p_cnt.
    json_t *p_cntJ = json_object_get(rootJm, "p_cnt");
    if (p_cntJ) {
        for (int i = 0; i < NR_OF_BANDS; i++) {
            json_t *elementJ = json_array_get(p_cntJ, i);
            if (elementJ) {
                p_cnt[i] = (int) json_number_value(elementJ);
            }
        }
    }

    // Restore button_values.
    int cnt = 0;
    json_t *button_valuesJ = json_object_get(rootJm, "button_values");
    if (button_valuesJ) {
        int index = 0;
        for (int i = 0; i < NR_OF_BANDS; i++) {
            for (int j = 0; j < p_cnt[i]; j++) {
                json_t *elementJ = json_array_get(button_valuesJ, index + j);
                if (elementJ) {
                    button_value[i][j] = (int) json_number_value(elementJ);
                    cnt++;
                } else {
                }
            }
            index += p_cnt[i];
        }
        matrix_mode_read_from_settings = true;
        refresh_led_matrix(lights_offset, p_cnt, button_value, led_state);
    } 

    // Restore mute_modulator.
    json_t *mute_modulatorJ = json_object_get(rootJm, "mute_modulator");
    if (mute_modulatorJ) {
        for (int i = 0; i < NR_OF_BANDS; i++) {
            json_t *elementJ = json_array_get(mute_modulatorJ, i);
            if (elementJ) {
                mute_modulator[i] = json_boolean_value(elementJ);
            }
        }
        refresh_mute_modulator_leds(mute_modulator_lights_offset, mute_modulator);
    }

    // Restore envelope attack time slider.
    json_t *envelope_attack_timeJ = json_object_get(rootJm, "envelope_attack_time");
    if (envelope_attack_timeJ) {
        for (int i = 0; i < NR_OF_BANDS; i++) {
            json_t *elementJ = json_array_get(envelope_attack_timeJ, i);
            if (elementJ) {
                envelope_attack_time[i] = json_boolean_value(elementJ);
            }
        }
    }

    // Restore envelope release time slider.
    json_t *envelope_release_timeJ = json_object_get(rootJm, "envelope_release_time");
    if (envelope_release_timeJ) {
        for (int i = 0; i < NR_OF_BANDS; i++) {
            json_t *elementJ = json_array_get(envelope_release_timeJ, i);
            if (elementJ) {
                envelope_release_time[i] = json_boolean_value(elementJ);
            }
        }
    }

    // Restore level slider.
    json_t *levelJ = json_object_get(rootJm, "level");
    if (levelJ) {
        for (int i = 0; i < NR_OF_BANDS; i++) {
            json_t *elementJ = json_array_get(levelJ, i);
            if (elementJ) {
                slider_level[i] = json_boolean_value(elementJ);
            }
        }
    }

    // Restore envelope pan slider.
    json_t *panJ = json_object_get(rootJm, "pan");
    if (panJ) {
        for (int i = 0; i < NR_OF_BANDS; i++) {
            json_t *elementJ = json_array_get(panJ, i);
            if (elementJ) {
                slider_pan[i] = json_boolean_value(elementJ);
            }
        }
    }

  } 
  // Increase / decrease envelope attack / release times in small steps.
  void increase_attack_time() {
    for (int i = 0; i < NR_OF_BANDS; i++) {
        float f_c = ((float) freq[i+1] + (float) freq[i]) / 2.0;
        float increment = ((2.0 * PI) / f_c) * LOWER_ENVELOPE_ATTACK_TEMPERATURE;
        envelope_attack_time[i] += increment;
        if (envelope_attack_time[i] > envelope_attack_time_upper_range[i]) 
            envelope_attack_time[i] = envelope_attack_time_upper_range[i];
        params[ATTACK_TIME_PARAM + i].value = envelope_attack_time[i];
    }
    comp_attack_factors(envelope_attack_factor, envelope_attack_time);
  }

  void decrease_attack_time() {
    for (int i = 0; i < NR_OF_BANDS; i++) {
        float f_c = ((float) freq[i+1] + (float) freq[i]) / 2.0;
        float increment = ((2.0 * PI) / f_c) * LOWER_ENVELOPE_ATTACK_TEMPERATURE;
        envelope_attack_time[i] -= increment;
        if (envelope_attack_time[i] < envelope_attack_time_lower_range[i]) 
            envelope_attack_time[i] = envelope_attack_time_lower_range[i];
        params[ATTACK_TIME_PARAM + i].value = envelope_attack_time[i];
    }
    comp_attack_factors(envelope_attack_factor, envelope_attack_time);
  }

  void increase_release_time() {
    for (int i = 0; i < NR_OF_BANDS; i++) {
        float f_c = ((float) freq[i+1] + (float) freq[i]) / 2.0;
        float increment = ((2.0 * PI) / f_c) * LOWER_ENVELOPE_RELEASE_TEMPERATURE;
        envelope_release_time[i] += increment;
        if (envelope_release_time[i] > envelope_release_time_upper_range[i]) 
            envelope_release_time[i] = envelope_release_time_upper_range[i];
        params[RELEASE_TIME_PARAM + i].value = envelope_release_time[i];
    }
    comp_release_factors(envelope_release_factor, envelope_release_time);
  }

  void decrease_release_time() {
    for (int i = 0; i < NR_OF_BANDS; i++) {
        float f_c = ((float) freq[i+1] + (float) freq[i]) / 2.0;
        float increment = ((2.0 * PI) / f_c) * LOWER_ENVELOPE_RELEASE_TEMPERATURE;
        envelope_release_time[i] -= increment;
        if (envelope_release_time[i] < envelope_release_time_lower_range[i]) 
            envelope_release_time[i] = envelope_release_time_lower_range[i];
        params[RELEASE_TIME_PARAM + i].value = envelope_release_time[i];
    }
    comp_release_factors(envelope_release_factor, envelope_release_time);
  }

  Vocode_O_Matic_XL() {
    // Define parameters and initialize them.
    char message[255];
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(Vocode_O_Matic_XL::CARRIER_GAIN_PARAM, MIN_CARRIER_GAIN, MAX_CARRIER_GAIN, INITIAL_CARRIER_GAIN, "Gain factor for carrier signal (default=1).", "");
    configParam(Vocode_O_Matic_XL::MODULATOR_GAIN_PARAM, MIN_MODULATOR_GAIN, MAX_MODULATOR_GAIN, INITIAL_MODULATOR_GAIN, "Gain factor for modulator signal (default=1)", "");
    configParam(Vocode_O_Matic_XL::BYPASS_SWITCH , 0.0f, 1.0f, 0.0f, "Bypass vocoder and play carrier on left and modulator on right channel.", "");
    configParam(Vocode_O_Matic_XL::MATRIX_MODE_TOGGLE_PARAM, 0.0f, 1.0f, 0.0f, "Toggle through all matrix modes.", "");
    configParam(Vocode_O_Matic_XL::MATRIX_ONE_STEP_RIGHT_PARAM, 0.0f, 1.0f, 0.0f, "Move matrix one step to the right.", "");
    configParam(Vocode_O_Matic_XL::MATRIX_ONE_STEP_LEFT_PARAM, 0.0f, 1.0f, 0.0f, "Move matrix one step to the left.", "");
    configParam(Vocode_O_Matic_XL::MATRIX_HOLD_TOGGLE_PARAM, 0.0f, 1.0f, 0.0f, "Prevent the matrix from shifting.", "");
    for (int offset = 0; offset < NR_OF_BANDS; offset++) {
      sprintf(message, "Mute %d Hz band.", freq[offset + 1]);
      configParam(Vocode_O_Matic_XL::MUTE_MODULATOR_PARAM + offset, 0.0, 1.0f, 0.0f, message, "");
    }

    // Add tooltips to the buttons.
    for (int i = 0; i < NR_OF_BANDS; i++) {
        for (int j = 0; j < NR_OF_BANDS; j++) {
            sprintf(message, "Modulator %d Hz -> Carrier %d Hz.", freq[j + 1], freq[i + 1]);
            configParam(Vocode_O_Matic_XL::MOD_MATRIX_PARAM + i + j * NR_OF_BANDS, 0.0, 1.0, 0.0, message);
        }
    }

    // Init the envelope follower's variables. 
    init_attack_times(envelope_attack_time); 
    init_release_times(envelope_release_time); 
    comp_release_factors(envelope_release_factor, envelope_release_time);
    comp_attack_factors(envelope_attack_factor, envelope_attack_time);
    comp_attack_and_release_time_ranges(envelope_attack_time_lower_range, envelope_attack_time_upper_range, envelope_release_time_lower_range, envelope_release_time_upper_range);
    for (int i = 0; i < NR_OF_BANDS; i++) {
        sprintf(message, "Attack time @ %d Hz", freq[i + 1]);
        configParam(Vocode_O_Matic_XL::ATTACK_TIME_PARAM + i, envelope_attack_time_lower_range[i], envelope_attack_time_upper_range[i], envelope_attack_time[i], message, " s");
        sprintf(message, "Release time @ %d Hz", freq[i + 1]);
        configParam(Vocode_O_Matic_XL::RELEASE_TIME_PARAM + i, envelope_release_time_lower_range[i], envelope_release_time_upper_range[i], envelope_release_time[i], message, " s");
    }

    // Initialize the filter coefficients.
    comp_all_coeffs(freq, mod_bandwidth, fsamp, mod_alpha1, mod_alpha2, mod_beta);
    comp_all_coeffs(freq, carr_bandwidth, fsamp, carr_alpha1, carr_alpha2, carr_beta);
  
    // Initialize all filter taps. 
    for (int i = 0; i < NR_OF_BANDS; i++) {
        for (int j = 0; j < 3; j++) { 
            ym[i][j] = 0.0;
            yc[i][j] = 0.0;
        }
        ym_env[i][0] = 0.0; // Envelope of modulator.
        ym_env[i][1] = 0.0;
    }
    // Initialize the levels and pans.
    initialize_slider_levels(slider_level);
    init_pan_and_level(slider_level, left_pan, right_pan, left_level, right_level);
    // Now set the sliders to the initial values.
    for (int i = 0; i < NR_OF_BANDS; i++) {
        sprintf(message, "Level @ %d Hz", freq[i + 1]);
        float minimum_equal_loudness_level = min_equal_loudness_value();
        float max_level = INITIAL_LEVEL + MAX_LEVEL * (equal_loudness_value(i) / minimum_equal_loudness_level);
        float min_level = INITIAL_LEVEL + MIN_LEVEL * (equal_loudness_value(i) / minimum_equal_loudness_level);
        configParam(Vocode_O_Matic_XL::LEVEL_PARAM + i, min_level, max_level, slider_level[i], message, " dB");
        sprintf(message, "Pan @ %d Hz", freq[i + 1]);
        if ((i % 2) != 0) {
            configParam(Vocode_O_Matic_XL::PAN_PARAM + i, MIN_PAN, MAX_PAN, INITIAL_PAN + INITIAL_PAN_OFFSET, message, "");
        } else {
            configParam(Vocode_O_Matic_XL::PAN_PARAM + i, MIN_PAN, MAX_PAN, INITIAL_PAN - INITIAL_PAN_OFFSET, message, "");
        }
    }
    if (!matrix_mode_read_from_settings) {
        choose_matrix(4, button_value, p_cnt); // Initialize linear filter coupling.
        initialize_mute_modulator(mute_modulator);   // Initialize all mute buttons (to be not pressed).
    }

    // Show leds in button matrix.
    refresh_led_matrix(lights_offset, p_cnt, button_value, led_state);

    // Show leds in mute output column.
    refresh_mute_modulator_leds(MUTE_MODULATOR_LIGHT, mute_modulator);

    blinkPhase = -1.0f;
    // Reset lights.
    lights[MATRIX_HOLD_TOGGLE_LIGHT].setBrightness(0.0);
    lights[BYPASS_LIGHT].setBrightness(0.0);
 
  } 
};

struct LButton_XL : SvgSwitch {
  Vocode_O_Matic_XL *module;
  LButton_XL() {
    momentary = true;
    shadow->visible = false;
    addFrame(APP->window->loadSvg(asset::plugin(thePlugin, "res/L.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(thePlugin, "res/L.svg")));
  }

  void onButton(const event::Button &e) override {
    if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT && (e.mods & RACK_MOD_MASK) == 0) {
      if (module) {
        module->button_left_clicked_val = paramId;
      }
    }
    if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0) {
      if (module) {
        module->button_right_clicked_val = paramId;
      }
    }
    e.consume(this);
  }
};
