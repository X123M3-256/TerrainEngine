#ifndef ENGINE_H_INCLUDED
#define ENGINE_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include<GL/gl.h>
#include<GL/glu.h>
#include "math.h"


typedef int bool;
#define true 1
#define false 0

typedef struct
{
Vector position;
Vector normal;
GLubyte padding[8];
}Vertex;

char* ReadFile(char* filename);

#endif // ENGINE_H_INCLUDED
