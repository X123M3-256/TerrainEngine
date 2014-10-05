#ifndef TERRAIN_H_INCLUDED
#define TERRAIN_H_INCLUDED

#include "engine.h"
#include "math.h"
#include "camera.h"
//*********************************************************************************************
//*************************ALL TERRAIN ENGINE CODE HERE****************************************
//*********************************************************************************************

#define PATCH_DETAIL_LEVELS 8
#define PATCH_SIZE 128
#define HALF_PATCH_SIZE 64


typedef struct
{
short height;
unsigned char roughness;
}TerrainPoint;


typedef struct
{
int width;
int height;
GLuint heightMapTex;
TerrainPoint** heightMap;
}Terrain;


void InitialiseTerrainSystem();

void FinishTerrainSystem();

Terrain CreateTerrain(const char* filename);

//Rendering code
void RenderTerrain(Terrain* terrain,Camera* camera,Matrix modelViewProjection);



#endif // TERRAIN_H_INCLUDED
