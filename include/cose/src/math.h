#ifndef CE_MATH_H
#define CE_MATH_H

#define CE_AlignPow2(n,a) (((n)+((a-1)))&(~(a-1)))
#define CE_Max(a, b) ((a) > (b) ? (a) : (b))
#define CE_Min(a, b) ((a) < (b) ? (a) : (b))

// Linear interpolation
CE_f64 CE_Lerp(CE_f64 a, CE_f64 b, CE_f64 t);

#endif
