#ifndef MATH_H_INCLUDED
#define MATH_H_INCLUDED

typedef struct
{
float Data[16];
}
Matrix;

typedef struct
{
float X;
float Y;
float Z;
}Vector;

typedef struct
{
float W;
float I;
float J;
float K;
}Quaternion;

Quaternion QuaternionConjugate(Quaternion Quat);
Quaternion QuaternionMultiply(Quaternion A,Quaternion B);
Vector QuaternionTransformVector(Quaternion Quat,Vector Vec);
Matrix MatrixFromQuaternion(Quaternion Qtr);

Vector VectorNormalize(Vector A);
Vector VectorAdd(Vector A,Vector B);
Vector VectorSubtract(Vector A,Vector B);
Vector VectorCrossProduct(Vector A,Vector B);

Matrix MatrixIdentity();
Matrix MatrixMultiply(Matrix A,Matrix B);


Matrix ProjectionMatrix(float left,float right,float bottom,float top,float near,float far);

#endif // MATH_H_INCLUDED
