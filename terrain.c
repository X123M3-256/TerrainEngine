#include "terrain.h"

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
char error[256];
glGetShaderInfoLog(vertexshader,255,NULL,error);
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
InitBuffers();
}

void FinishTerrainSystem()
{

}


void CreateTexture(Terrain* terrain)
{
glGenTextures(1,&(terrain->heightMapTex));
glBindTexture(GL_TEXTURE_2D,terrain->heightMapTex);
}

Terrain CreateTerrain(int width,int height)
{
Terrain terrain;

terrain.width=width;
terrain.height=height;

int xpoints=width+1;
int ypoints=height+1;

terrain.heightMap=malloc(xpoints*sizeof(TerrainPoint*));
int i,j;
    for(i=0;i<xpoints;i++)
    {
    terrain.heightMap[i]=malloc(ypoints*sizeof(TerrainPoint));
        for(j=0;j<ypoints;j++)
        {
        terrain.heightMap[i][j].height=(int)(SimplexNoise(i/8.0,j/8.0,1)*2000+SimplexNoise(i/4.0,j/4.0,2)*1000+SimplexNoise(i/2.0,j/2.0,3)*500+SimplexNoise(i,j,3)*250);
        terrain.heightMap[i][j].roughness=i*10+j*10;
        }
    }
return terrain;
}

void FreeTerrain(Terrain* terrain)
{

}

void RenderPatch(float scale,Vector displacement,Matrix modelViewProjection)
{
glUniformMatrix4fv(Resources.MVPLocation,1,GL_FALSE,&(modelViewProjection.Data));

glUniform1fv(Resources.scaleLocation,1,&scale);
glUniform2fv(Resources.displacementLocation,1,&displacement);
glDrawElements(GL_TRIANGLE_STRIP,NUM_INDICES,GL_UNSIGNED_SHORT,0);
}


void EpicRecursiveRenderTime(int scale,Vector displacement,Vector cameraPosition,Matrix modelViewProjection)
{
//Compute centre
Vector centre;
centre.X=displacement.X;
centre.Z=displacement.Y;
centre.Y=0;
centre.X+=scale*128;
centre.Z+=scale*128;
float distance=VectorMagnitude(VectorSubtract(centre,cameraPosition))-scale*128;
//printf("%f\n",distance);
    if(distance<scale*256&&scale>1)
    {
    scale/=2;
    Vector translation;
    translation.X=0;
    translation.Y=0;
    translation.Z=0;
    EpicRecursiveRenderTime(scale,VectorAdd(displacement,translation),cameraPosition,modelViewProjection);
    translation.X=scale*256;
    EpicRecursiveRenderTime(scale,VectorAdd(displacement,translation),cameraPosition,modelViewProjection);
    translation.Y=translation.X;
    EpicRecursiveRenderTime(scale,VectorAdd(displacement,translation),cameraPosition,modelViewProjection);
    translation.X=0;
    EpicRecursiveRenderTime(scale,VectorAdd(displacement,translation),cameraPosition,modelViewProjection);
    return;
    }
RenderPatch(scale,displacement,modelViewProjection);
}


void DrawBox(float x,float y,float z,float size)
{
glVertex3f(x-size,y,z+size);
glVertex3f(x+size,y,z+size);
glVertex3f(x+size,y,z-size);
glVertex3f(x-size,y,z-size);

glVertex3f(x-size,y+2*size,z+size);
glVertex3f(x+size,y+2*size,z+size);
glVertex3f(x+size,y+2*size,z-size);
glVertex3f(x-size,y+2*size,z-size);

glVertex3f(x-size,y+2*size,z+size);
glVertex3f(x+size,y+2*size,z+size);
glVertex3f(x+size,y,z+size);
glVertex3f(x-size,y,z+size);

glVertex3f(x-size,y+2*size,z-size);
glVertex3f(x+size,y+2*size,z-size);
glVertex3f(x+size,y,z-size);
glVertex3f(x-size,y,z-size);

glVertex3f(x+size,y+2*size,z-size);
glVertex3f(x+size,y+2*size,z+size);
glVertex3f(x+size,y,z+size);
glVertex3f(x+size,y,z-size);

glVertex3f(x-size,y+2*size,z-size);
glVertex3f(x-size,y+2*size,z+size);
glVertex3f(x-size,y,z+size);
glVertex3f(x-size,y,z-size);
}

void RenderTerrain(Terrain* terrain,Camera* camera,Matrix modelViewProjection)
{
glEnableClientState(GL_VERTEX_ARRAY);
glUseProgram(Resources.Shader);

glBindBuffer(GL_ARRAY_BUFFER,Resources.VBO);
glVertexPointer(3,GL_FLOAT,32,0);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,Resources.IBO);

Vector displacement;
displacement.X=0;
displacement.Y=0;
displacement.Z=0;

EpicRecursiveRenderTime(64,displacement,camera->Position,modelViewProjection);


glDisableClientState(GL_VERTEX_ARRAY);
}


//*********************************************************************************************
//*************************************END TERRAIN CODE****************************************
//*********************************************************************************************
