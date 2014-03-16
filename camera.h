#ifndef CAMERA_H_INCLUDED
#define CAMERA_H_INCLUDED
#include "math.h"
typedef struct
{
Vector Position;
Quaternion Rotation;
}Camera;


Camera CreateCamera();
void PitchCamera(Camera* Cam,float Angle);
void RollCamera(Camera* Cam,float Angle);
void YawCamera(Camera* Cam,float Angle);
void TranslateCamera(Camera* Cam,Vector Translation);

Matrix MatrixFromCamera(Camera* camera);

#endif // CAMERA_H_INCLUDED
