#include "std.hpp"
#include <stdio.h>
#include "matrix.hpp"

void initialize_start_levels(float start_level[NR_OF_BANDS]) {
  for (int i = 0; i < NR_OF_BANDS; i++) { start_level[i] = INITIAL_START_LEVEL; }
}


void initialize_matrix(int button_value[NR_OF_BANDS][NR_OF_BANDS], int p_cnt[NR_OF_BANDS])
{ 
  // clear matrix
  for (int i = 0; i < NR_OF_BANDS; i++)     
  {                        
    p_cnt[i] = 0;  
    for (int j = 0; j < NR_OF_BANDS; j++)   
    {                      
       button_value[i][j] = 0;
    }                      
  } 
  // Prepare linear filter mapping
  for (int i = 0; i < NR_OF_BANDS; i++)
  {
    p_cnt[i] = 0;
    button_value[i][p_cnt[i]++]=i;
  }
}

void print_matrix(int button_value[NR_OF_BANDS][NR_OF_BANDS]) 
{
  for (int i = 0; i < NR_OF_BANDS; i++)     
  {                        
    for (int j = 0; j < NR_OF_BANDS; j++)   
    {                      
       printf("%d ", button_value[i][j]);
    }                      
    printf("\n"); 
  } 
}

void print_p_cnt(int p_cnt[NR_OF_BANDS]) 
{
  for (int i = 0; i < NR_OF_BANDS; i++) {
    printf("%d ", p_cnt[i]);
  }
  printf(" <--\n");
}

void refresh_matrix(int button_value[NR_OF_BANDS][NR_OF_BANDS], int p_cnt[NR_OF_BANDS], int buttons[NR_OF_BANDS][NR_OF_BANDS])
{
  for (int i = 0; i < NR_OF_BANDS; i++)
  {
    p_cnt[i] = 0;
    for (int j = 0; j < NR_OF_BANDS; j++)
       button_value[i][j] = 0;
  }
  for (int i = 0; i < NR_OF_BANDS; i++)
  {
    for (int j = 0; j < NR_OF_BANDS; j++)
    {
      int set = buttons[j][i];
      if (set == PRESSED) {
          button_value[i][p_cnt[i]++]=j;
      }
    }
  }
}

// implement linear, inverse and 4 log filter bindings.

