#include "../incl/structs.h"
#include "../incl/macros.h"
#include "../incl/visualizer.h"
#include <cmath> 


Matrix  projection;



Vect3 Normalize(Vect3 v) {
    double len = sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
    return (Vect3){v.x/len, v.y/len, v.z/len};
}

Vect3 Cross(Vect3 a, Vect3 b) {
    return (Vect3){a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x};
}

double Dot(Vect3 a, Vect3 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

void Update_Camera(Vect3 eye, Vect3 target, Vect3 up)
{
    // Forward (camera direction)
    Vect3 f = Normalize((Vect3){
        target.x - eye.x,
        target.y - eye.y,
        target.z - eye.z
    });

    // Right (correct order!)
    Vect3 r = Normalize(Cross(f, up));

    // True up
    Vect3 u = Cross(r, f);

    bzero(projection.mat, projection.m * projection.n * sizeof(double));

    // Row-major view matrix
    projection.mat[0][0] = r.x;
    projection.mat[0][1] = r.y;
    projection.mat[0][2] = r.z;
    projection.mat[0][3] = -Dot(r, eye);

    projection.mat[1][0] = u.x;
    projection.mat[1][1] = u.y;
    projection.mat[1][2] = u.z;
    projection.mat[1][3] = -Dot(u, eye);

    projection.mat[2][0] = -f.x;
    projection.mat[2][1] = -f.y;
    projection.mat[2][2] = -f.z;
    projection.mat[2][3] = Dot(f, eye);

    projection.mat[3][0] = 0.0;
    projection.mat[3][1] = 0.0;
    projection.mat[3][2] = 0.0;
    projection.mat[3][3] = 1.0;
}


void Init_Matrix(){
    // Ensure FOV is handled correctly (Field of projection factor)
    double field = 1.0 / tan(ToRad(FOV / 2.0));
    double range = FAR - NEAR;

    // Zero out the matrix first
    bzero(projection.mat, projection.m * projection.n * sizeof(double));

    // Standard Perspective Projection
    projection.mat[0][0] = field / A_RATIO; // Divide by Aspect Ratio
    projection.mat[1][1] = field;
    projection.mat[2][2] = FAR / range;
    projection.mat[3][2] = (-FAR * NEAR) / range;
    projection.mat[2][3] = 1.0; 
    projection.mat[3][3] = 0.0; // Ensure this is 0
}

// void Init_Matrix(){
//     double field = 1.0f / SDL_tanf(ToRad(FOV / 2));

//     bzero(projection.mat, projection.m * projection.n * sizeof(double));
//     projection.mat[0][0] = A_RATIO * field;
//     projection.mat[1][1] = field;
//     projection.mat[2][2] =  FAR / (FAR - NEAR);
//     projection.mat[2][3] =  1;
//     projection.mat[3][2] = (-FAR * NEAR)/ (FAR - NEAR);
// }

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