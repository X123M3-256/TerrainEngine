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
TerrainPoint** heightMap;
}Terrain;


void InitialiseTerrainSystem();

void FinishTerrainSystem();

Terrain CreateTerrain(int width,int height);

//Rendering code
void RenderTerrain(Terrain* terrain,Matrix modelViewProjection);



#endif // TERRAIN_H_INCLUDED
