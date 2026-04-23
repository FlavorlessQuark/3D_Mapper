#include "../incl/structs.h"

Vec3 VectSub(Vec3 a, Vec3 b)
{
	Vec3 vec;

	vec.x = a.x - b.x; 
	vec.y = a.y - b.y; 
	vec.z = a.z - b.z;

	return vec;
}

Vec3 VectAdd(Vec3 a, Vec3 b)
{
	Vec3 vec;

	vec.x = a.x + b.x;
	vec.y = a.y + b.y;
	vec.z = a.z + b.z;

	return vec;
}

Vec3 VectScale(Vec3 a, double scalar)
{
	Vec3 vec;

	vec.x = a.x * scalar;
	vec.y = a.y * scalar;
	vec.z = a.z * scalar;

	return vec;
}

double VectAbs(Vec3 a)
{
	return sqrt((a.x * a.x) + (a.y * a.y) + (a.z * a.z));
}

double VectDot(Vec3 a, Vec3 b, double *angle)
{
	if (angle != NULL)
		return VectMag(a) * VectMag(a) * cos(*angle);
	return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

Vec3 VectFromAngle_Deg(double x_angle, double y_angle, double magnitude)
{
	Vec3 result;

	result.x = cos(MT_ToRadf(y_angle)) * sin(MT_ToRadf(x_angle)) * magnitude;
	result.y = sin(MT_ToRadf(y_angle)) * magnitude;
	result.z = cos(MT_ToRadf(y_angle)) * cos(MT_ToRadf(x_angle)) * magnitude;

	return result;
}

Vec3 VectNormalize(Vec3 vec)
{
	Vec3 result;
	double mag;

	mag = VectMag(vec);

	result.x = vec.x / mag;
	result.y = vec.y / mag;
	result.z = vec.z / mag;

	return result;
}

double VectMag(Vec3 vec)
{
	return sqrt((vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z));
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