void choose_matrix(int filter_coupling_type, int button_value[NR_OF_BANDS][NR_OF_BANDS], int p_cnt[NR_OF_BANDS]) 
{
  int iindex[] = {
      0, 5,  9, 12, 14, 16, 17, 18, 19, 20, 21, 22, 23, 23, 24, 25, 25, 26,
      26, 27, 27, 28, 28, 28, 29, 29, 29, 30, 30, 30, 30};

  int buttons[NR_OF_BANDS][NR_OF_BANDS]; // temporary structure for matrix layout
  for (int i = 0; i < NR_OF_BANDS; i++) {
    p_cnt[i] = 0;
    for (int j = 0; j < NR_OF_BANDS; j++) {
      buttons[i][j] = NOT_PRESSED;
    }
  }

  switch(filter_coupling_type) {
      case 0: // log 1
        for (int i = 0; i < NR_OF_BANDS; i++)
        {
          buttons[i][iindex[i]] = PRESSED;
        }
        buttons[0][1] = PRESSED;
        buttons[0][2] = PRESSED;
        buttons[0][3] = PRESSED;
        buttons[1][4] = PRESSED;
        buttons[1][6] = PRESSED;
        buttons[1][7] = PRESSED;
        buttons[2][8] = PRESSED;
        buttons[3][10] = PRESSED;
        buttons[3][11] = PRESSED;
        buttons[4][13] = PRESSED;
        buttons[5][15] = PRESSED;
        break;
      case 1: // log 2
        for (int i = 0; i < NR_OF_BANDS; i++)
        {
          buttons[NR_OF_BANDS-1-i][iindex[i]] = PRESSED;
        }
        buttons[NR_OF_BANDS-1-0][1] = PRESSED;
        buttons[NR_OF_BANDS-1-0][2] = PRESSED;
        buttons[NR_OF_BANDS-1-0][3] = PRESSED;
        buttons[NR_OF_BANDS-1-1][4] = PRESSED;
        buttons[NR_OF_BANDS-1-1][6] = PRESSED;
        buttons[NR_OF_BANDS-1-1][7] = PRESSED;
        buttons[NR_OF_BANDS-1-2][8] = PRESSED;
        buttons[NR_OF_BANDS-1-2][9] = PRESSED;
        buttons[NR_OF_BANDS-1-3][10] = PRESSED;
        buttons[NR_OF_BANDS-1-3][11] = PRESSED;
        buttons[NR_OF_BANDS-1-4][13] = PRESSED;
        buttons[NR_OF_BANDS-1-5][15] = PRESSED;
        break;
      case 2: // log 3
        for (int i = 0; i < NR_OF_BANDS; i++)
        {
          buttons[NR_OF_BANDS-1-i][NR_OF_BANDS-1-iindex[i]] = PRESSED;
        }
        buttons[NR_OF_BANDS-1-0][NR_OF_BANDS-1-1] = PRESSED;
        buttons[NR_OF_BANDS-1-0][NR_OF_BANDS-1-2] = PRESSED;
        buttons[NR_OF_BANDS-1-0][NR_OF_BANDS-1-3] = PRESSED;
        buttons[NR_OF_BANDS-1-1][NR_OF_BANDS-1-4] = PRESSED;
        buttons[NR_OF_BANDS-1-1][NR_OF_BANDS-1-6] = PRESSED;
        buttons[NR_OF_BANDS-1-1][NR_OF_BANDS-1-7] = PRESSED;
        buttons[NR_OF_BANDS-1-2][NR_OF_BANDS-1-8] = PRESSED;
        buttons[NR_OF_BANDS-1-2][NR_OF_BANDS-1-9] = PRESSED;
        buttons[NR_OF_BANDS-1-3][NR_OF_BANDS-1-10] = PRESSED;
        buttons[NR_OF_BANDS-1-3][NR_OF_BANDS-1-11] = PRESSED;
        buttons[NR_OF_BANDS-1-4][NR_OF_BANDS-1-13] = PRESSED;
        buttons[NR_OF_BANDS-1-5][NR_OF_BANDS-1-15] = PRESSED;
        break;
      case 3: // log 4
        for (int i = 0; i < NR_OF_BANDS; i++)
        {
          buttons[i][NR_OF_BANDS-1-iindex[i]] = PRESSED;
        }
        buttons[0][NR_OF_BANDS-1-1] = PRESSED;
        buttons[0][NR_OF_BANDS-1-2] = PRESSED;
        buttons[0][NR_OF_BANDS-1-3] = PRESSED;
        buttons[1][NR_OF_BANDS-1-4] = PRESSED;
        buttons[1][NR_OF_BANDS-1-6] = PRESSED;
        buttons[1][NR_OF_BANDS-1-7] = PRESSED;
        buttons[2][NR_OF_BANDS-1-8] = PRESSED;
        buttons[2][NR_OF_BANDS-1-9] = PRESSED;
        buttons[3][NR_OF_BANDS-1-10] = PRESSED;
        buttons[3][NR_OF_BANDS-1-11] = PRESSED;
        buttons[4][NR_OF_BANDS-1-13] = PRESSED;
        buttons[5][NR_OF_BANDS-1-15] = PRESSED;
        break;
     case 4: // linear
        for (int i = 0; i < NR_OF_BANDS; i++)
        { 
          buttons[i][i] = PRESSED;
        }
        break;
     case 5: // inverse
        for (int i = 0; i < NR_OF_BANDS; i++)
        { 
          buttons[i][NR_OF_BANDS - 1 - i] = PRESSED;
        }
        break;
     default: // default is linear.
        for (int i = 0; i < NR_OF_BANDS; i++)
        { 
          buttons[i][i] = PRESSED;
        }
        break;
     break;
  }
#ifdef DEBUG
  print_matrix(buttons);
#endif
  refresh_matrix(button_value, p_cnt, buttons);
#ifdef DEBUG
  print_matrix(button_value);
  print_p_cnt(p_cnt);
#endif
}

void matrix_shift_buttons_to_right(int button_value[NR_OF_BANDS][NR_OF_BANDS], int p_cnt[NR_OF_BANDS])
{
  for (int i = 0; i < NR_OF_BANDS; i++)
  {
    for (int j = 0; j < p_cnt[i]; j++)
    {
       if (button_value[i][j] < NR_OF_BANDS-1)
          button_value[i][j]  += 1;
        else
          button_value[i][j]  = 0;
    }
  }
}
