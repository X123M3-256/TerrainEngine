#include<math.h>
#include "math.h"
Quaternion QuaternionConjugate(Quaternion Quat)
{
Quaternion Conjugate;
Conjugate.W=Quat.W;
Conjugate.I=-Quat.I;
Conjugate.J=-Quat.J;
Conjugate.K=-Quat.K;
return Conjugate;
}
Quaternion QuaternionMultiply(Quaternion A,Quaternion B)
{
Quaternion Result;
Result.W=(A.W*B.W)-(A.I*B.I)-(A.J*B.J)-(A.K*B.K);
Result.I=(A.W*B.I)+(A.I*B.W)+(A.J*B.K)-(A.K*B.J);
Result.J=(A.W*B.J)-(A.I*B.K)+(A.J*B.W)+(A.K*B.I);
Result.K=(A.W*B.K)+(A.I*B.J)-(A.J*B.I)+(A.K*B.W);
return Result;
}
Vector QuaternionTransformVector(Quaternion Quat,Vector Vec)
{
Vector Result;
//Multiply quaternion conjugate by vector
Quaternion QV;
QV.W=(Quat.I*Vec.X)+(Quat.J*Vec.Y)+(Quat.K*Vec.Z);
QV.I=(Quat.W*Vec.X)-(Quat.J*Vec.Z)+(Quat.K*Vec.Y);
QV.J=(Quat.W*Vec.Y)+(Quat.I*Vec.Z)-(Quat.K*Vec.X);
QV.K=(Quat.W*Vec.Z)-(Quat.I*Vec.Y)+(Quat.J*Vec.X);
//Multiply result by quaternion again
Result.X=(QV.W*Quat.I)+(QV.I*Quat.W)+(QV.J*Quat.K)-(QV.K*Quat.J);
Result.Y=(QV.W*Quat.J)-(QV.I*Quat.K)+(QV.J*Quat.W)+(QV.K*Quat.I);
Result.Z=(QV.W*Quat.K)+(QV.I*Quat.J)-(QV.J*Quat.I)+(QV.K*Quat.W);
return Result;
}
#define SQ(A) (A*A)
Matrix MatrixFromQuaternion(Quaternion Qtr)
{
Matrix RotationMatrix;
float IJ=Qtr.I*Qtr.J;
float KW=Qtr.K*Qtr.W;
float IK=Qtr.I*Qtr.K;
float JW=Qtr.J*Qtr.W;
float JK=Qtr.J*Qtr.K;
float IW=Qtr.I*Qtr.W;
RotationMatrix.Data[0]=1-2*(SQ(Qtr.J)+SQ(Qtr.K));
RotationMatrix.Data[4]=2*(IJ-KW);
RotationMatrix.Data[8]=2*(IK+JW);
RotationMatrix.Data[12]=0;
RotationMatrix.Data[1]=2*(IJ+KW);
RotationMatrix.Data[5]=1-2*(SQ(Qtr.I)+SQ(Qtr.K));
RotationMatrix.Data[9]=2*(JK-IW);
RotationMatrix.Data[13]=0;
RotationMatrix.Data[2]=2*(IK-JW);
RotationMatrix.Data[6]=2*(JK+IW);
RotationMatrix.Data[10]=1-2*(SQ(Qtr.I)+SQ(Qtr.J));
RotationMatrix.Data[14]=0;
RotationMatrix.Data[3]=0;
RotationMatrix.Data[7]=0;
RotationMatrix.Data[11]=0;
RotationMatrix.Data[15]=1;
return RotationMatrix;
}

float VectorMagnitude(Vector A)
{
return sqrt(A.X*A.X+A.Y*A.Y+A.Z*A.Z);
}
Vector VectorNormalize(Vector A)
{
float magnitude=sqrt(A.X*A.X+A.Y*A.Y+A.Z*A.Z);
A.X/=magnitude;
A.Y/=magnitude;
A.Z/=magnitude;
return A;
}
float VectorDotProduct(Vector A,Vector B)
{
return A.X*B.X+A.Y*B.Y+A.Z*B.Z;
}
Vector VectorCrossProduct(Vector A,Vector B)
{
Vector Result;
Result.X=(A.Y*B.Z)-(A.Z*B.Y);
Result.Y=(A.Z*B.X)-(A.X*B.Z);
Result.Z=(A.X*B.Y)-(A.Y*B.X);
return Result;
}
Vector VectorAdd(Vector A,Vector B)
{
Vector Result;
Result.X=A.X+B.X;
Result.Y=A.Y+B.Y;
Result.Z=A.Z+B.Z;
return Result;
}
Vector VectorSubtract(Vector A,Vector B)
{
Vector Result;
Result.X=A.X-B.X;
Result.Y=A.Y-B.Y;
Result.Z=A.Z-B.Z;
return Result;
}

Vector VectorMultiply(Vector A,float B)
{
A.X*=B;
A.Y*=B;
A.Z*=B;
return A;
}



Matrix MatrixIdentity()
{
Matrix matrix;
matrix.Data[0]=1;
matrix.Data[4]=0;
matrix.Data[8]=0;
matrix.Data[12]=0;
matrix.Data[1]=0;
matrix.Data[5]=1;
matrix.Data[9]=0;
matrix.Data[13]=0;
matrix.Data[2]=0;
matrix.Data[6]=0;
matrix.Data[10]=1;
matrix.Data[14]=0;
matrix.Data[3]=0;
matrix.Data[7]=0;
matrix.Data[11]=0;
matrix.Data[15]=1;
return matrix;
}
Matrix MatrixMultiply(Matrix A,Matrix B)
{
Matrix result;
int i,row,column;
for(row=0;row<4;row++)
for(column=0;column<4;column++)
{
result.Data[4*column+row]=0;
for(i=0;i<4;i++)result.Data[4*column+row]+=A.Data[4*i+row]*B.Data[4*column+i];
}
return result;
}
Matrix TranslationMatrix(Vector displacement)
{
Matrix matrix=MatrixIdentity();
matrix.Data[12]-=displacement.X;
matrix.Data[13]-=displacement.Y;
matrix.Data[14]-=displacement.Z;
return matrix;
}
Matrix ProjectionMatrix(float left,float right,float bottom,float top,float near,float far)
{
Matrix matrix;
matrix.Data[0]=2*near/(right-left);
matrix.Data[4]=0;
matrix.Data[8]=(right+left)/(right-left);
matrix.Data[12]=0;
matrix.Data[1]=0;
matrix.Data[5]=2*near/(top-bottom);
matrix.Data[9]=2*(top+bottom)/(top-bottom);
matrix.Data[13]=0;
matrix.Data[2]=0;
matrix.Data[6]=0;
matrix.Data[10]=-(far+near)/(far-near);
matrix.Data[14]=-2*far*near/(far-near);
matrix.Data[3]=0;
matrix.Data[7]=0;
matrix.Data[11]=-1;
matrix.Data[15]=0;
return matrix;
}
