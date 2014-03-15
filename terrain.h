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
//This is *not* the coordinates of the patch but indices into the height map
int x,y;
//The coordinates of the midpoint of this terrain patch (Simple average of the four corners)
Vector midPoint;
//The level of detail that this terrain patch is currently rendering at
char detail;
//Client side copy of the terrain vertices. Allows editing without regenerating the entire patch.
Vertex* vertices;
int numVertices;
//Handle for the VBO
int VBO;
}TerrainPatch;

typedef struct
{
int width;
int height;
TerrainPoint** heightMap;
TerrainPatch** patches;
}Terrain;


void InitialiseTerrainSystem();

void FinishTerrainSystem(Terrain* terrain);

Terrain CreateTerrain(int width,int height);

//Rendering code
void RenderTerrain(Camera* camera,Terrain* terrain);



#endif // TERRAIN_H_INCLUDED
