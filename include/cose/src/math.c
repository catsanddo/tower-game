#include "math.h"

CE_f64 CE_Lerp(CE_f64 a, CE_f64 b, CE_f64 t)
{
    return a + ((b - a) * t);
}
