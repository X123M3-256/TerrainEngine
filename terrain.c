#include<stdio.h>
#include "terrain.h"
#include "simplex.h"
//*********************************************************************************************
//*************************ALL TERRAIN ENGINE CODE HERE****************************************
//*********************************************************************************************
#define PATCH_SIZE 128
#define HALF_PATCH_SIZE 64
#define POINTS_PER_SIDE 129
#define NUM_INDICES 33280
struct
    {
    GLuint IBO;
    GLuint VBO;
    GLuint Shader;
    GLint MVPLocation;
    GLint scaleLocation;
    GLint displacementLocation;
    GLint heightmapLocation;

    GLuint WaterShader;//Temporary
    GLint waterMVPLocation;
    GLint waterScaleLocation;
    GLint waterDisplacementLocation;
    GLint waterHeightmapLocation;
    }Resources;


void CreateTerrainShader()
{
GLuint vertexshader=glCreateShader(GL_VERTEX_SHADER);
GLuint fragmentshader=glCreateShader(GL_FRAGMENT_SHADER);
Resources.Shader=glCreateProgram();
glAttachShader(Resources.Shader,vertexshader);
glAttachShader(Resources.Shader,fragmentshader);
char* vertsrc=ReadFile("vertexshader.glsl");
char* fragsrc=ReadFile("fragmentshader.glsl");
glShaderSource(vertexshader,1,&vertsrc,NULL);
glShaderSource(fragmentshader,1,&fragsrc,NULL);
glCompileShader(vertexshader);
glCompileShader(fragmentshader);
glLinkProgram(Resources.Shader);


Resources.MVPLocation=glGetUniformLocation(Resources.Shader,"modelViewProjection");
Resources.scaleLocation=glGetUniformLocation(Resources.Shader,"scale");
Resources.displacementLocation=glGetUniformLocation(Resources.Shader,"displacement");
Resources.heightmapLocation=glGetUniformLocation(Resources.Shader,"heightmap");

char error[1024];
glGetShaderInfoLog(vertexshader,1023,NULL,error);
printf("version %s\n",glGetString(GL_SHADING_LANGUAGE_VERSION));
printf("error %s\n",error);
}

void CreateWaterShader()
{
GLuint vertexshader=glCreateShader(GL_VERTEX_SHADER);
GLuint fragmentshader=glCreateShader(GL_FRAGMENT_SHADER);
Resources.WaterShader=glCreateProgram();
glAttachShader(Resources.WaterShader,vertexshader);
glAttachShader(Resources.WaterShader,fragmentshader);
char* vertsrc=ReadFile("watervertexshader.glsl");
char* fragsrc=ReadFile("waterfragmentshader.glsl");
glShaderSource(vertexshader,1,&vertsrc,NULL);
glShaderSource(fragmentshader,1,&fragsrc,NULL);
glCompileShader(vertexshader);
glCompileShader(fragmentshader);
glLinkProgram(Resources.WaterShader);


Resources.waterMVPLocation=glGetUniformLocation(Resources.Shader,"modelViewProjection");
Resources.waterScaleLocation=glGetUniformLocation(Resources.Shader,"scale");
Resources.waterDisplacementLocation=glGetUniformLocation(Resources.Shader,"displacement");
Resources.waterHeightmapLocation=glGetUniformLocation(Resources.Shader,"heightmap");

char error[1024];
glGetShaderInfoLog(vertexshader,1023,NULL,error);
printf("version %s\n",glGetString(GL_SHADING_LANGUAGE_VERSION));
printf("error %s\n",error);
}

void InitBuffers()
{
glGenBuffers(1,&(Resources.VBO));
glGenBuffers(1,&(Resources.IBO));
//Allocate memory for buffers
Vertex* vertices=malloc(sizeof(Vertex)*POINTS_PER_SIDE*POINTS_PER_SIDE);
GLushort* indices=malloc(sizeof(GLushort)*NUM_INDICES);

//Create vertices
int curindex=0;
int x,z;
    for(z=0;z<POINTS_PER_SIDE;z++)
    for(x=0;x<POINTS_PER_SIDE;x++)
    {
    vertices[curindex].position.X=x*2;
    vertices[curindex].position.Y=0;
    vertices[curindex].position.Z=z*2;
    curindex++;
    }

//Create indices
curindex=0;
int row,col;
    for(row=0;row<POINTS_PER_SIDE-1;row++)
    {
        for(col=0;col<POINTS_PER_SIDE;col++)
        {
        indices[curindex++]=((row+1)*POINTS_PER_SIDE)+col;//Add bottom vertex
        indices[curindex++]=(row*POINTS_PER_SIDE)+col;//Add top vertex
        }
    indices[curindex++]=((row+1)*POINTS_PER_SIDE)-1;//Add repeat vertices
    indices[curindex++]=(row+2)*POINTS_PER_SIDE;//Add repeat vertices
    }
//Send buffer data to graphics card
glEnableClientState(GL_VERTEX_ARRAY);
glBindBuffer(GL_ARRAY_BUFFER,Resources.VBO);
glBufferData(GL_ARRAY_BUFFER,sizeof(Vertex)*POINTS_PER_SIDE*POINTS_PER_SIDE,vertices,GL_STATIC_DRAW);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,Resources.IBO);
glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(GLushort)*NUM_INDICES,indices,GL_STATIC_DRAW);
glDisableClientState(GL_VERTEX_ARRAY);

//Client side memory is no longer needed
free(vertices);
free(indices);
}

void InitialiseTerrainSystem()
{
printf("Initialising terrain system\n");
CreateTerrainShader();
CreateWaterShader();
InitBuffers();
}

void FinishTerrainSystem()
{

}



