#ifndef _STD_H
#define _STD_H
float fl_abs(float a)
{
  if (a < 0) return(-a);
  return(a);
}

float fl_max(float a, float b)
{
   if (a > b) return(a);
   return(b);
}

float fl_min(float a, float b)
{
   if (a < b) return(a);
   return(b);
}
#endif
