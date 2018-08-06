#include "std.hpp"

#define PRESSED 1
#define NOT_PRESSED 0

#define NR_MATRIX_TYPES 6 //lin,inv,4*log
#define INITIAL_MATRIX_TYPE 4

void initialize_start_levels(float start_level[NR_OF_BANDS]);

void initialize_matrix(int button_value[NR_OF_BANDS][NR_OF_BANDS], int p_cnt[NR_OF_BANDS]);

void print_matrix(int button_value[NR_OF_BANDS][NR_OF_BANDS]);

void print_p_cnt(int p_cnt[NR_OF_BANDS]);

void choose_matrix(int filter_coupling_type, int button_value[NR_OF_BANDS][NR_OF_BANDS], int p_cnt[NR_OF_BANDS]);

void matrix_shift_buttons_to_right(int button_value[NR_OF_BANDS][NR_OF_BANDS], int p_cnt[NR_OF_BANDS]);
