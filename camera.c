#include<SDL/SDL_opengl.h>
#include<GL/gl.h>
#include "camera.h"

Camera CreateCamera()
{
Camera Cam;
Cam.Position.X=0;
Cam.Position.Y=10;
Cam.Position.Z=0;

Cam.Rotation.W=1;
Cam.Rotation.I=0;
Cam.Rotation.J=0;
Cam.Rotation.K=0;
return Cam;
}

void PitchCamera(Camera* Cam,float Angle)
{
Quaternion Pitch;
Pitch.W=cos(Angle/2);
Pitch.I=sin(Angle/2);
Pitch.J=0;
Pitch.K=0;
Cam->Rotation=QuaternionMultiply(Pitch,Cam->Rotation);
}
void RollCamera(Camera* Cam,float Angle)
{
Quaternion Roll;
Roll.W=cos(Angle/2);
Roll.I=0;
Roll.J=0;
Roll.K=sin(Angle/2);
Cam->Rotation=QuaternionMultiply(Roll,Cam->Rotation);
}
void YawCamera(Camera* Cam,float Angle)
{
Quaternion Yaw;
Yaw.W=cos(Angle/2);
Yaw.I=0;
Yaw.J=sin(Angle/2);
Yaw.K=0;

Cam->Rotation=QuaternionMultiply(Cam->Rotation,Yaw);
}
void TranslateCamera(Camera* Cam,Vector Translation)
{
Translation=QuaternionTransformVector(Cam->Rotation,Translation);
Cam->Position=VectorAdd(Cam->Position,Translation);
}

Matrix MatrixFromCamera(Camera* camera)
{
Matrix rotation=MatrixFromQuaternion(camera->Rotation);

Matrix matrix=TranslationMatrix(camera->Position);
matrix=MatrixMultiply(rotation,matrix);

return matrix;
}

