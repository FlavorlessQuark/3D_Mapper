#include "../incl/structs.h"
#include "../incl/macros.h"
#include <cmath> 


Matrix  projection;


double	ToRad(double angle)
{
    double result;
    
	result = angle * M_PI;
	result /= 180;
    
	return result;
}

void Init_Matrix(){
    double field = 1.0f / SDL_tanf(ToRad(FOV / 2));

    bzero(projection.mat, projection.m * projection.n * sizeof(double));
    projection.mat[0][0] = A_RATIO * field;
    projection.mat[1][1] = field;
    projection.mat[2][2] =  FAR / (FAR - NEAR);
    projection.mat[2][3] =  1;
    projection.mat[3][2] = (-FAR * NEAR)/ (FAR - NEAR);
}

Vect3 Mat_Mult(Vect3 vec, Matrix mat) {
    Vect3 out;

    out.x = vec.x * mat.mat[0][0] + vec.y * mat.mat[1][0] + vec.z * mat.mat[2][0] + mat.mat[3][0];
    out.y = vec.x * mat.mat[0][1] + vec.y * mat.mat[1][1] + vec.z * mat.mat[2][1] + mat.mat[3][1];
    out.z = vec.x * mat.mat[0][2] + vec.y * mat.mat[1][2] + vec.z * mat.mat[2][2] + mat.mat[3][2];
    
    double w = vec.x * mat.mat[0][3] + vec.y * mat.mat[1][3] + vec.z * mat.mat[2][3] + mat.mat[3][3];

    if (w != 0.0f) {
        out.x /= w;
        out.y /= w;
        out.z /= w;
    }

    return out;
}

Vect3 Polar_to_Cartesian(Vect3_Polar p) {
   Vect3 cartesian;

   cartesian.x = p.r * SDL_sinf(p.angle_y) * SDL_cosf(p.angle_z);
   cartesian.y = p.r * SDL_sinf(p.angle_y) * SDL_sinf(p.angle_z);
   cartesian.z = p.r * SDL_cosf(p.angle_y);

    return cartesian;
}

// void project(Vect3_Polar *in, Vect2 *out, int count) {
//     for (int i = 0; i < count; i++) {
//         Vect3 proj_result = Mat_Mult(Polar_to_Cartesian(in[i]), projection);
//         out[i] = {.x = proj_result.x, .y = proj_result.y};
//     }
// }

void project(Vect3 *in, Vect2 *out, int count) {
    for (int i = 0; i < count; i++) {
        Vect3 proj_result = Mat_Mult(in[i], projection);
        out[i] = {.x = proj_result.x, .y = proj_result.y};
    }
}