#include "std.hpp"
#include "../deps/SynthDevKit/src/CV.hpp"
#include "dsp/digital.hpp"
#include "Sculpt-O-Sound.hpp"
#include "comp_coeffs.hpp"
#include "pan_and_level.hpp"

struct Vocode_O_Matic : Module {

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

  void refresh_mute_output_leds(int offset, bool mute_output[NR_OF_BANDS])
  {
    for (int i = 0; i < NR_OF_BANDS; i++) {
        // Switch led off if mute_output is true.
        lights[offset + i].value = (mute_output[i] == true) ? 0.0: 1.0;
    }
  }

  void shift_buttons_right(int button_value[NR_OF_BANDS][NR_OF_BANDS], int p_cnt[NR_OF_BANDS], bool led_state[1024], int *matrix_shift_position) {
    matrix_shift_buttons_right(button_value, p_cnt);
#ifdef DEBUG
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
#ifdef DEBUG
    print_matrix(button_value, p_cnt);
#endif
    // Refresh the visible matrix.
    refresh_led_matrix(lights_offset, p_cnt, button_value, led_state);
    *matrix_shift_position -= 1;
    if (*matrix_shift_position < 0) 
        *matrix_shift_position = NR_OF_BANDS - 1;
  }

  void initialize_mute_output(bool mute_output[NR_OF_BANDS]) {
    for (int i = 0; i < NR_OF_BANDS; i++) {
        mute_output[i] = false;
    }
  }
 
  void print_mute_buttons(bool mute_output[NR_OF_BANDS]) {
    for (int i = 0; i <  NR_OF_BANDS - 1; i++) {
        printf("%02d: %d ", i, mute_output[i] == true ? 1: 0);
    }
    printf("%02d %d\n", NR_OF_BANDS - 1, mute_output[NR_OF_BANDS - 1] == true ? 1: 0);
  }

  enum ParamIds {
    // aplification factor
    DEBUG_PARAM,
    // bypass switch
    BYPASS_SWITCH,
    // toggle button to choose matrix type
    MATRIX_MODE_TOGGLE_PARAM,
    // switch to start shift of matrix (to the right)
    MATRIX_HOLD_TOGGLE_PARAM,
    MATRIX_ONE_STEP_RIGHT_PARAM,
    MATRIX_ONE_STEP_LEFT_PARAM,
    CARRIER_GAIN_PARAM,
    MODULATOR_GAIN_PARAM,
    PANNING_PARAM,
    MUTE_OUTPUT_PARAM_00,
    MUTE_OUTPUT_PARAM_01,
    MUTE_OUTPUT_PARAM_02,
    MUTE_OUTPUT_PARAM_03,
    MUTE_OUTPUT_PARAM_04,
    MUTE_OUTPUT_PARAM_05,
    MUTE_OUTPUT_PARAM_06,
    MUTE_OUTPUT_PARAM_07,
    MUTE_OUTPUT_PARAM_08,
    MUTE_OUTPUT_PARAM_09,
    MUTE_OUTPUT_PARAM_10,
    MUTE_OUTPUT_PARAM_11,
    MUTE_OUTPUT_PARAM_12,
    MUTE_OUTPUT_PARAM_13,
    MUTE_OUTPUT_PARAM_14,
    MUTE_OUTPUT_PARAM_15,
    MUTE_OUTPUT_PARAM_16,
    MUTE_OUTPUT_PARAM_17,
    MUTE_OUTPUT_PARAM_18,
    MUTE_OUTPUT_PARAM_19,
    MUTE_OUTPUT_PARAM_20,
    MUTE_OUTPUT_PARAM_21,
    MUTE_OUTPUT_PARAM_22,
    MUTE_OUTPUT_PARAM_23,
    MUTE_OUTPUT_PARAM_24,
    MUTE_OUTPUT_PARAM_25,
    MUTE_OUTPUT_PARAM_26,
    MUTE_OUTPUT_PARAM_27,
    MUTE_OUTPUT_PARAM_28,
    MUTE_OUTPUT_PARAM_29,
    MUTE_OUTPUT_PARAM_30,
    MOD_MATRIX_PARAM,
    NUM_PARAMS = MOD_MATRIX_PARAM + NR_OF_BANDS * NR_OF_BANDS 
  };

  enum InputIds {
    // input signal
    CARR_INPUT,
    MOD_INPUT, 
    SHIFT_RIGHT_INPUT,
    SHIFT_LEFT_INPUT,
    NUM_INPUTS
  };

  enum OutputIds {
    // output signal
    LEFT_OUTPUT,
    RIGHT_OUTPUT,
    NUM_OUTPUTS
  };

