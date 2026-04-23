#pragma once

#include "structs.h"

void            SDLX_InitDefault();
SDLX_Display	*SDLX_DisplayGet(void);
void Init_Matrix();
void project(Vect3 *in, Vect2 *out, int count);
void InputLoop(void);

// Utils
double  VectAbs(Vec3 a);
double  VectDot(Vec3 a, Vec3 b, double *angle);
double  VectMag(Vec3 vec);
Vec3    VectSub(Vec3 a, Vec3 b);
Vec3    VectScale(Vec3 a, double scalar);
Vec3    VectNormalize(Vec3 vec);
Vect3   MatVec_Mult(Vect3 vec, Matrix mat)