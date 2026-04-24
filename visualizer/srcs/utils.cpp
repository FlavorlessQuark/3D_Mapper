#include "../incl/structs.h"

double	ToRad(double angle)
{
    double result;
    
	result = angle * M_PI;
	result /= 180;
    
	return result;
}

double VectMag(Vect3 vec) {
	return sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
}


Vect3 VectSub(Vect3 a, Vect3 b)
{
	Vect3 vec;

	vec.x = a.x - b.x; 
	vec.y = a.y - b.y; 
	vec.z = a.z - b.z;

	return vec;
}

Vect3 VectAdd(Vect3 a, Vect3 b)
{
	Vect3 vec;

	vec.x = a.x + b.x;
	vec.y = a.y + b.y;
	vec.z = a.z + b.z;

	return vec;
}

Vect3 VectScale(Vect3 a, double scalar)
{
	Vect3 vec;

	vec.x = a.x * scalar;
	vec.y = a.y * scalar;
	vec.z = a.z * scalar;

	return vec;
}

double VectAbs(Vect3 a)
{
	return sqrt((a.x * a.x) + (a.y * a.y) + (a.z * a.z));
}

double VectDot(Vect3 a, Vect3 b, double *angle)
{
	if (angle != NULL)
		return VectMag(a) * VectMag(a) * cos(*angle);
	return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

Vect3 VectFromAngle_Deg(double x_angle, double y_angle, double magnitude)
{
	Vect3 result;

	result.x = cos(ToRad(y_angle)) * sin(ToRad(x_angle)) * magnitude;
	result.y = sin(ToRad(y_angle)) * magnitude;
	result.z = cos(ToRad(y_angle)) * cos(ToRad(x_angle)) * magnitude;

	return result;
}

Vect3 VectNormalize(Vect3 vec)
{
	Vect3 result;
	double mag;

	mag = VectMag(vec);

	result.x = vec.x / mag;
	result.y = vec.y / mag;
	result.z = vec.z / mag;

	return result;
}

Vect3 MatVec_Mult(Vect3 vec, Matrix mat) {
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