  enum LightIds {
    // bypass light.
    BYPASS_LIGHT,
    // matrix type light (toggles when pressed)
    MATRIX_MODE_TOGGLE_LIGHT, 
    // matrix shift indicator light
    MATRIX_HOLD_TOGGLE_LIGHT,
    // Step toggle to set shift by hand
    MATRIX_ONE_STEP_RIGHT_LIGHT,
    MATRIX_ONE_STEP_LEFT_LIGHT,
    MUTE_OUTPUT_LIGHT_00,
    MUTE_OUTPUT_LIGHT_01,
    MUTE_OUTPUT_LIGHT_02,
    MUTE_OUTPUT_LIGHT_03,
    MUTE_OUTPUT_LIGHT_04,
    MUTE_OUTPUT_LIGHT_05,
    MUTE_OUTPUT_LIGHT_06,
    MUTE_OUTPUT_LIGHT_07,
    MUTE_OUTPUT_LIGHT_08,
    MUTE_OUTPUT_LIGHT_09,
    MUTE_OUTPUT_LIGHT_10,
    MUTE_OUTPUT_LIGHT_11,
    MUTE_OUTPUT_LIGHT_12,
    MUTE_OUTPUT_LIGHT_13,
    MUTE_OUTPUT_LIGHT_14,
    MUTE_OUTPUT_LIGHT_15,
    MUTE_OUTPUT_LIGHT_16,
    MUTE_OUTPUT_LIGHT_17,
    MUTE_OUTPUT_LIGHT_18,
    MUTE_OUTPUT_LIGHT_19,
    MUTE_OUTPUT_LIGHT_20,
    MUTE_OUTPUT_LIGHT_21,
    MUTE_OUTPUT_LIGHT_22,
    MUTE_OUTPUT_LIGHT_23,
    MUTE_OUTPUT_LIGHT_24,
    MUTE_OUTPUT_LIGHT_25,
    MUTE_OUTPUT_LIGHT_26,
    MUTE_OUTPUT_LIGHT_27,
    MUTE_OUTPUT_LIGHT_28,
    MUTE_OUTPUT_LIGHT_29,
    MUTE_OUTPUT_LIGHT_30,
    MOD_MATRIX,
    NUM_LIGHTS = MOD_MATRIX + NR_OF_BANDS * NR_OF_BANDS
  };

  float blinkPhase = -1.0f;
  float oneStepBlinkPhase = 0.0f;

  void step() override;

  // For more advanced Module features, read Rack's engine.hpp header file
  // - toJson, fromJson: serialization of internal data
  // - onSampleRateChange: event triggered by a change of sample rate
  // - onReset, onRandomize, onCreate, onDelete: implements special behavior when user clicks these from the context menu

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
  SchmittTrigger bypass_button_trig;
  bool fx_bypass = false;
  // Button to toggle the filter band coupling type (4 * log)
  SchmittTrigger matrix_mode_button_trig;
  bool matrix_mode_button_pressed = false;
  // Start with linear coupling of filters.
  int matrix_mode_selector = INITIAL_MATRIX_MODE;
  int matrix_mode = matrix_mode_selector;

  // What is the shift position of the matrix.
  int matrix_shift_position = 1;

  SchmittTrigger matrix_hold_button_trig;
  bool matrix_hold_button_pressed = false;
  SchmittTrigger matrix_one_step_right_button_trig;
  bool matrix_one_step_right_button_pressed = false;
  SchmittTrigger matrix_one_step_left_button_trig;
  bool matrix_one_step_left_button_pressed = false;

  SchmittTrigger mute_output_trig;

  int wait = 0;
  int wait2 = 0;
  int p_cnt[NR_OF_BANDS];
  int button_value[NR_OF_BANDS][NR_OF_BANDS];
  bool mute_output[NR_OF_BANDS]; 
  float mod_alpha1[NR_OF_BANDS];
  float mod_alpha2[NR_OF_BANDS];
  float mod_beta[NR_OF_BANDS];
  float carr_alpha1[NR_OF_BANDS];
  float carr_alpha2[NR_OF_BANDS];
  float carr_beta[NR_OF_BANDS];
  float left_pan[NR_OF_BANDS];
  float right_pan[NR_OF_BANDS];
  float left_level[NR_OF_BANDS];
  float right_level[NR_OF_BANDS];
  float start_level[NR_OF_BANDS];
  float envelope_attack_time[NR_OF_BANDS], envelope_release_time[NR_OF_BANDS]; 
  float envelope_attack_factor[NR_OF_BANDS], envelope_release_factor[NR_OF_BANDS]; 
  float width = 1.0;
  float width_old = width;
  bool led_state[1024] = {};
  bool mute_output_led_state[NR_OF_BANDS] = {};
  bool matrix_mode_read_from_settings = false;
  int lights_offset = MOD_MATRIX;
  int mute_output_lights_offset = MUTE_OUTPUT_LIGHT_00;

