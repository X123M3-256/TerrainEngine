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


void SetModelViewFromCamera(Camera* Cam)
{
glMatrixMode(GL_MODELVIEW);
Matrix RotationMatrix=MatrixFromQuaternion(Cam->Rotation);
glLoadMatrixf(RotationMatrix.Data);
glTranslatef(-Cam->Position.X,-Cam->Position.Y,-Cam->Position.Z);
}

void RenderWithCamera(Camera* Cam,void(*Render)())
{
glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
glMatrixMode(GL_MODELVIEW);
Matrix RotationMatrix=MatrixFromQuaternion(Cam->Rotation);
glLoadMatrixf(RotationMatrix.Data);
glTranslatef(-Cam->Position.X,-Cam->Position.Y,-Cam->Position.Z);
Render();
glFinish();
SDL_GL_SwapBuffers();
}
//This had to be done
void RenderWithCameraStereoscopic(Camera* Cam,void(*Render)())
{
Vector StereoBase;
StereoBase.X=1;
StereoBase.Y=0;
StereoBase.Z=0;
StereoBase=QuaternionTransformVector(Cam->Rotation,StereoBase);

StereoBase.X*=0.25;
StereoBase.Y*=0.25;
StereoBase.Z*=0.25;

glMatrixMode(GL_MODELVIEW);
Matrix RotationMatrix=MatrixFromQuaternion(Cam->Rotation);
glLoadMatrixf(RotationMatrix.Data);
glTranslatef(-Cam->Position.X,-Cam->Position.Y,-Cam->Position.Z);
glPushMatrix();

glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
//Render left eye image
glColorMask(GL_TRUE,GL_FALSE,GL_FALSE,GL_FALSE);
glTranslatef(StereoBase.X,StereoBase.Y,StereoBase.Z);
Render();

glClear(GL_DEPTH_BUFFER_BIT);
//Render right eye image
glPopMatrix();
glColorMask(GL_FALSE,GL_TRUE,GL_TRUE,GL_FALSE);
glTranslatef(-StereoBase.X,-StereoBase.Y,-StereoBase.Z);
Render();

glFinish();
SDL_GL_SwapBuffers();
}
