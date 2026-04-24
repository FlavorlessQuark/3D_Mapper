#pragma once

#include "structs.h"

void            SDLX_InitDefault();
SDLX_Display	*SDLX_DisplayGet(void);
void Init_Matrix();
void project(Vect3 *in, Vect2 *out, int count);
void InputLoop(float dt);

// Utils
double  VectAbs(Vect3 a);
double  VectDot(Vect3 a, Vect3 b, double *angle);
double  VectMag(Vect3 vec);
Vect3    VectSub(Vect3 a, Vect3 b);
Vect3    VectScale(Vect3 a, double scalar);
Vect3    VectNormalize(Vect3 vec);
Vect3   MatVec_Mult(Vect3 vec, Matrix mat);
double	ToRad(double angle);
void Update_Camera(Vect3 eye, Vect3 target, Vect3 up);