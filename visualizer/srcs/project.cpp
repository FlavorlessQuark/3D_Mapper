
#include "../incl/structs.h"
#include "../incl/macros.h"
#include "../incl/visualizer.h"
#include <cmath>
#include <strings.h> // for bzero

Matrix view;
Matrix projection;

Vect3 Normalize(Vect3 v) {
    double len = sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
    if (len == 0.0)
        return (Vect3){0, 0, 0};
    return (Vect3){v.x/len, v.y/len, v.z/len};
}

Vect3 Cross(Vect3 a, Vect3 b) {
    return (Vect3){
        a.y*b.z - a.z*b.y,
        a.z*b.x - a.x*b.z,
        a.x*b.y - a.y*b.x
    };
}

double Dot(Vect3 a, Vect3 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

void Update_Camera(Vect3 eye, Vect3 target, Vect3 up)
{
    Vect3 f = Normalize((Vect3){
        target.x - eye.x,
        target.y - eye.y,
        target.z - eye.z
    });

    Vect3 r = Normalize(Cross(f, up));
    Vect3 u = Cross(r, f);

    bzero(view.mat, view.m * view.n * sizeof(double));

    // Row-major view matrix
    view.mat[0][0] = r.x;
    view.mat[0][1] = r.y;
    view.mat[0][2] = r.z;
    view.mat[0][3] = -Dot(r, eye);

    view.mat[1][0] = u.x;
    view.mat[1][1] = u.y;
    view.mat[1][2] = u.z;
    view.mat[1][3] = -Dot(u, eye);

    view.mat[2][0] = -f.x;
    view.mat[2][1] = -f.y;
    view.mat[2][2] = -f.z;
    view.mat[2][3] = -Dot(f, eye); // FIXED SIGN

    view.mat[3][0] = 0.0;
    view.mat[3][1] = 0.0;
    view.mat[3][2] = 0.0;
    view.mat[3][3] = 1.0;
}

void Init_Matrix()
{
    double f = 1.0 / tan(ToRad(FOV / 2.0));

    bzero(projection.mat, sizeof(projection.mat));

    projection.mat[0][0] = f * A_RATIO;
    projection.mat[1][1] = f;

    projection.mat[2][2] = (FAR + NEAR) / (NEAR - FAR);
    projection.mat[2][3] = (2 * FAR * NEAR) / (NEAR - FAR);

    projection.mat[3][2] = -1.0;
    projection.mat[3][3] = 0.0;
}

Vect3 Mat_Mult(Vect3 vec, Matrix mat) {
    Vect3 out;

    // Row-major * column vector
    out.x = vec.x * mat.mat[0][0] + vec.y * mat.mat[0][1] + vec.z * mat.mat[0][2] + mat.mat[0][3];
    out.y = vec.x * mat.mat[1][0] + vec.y * mat.mat[1][1] + vec.z * mat.mat[1][2] + mat.mat[1][3];
    out.z = vec.x * mat.mat[2][0] + vec.y * mat.mat[2][1] + vec.z * mat.mat[2][2] + mat.mat[2][3];

    double w = vec.x * mat.mat[3][0] + vec.y * mat.mat[3][1] + vec.z * mat.mat[3][2] + mat.mat[3][3];

    if (w != 0.0) {
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

Matrix Mat4_Mul(Matrix a, Matrix b) {
    Matrix result;

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result.mat[i][j] =
                a.mat[i][0] * b.mat[0][j] +
                a.mat[i][1] * b.mat[1][j] +
                a.mat[i][2] * b.mat[2][j] +
                a.mat[i][3] * b.mat[3][j];
        }
    }

    return result;
}

void project(Vect3 *in, Vect2 *out, int count) {
    // Correct order: projection * view
    Matrix vp = Mat4_Mul(projection, view);

    for (int i = 0; i < count; i++) {
        Vect3 proj_result = Mat_Mult(in[i], vp);
        out[i] = (Vect2){proj_result.x, proj_result.y};
    }
}