  // Some code to read/save state of bypass button.
  json_t *toJson() override {
    json_t *rootJm = json_object();

    // Store bypass setting
    json_t *bypassJ = json_boolean(fx_bypass);
    json_object_set_new(rootJm, "fx_bypass", bypassJ);

    // Store setting of matrix_shift_position
    json_t *matrix_shift_positionJ = json_real(matrix_shift_position);
    json_object_set_new(rootJm, "matrix_shift_position", matrix_shift_positionJ);

    // Store setting of matrix_mode
    json_t *matrix_modeJ = json_real(matrix_mode);
    json_object_set_new(rootJm, "matrix_mode", matrix_modeJ);
    
    // Store matrix hold button status
    json_t *matrix_hold_button_pressedJ = json_boolean(matrix_hold_button_pressed);
    json_object_set_new(rootJm, "matrix_hold_button_pressed", matrix_hold_button_pressedJ);

    // Store p_cnt
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

    // Store mute_output button values
    json_t *mute_outputJ = json_array();
    for (int i = 0; i < NR_OF_BANDS; i++) {
       json_array_append_new(mute_outputJ, json_boolean(mute_output[i]));
    }
    json_object_set_new(rootJm, "mute_output", mute_outputJ);

    return rootJm;
  }       
          
  void fromJson(json_t *rootJm) override {


    // Restore bypass state 
    // fx_bypass = (params[BYPASS_SWITCH].value == 1) ? true: false;
    json_t *bypassJ = json_object_get(rootJm, "fx_bypass");
    if (bypassJ) {
        fx_bypass = json_boolean_value(bypassJ);
    }

    // Restore matrix shift position
    json_t *matrix_shift_positionJ = json_object_get(rootJm, "matrix_shift_position");
    if (matrix_shift_positionJ) {
        matrix_shift_position = (int) json_number_value(matrix_shift_positionJ);
    }

    // Restore matrix type 
    json_t *matrix_modeJ = json_object_get(rootJm, "matrix_mode");
    if (matrix_modeJ) {
        matrix_mode = (int) json_number_value(matrix_modeJ);
    }

    // Restore matrix_hold_button_pressed button status
    json_t *matrix_hold_button_pressedJ = json_object_get(rootJm, "matrix_hold_button_pressed");
    if (matrix_hold_button_pressedJ) {
        matrix_hold_button_pressed = json_boolean_value(matrix_hold_button_pressedJ);
    }

    // Restore p_cnt
    json_t *p_cntJ = json_object_get(rootJm, "p_cnt");
    if (p_cntJ) {
        for (int i = 0; i < NR_OF_BANDS; i++) {
            json_t *elementJ = json_array_get(p_cntJ, i);
            if (elementJ) {
                p_cnt[i] = (int) json_number_value(elementJ);
            }
        }
    }

    // Restore button_values
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

    // Restore mute_output
    json_t *mute_outputJ = json_object_get(rootJm, "mute_output");
    if (mute_outputJ) {
        for (int i = 0; i < NR_OF_BANDS; i++) {
            json_t *elementJ = json_array_get(mute_outputJ, i);
            if (elementJ) {
                mute_output[i] = json_boolean_value(elementJ);
            }
        }
        refresh_mute_output_leds(mute_output_lights_offset, mute_output);
    }

  } 

  Vocode_O_Matic() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {

    // Initialize the filter coefficients.
    comp_all_coeffs(freq, mod_bandwidth, fsamp, mod_alpha1, mod_alpha2, mod_beta);
    comp_all_coeffs(freq, carr_bandwidth, fsamp, carr_alpha1, carr_alpha2, carr_beta);
  
    // Initialize all filter taps. 
    for (int i = 0; i < NR_OF_BANDS; i++) {
     for (int j = 0; j < 3; j++) { 
          ym[i][j] = 0.0;
      }
      ym_env[i][0] = 0.0; // Envelope of modulator.
      ym_env[i][1] = 0.0;
    }
    // Initialize the levels and pans.
    initialize_levels(start_level);
    init_pan_and_level(start_level, left_pan, right_pan, left_level, right_level);
    if (!matrix_mode_read_from_settings) {
        choose_matrix(4, button_value, p_cnt); // Initialize linear filter coupling.
        initialize_mute_output(mute_output); // initialize all mute buttons (to be not pressed).
    }

    // Show leds in button matrix.
    refresh_led_matrix(lights_offset, p_cnt, button_value, led_state);

    // Show leds in mute output column.
    refresh_mute_output_leds(MUTE_OUTPUT_LIGHT_00, mute_output);


    blinkPhase = -1.0f;
    // Reset lights.
    lights[MATRIX_HOLD_TOGGLE_LIGHT].value = 0.0;
    lights[BYPASS_LIGHT].value = 0.0;
  
    init_attack_times(envelope_attack_time); 
    comp_attack_factors(envelope_attack_factor, envelope_attack_time);
  
    init_release_times(envelope_release_time); 
    comp_release_factors(envelope_release_factor, envelope_release_time);
  } 
};