Terrain CreateTerrain(const char* filename)
{
Terrain terrain;



terrain.width=255;//width;
terrain.height=255;//height;

//Generate heightmap
int xpoints=terrain.width+1;
int ypoints=terrain.width+1;


terrain.heightMap=malloc(xpoints*sizeof(TerrainPoint*));
GLfloat* texData=malloc(xpoints*ypoints*sizeof(GLfloat)*4);

FILE* file=fopen(filename,"r");
int i,j,index=0;
    for(i=0;i<xpoints;i++)
    {
    terrain.heightMap[i]=malloc(ypoints*sizeof(TerrainPoint));
        for(j=0;j<ypoints;j++)
        {
        texData[index*4]=fgetc(file)*10.0;//(SimplexNoise(i/32.0,j/32.0,1)*800+SimplexNoise(i/16.0,j/16.0,1)*400+SimplexNoise(i/8.0,j/8.0,1)*200+SimplexNoise(i/4.0,j/4.0,2)*100+SimplexNoise(i/2.0,j/2.0,3)*50+SimplexNoise(i,j,3)*25);
        texData[1+index*4]=200.0;
        terrain.heightMap[i][j].height=(int)texData[index];
        terrain.heightMap[i][j].roughness=i*10+j*10;
        index++;
        }
    }
fclose(file);

//Generate texture
glGenTextures(1,&(terrain.heightMapTex));
glBindTexture(GL_TEXTURE_2D,terrain.heightMapTex);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA32F_ARB,xpoints,ypoints,0,GL_RGBA,GL_FLOAT,texData);
free(texData);


return terrain;
}



void FreeTerrain(Terrain* terrain)
{

}

void RenderPatch(float scale,Vector displacement,Matrix modelViewProjection)
{
glUniformMatrix4fv(Resources.MVPLocation,1,GL_FALSE,&(modelViewProjection.Data));

glUniform1i(Resources.heightmapLocation,0);
glUniform1fv(Resources.scaleLocation,1,&scale);
glUniform2fv(Resources.displacementLocation,1,&displacement);
glDrawElements(GL_TRIANGLE_STRIP,NUM_INDICES,GL_UNSIGNED_SHORT,0);
}

void RenderWaterPatch(float scale,Vector displacement,Matrix modelViewProjection)
{
glUniformMatrix4fv(Resources.waterMVPLocation,1,GL_FALSE,&(modelViewProjection.Data));
glUniform1i(Resources.waterHeightmapLocation,0);
glUniform1fv(Resources.waterScaleLocation,1,&scale);
glUniform2fv(Resources.waterDisplacementLocation,1,&displacement);
glDrawElements(GL_TRIANGLE_STRIP,NUM_INDICES,GL_UNSIGNED_SHORT,0);
}

void EpicRecursiveRenderTime(int scale,Vector displacement,Vector cameraPosition,Vector cameraDirection,Matrix modelViewProjection)
{
//Compute centre
Vector centre;
centre.X=displacement.X;
centre.Z=displacement.Y;
centre.Y=0;
centre.X+=scale*128;
centre.Z+=scale*128;
cameraPosition.Y=0;
Vector cameraToCentre=VectorSubtract(centre,cameraPosition);
float distance=VectorMagnitude(cameraToCentre);//Compute distance to center of patch
cameraToCentre=VectorMultiply(cameraToCentre,1/distance);//Normalize
distance-=scale*(128*1.414);//We want the distance to the *edge* of the patch
//Remove those areas behind the viewer
 //   if(VectorDotProduct(cameraToCentre,cameraDirection)<-0.5&&distance>0)return;
    if(distance<scale*150&&scale>4)
    {
    scale/=2;
    Vector translation;
    translation.X=0;
    translation.Y=0;
    translation.Z=0;
    EpicRecursiveRenderTime(scale,VectorAdd(displacement,translation),cameraPosition,cameraDirection,modelViewProjection);
    translation.X=scale*256;
    EpicRecursiveRenderTime(scale,VectorAdd(displacement,translation),cameraPosition,cameraDirection,modelViewProjection);
    translation.Y=translation.X;
    EpicRecursiveRenderTime(scale,VectorAdd(displacement,translation),cameraPosition,cameraDirection,modelViewProjection);
    translation.X=0;
    EpicRecursiveRenderTime(scale,VectorAdd(displacement,translation),cameraPosition,cameraDirection,modelViewProjection);
    return;
    }

glUseProgram(Resources.Shader);
RenderPatch(scale,displacement,modelViewProjection);
//glUseProgram(Resources.WaterShader);
//RenderPatch(scale,displacement,modelViewProjection);
}



void RenderTerrain(Terrain* terrain,Camera* camera,Matrix modelViewProjection)
{
glEnableClientState(GL_VERTEX_ARRAY);
glUseProgram(Resources.Shader);

glBindBuffer(GL_ARRAY_BUFFER,Resources.VBO);
glVertexPointer(3,GL_FLOAT,32,0);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,Resources.IBO);

glActiveTexture(GL_TEXTURE0);
glBindTexture(GL_TEXTURE_2D,terrain->heightMapTex);


Vector cameraDirection;
cameraDirection.X=0;
cameraDirection.Z=-1;
cameraDirection.Y=0;
cameraDirection=QuaternionTransformVector(camera->Rotation,cameraDirection);


Vector displacement;
displacement.X=0;
displacement.Y=0;
displacement.Z=0;

EpicRecursiveRenderTime(128,displacement,camera->Position,cameraDirection,modelViewProjection);


EpicRecursiveRenderTime(128,displacement,camera->Position,cameraDirection,modelViewProjection);

glDisableClientState(GL_VERTEX_ARRAY);
}


//*********************************************************************************************
//*************************************END TERRAIN CODE****************************************
//*********************************************************************************************
