#include "terrain.h"
#include "simplex.h"
//*********************************************************************************************
//*************************ALL TERRAIN ENGINE CODE HERE****************************************
//*********************************************************************************************
#define PATCH_DETAIL_LEVELS 8
#define PATCH_SIZE 128
#define HALF_PATCH_SIZE 64

struct
{
GLuint TerrainShader;
GLuint IndexBufferHandles[PATCH_DETAIL_LEVELS];
int numIndices[PATCH_DETAIL_LEVELS];
}TerrainData;


float BilinearInterp(float p1,float p2,float p3,float p4,float ux,float uy)
{
float i1=0,i2=0;
i1=((p2-p1)*ux)+p1;
i2=((p4-p3)*ux)+p3;
return ((i2-i1)*uy)+i1;
}

const unsigned char randtbl[256]=
    {151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,
    8,99,37,240,21,10,23,190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,
    35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,74,165,71,
    134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,
    55,46,245,40,244,102,143,54,65,25,63,161,1,216,80,73,209,76,132,187,208, 89,
    18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,52,217,226,
    250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,
    189,28,42,223,183,170,213,119,248,152,2,44,154,163,70,221,153,101,155,167,43,
    172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,97,
    228,251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,
    107,49,192,214,31,181,199,106,157,184,84,204,176,115,121,50,45,127,4,150,254,
    138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180};

//TODO- a custom prng would probably be faster
float GetRandFromSeed(int x,int y,int seed)
{
return ((float)(randtbl[(seed+randtbl[(x+randtbl[y])&0xFF]&0xFF)])/128.0)-1.0;
}



//*********************************************************************************************
//*****************************Initialisation code*********************************************
//*********************************************************************************************


void CreateTerrainShader()
{
printf("Creating Terrain shader\n");
GLuint vertexshader=glCreateShader(GL_VERTEX_SHADER);
GLuint fragmentshader=glCreateShader(GL_FRAGMENT_SHADER);
printf("Created shader objects\n");
TerrainData.TerrainShader=glCreateProgram();
glAttachShader(TerrainData.TerrainShader,vertexshader);
glAttachShader(TerrainData.TerrainShader,fragmentshader);
printf("Attached shaders\n");
char* vertsrc=ReadFile("vertexshader.glsl");
char* fragsrc=ReadFile("fragmentshader.glsl");
printf("Read files %d %d\n",vertsrc,fragsrc);
glShaderSource(vertexshader,1,&vertsrc,NULL);
glShaderSource(fragmentshader,1,&fragsrc,NULL);
glCompileShader(vertexshader);
glCompileShader(fragmentshader);
glLinkProgram(TerrainData.TerrainShader);


char error[256];
glGetShaderInfoLog(vertexshader,255,NULL,error);
printf("version %s\n",glGetString(GL_SHADING_LANGUAGE_VERSION));
printf("error %s\n",error);
//if(glGetShaderiv(vertexshader,GL_COMPILE_STATUS)==GL_TRUE)printf("Vertex shader comiled succesfully\n");
//if(glGetShaderiv(fragmentshader,GL_COMPILE_STATUS)==GL_TRUE)printf("Fragment shader comiled succesfully\n");
printf("Shader handle: %d\n",TerrainData.TerrainShader);
printf("Errors: %x\n",glGetError());
}


void InitIndexBuffers()
{
glGenBuffers(PATCH_DETAIL_LEVELS,&(TerrainData.IndexBufferHandles));
//Generate index buffer
GLushort* indices=malloc(sizeof(GLushort)*34000);

//TODO- see if I can remove some of this nesting
int detail;
    for(detail=0;detail<PATCH_DETAIL_LEVELS;detail++)
    {
    int points_per_side=(1<<detail)+1;
    int curindex=0;
    int i,j;
        for(j=0;j<points_per_side-1;j++)
        {
            for(i=0;i<points_per_side;i++)
            {
            //printf("curindex %d\n",curindex);
            //j-Current Row
            //i-current column
            indices[curindex++]=((j+1)*points_per_side)+i;//Add bottom vertex
            indices[curindex++]=(j*points_per_side)+i;//Add top vertex
            }
        indices[curindex++]=((j+1)*points_per_side)-1;//Add repeat vertices
        indices[curindex++]=((j+2)*points_per_side);//Add repeat vertices
        }
    printf("Num %d\n",curindex);
    TerrainData.numIndices[detail]=curindex;
    glEnableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,TerrainData.IndexBufferHandles[detail]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(GLushort)*TerrainData.numIndices[detail],indices,GL_STATIC_DRAW);
    glDisableClientState(GL_VERTEX_ARRAY);
    }
free(indices);

}


void InitialiseTerrainSystem()
{
//Create the terrain shader
CreateTerrainShader();
//Create the index buffers for terrain patches
InitIndexBuffers();
}


//*********************************************************************************************
//***********************************Geometry code*********************************************
//*********************************************************************************************

//Diamond square algorithm
void DiamondSquare(TerrainPoint** heightMap,TerrainPatch* patch,int scale)
{
int squares_per_side=1<<patch->detail;
int points_per_side=squares_per_side+1;
int square_size=PATCH_SIZE/squares_per_side;
int vertex_step=scale/square_size;
int step_size=vertex_step*square_size;
//printf("detail %d\nvertex_steps %d\nsquare_size %d\n\n",patch->detail,vertex_step,square_size);

//Calculate the roughness of each corner
float r1=(float)heightMap[patch->x][patch->y].roughness/255;
float r2=(float)heightMap[patch->x+1][patch->y].roughness/255;
float r3=(float)heightMap[patch->x][patch->y+1].roughness/255;
float r4=(float)heightMap[patch->x+1][patch->y+1].roughness/255;
//These values will be used for interpolating roughness
float ux,uy;
float u_step=(float)step_size/PATCH_SIZE;


//Random seed data must be taken from actual coordinates so that neighbouring patches will line up;these values are used to calculate those coordinates
int x,y;
int xoffset,yoffset;
xoffset=patch->x*PATCH_SIZE+step_size;
yoffset=patch->y*PATCH_SIZE+step_size;



int row,column;
//Accessing a vertex height is really long so here's a macro:
#define VERTEX_HEIGHT(row,col) (patch->vertices[(row)*points_per_side+(col)].position.Y)
//I'll also define a shorter alias for vertex_step
#define vs vertex_step

/*Diamond step. We set the height of the midpoint of each square to the average of its four vertices, and add
some random pertubation*/


uy=u_step;
y=yoffset;
    for(row=vertex_step;row<points_per_side;row+=2*vertex_step)
    {
    ux=u_step;
    x=xoffset;
        for(column=vertex_step;column<points_per_side;column+=2*vertex_step)
        {
        //Interpolate height based on four surrounding vertices
        VERTEX_HEIGHT(row,column)=(VERTEX_HEIGHT(row-vs,column-vs)+VERTEX_HEIGHT(row+vs,column-vs)+VERTEX_HEIGHT(row-vs,column+vs)+VERTEX_HEIGHT(row+vs,column+vs))/4;
        //Add random pertubation
        VERTEX_HEIGHT(row,column)+=GetRandFromSeed(x,y,scale)*scale*BilinearInterp(r1,r2,r3,r4,ux,uy);
        ux+=2*u_step;
        x+=2*step_size;
        }
    uy+=2*u_step;
    y+=2*step_size;
    }


/*The edge vertices have to be treated specially to ensure alignment with neigbouring patches; they take
their interpolated values from other vertices on the same edge only*/
//Processing the edges, so the x and y offsets are just the x and y coordinates of the patch

//Top edge
ux=u_step;
uy=0.0;
x=xoffset;
y=patch->y*PATCH_SIZE;
    for(column=vertex_step;column<points_per_side;column+=2*vertex_step)
    {
    VERTEX_HEIGHT(0,column)=(VERTEX_HEIGHT(0,column-vs)+VERTEX_HEIGHT(0,column+vs))/2;
    VERTEX_HEIGHT(0,column)+=GetRandFromSeed(x,y,scale)*scale*BilinearInterp(r1,r2,r3,r4,ux,uy);
    ux+=u_step;
    x+=2*step_size;
    }
//Bottom edge
ux=u_step;
uy=1.0;
x=xoffset;
y+=PATCH_SIZE;
row=squares_per_side;
    for(column=vertex_step;column<points_per_side;column+=2*vertex_step)
    {
    VERTEX_HEIGHT(row,column)=(VERTEX_HEIGHT(row,column-vs)+VERTEX_HEIGHT(row,column+vs))/2;
    VERTEX_HEIGHT(row,column)+=GetRandFromSeed(x,y,scale)*scale*BilinearInterp(r1,r2,r3,r4,ux,uy);
    ux+=u_step;
    x+=2*step_size;
    }
//Left edge
ux=0;
uy=u_step;
x=patch->x*PATCH_SIZE;
y=yoffset;
    for(row=vertex_step;row<points_per_side;row+=2*vertex_step)
    {
    VERTEX_HEIGHT(row,0)=(VERTEX_HEIGHT(row-vs,0)+VERTEX_HEIGHT(row+vs,0))/2;
    VERTEX_HEIGHT(row,0)+=GetRandFromSeed(x,y,scale)*scale*BilinearInterp(r1,r2,r3,r4,ux,uy);
    uy+=u_step;
    y+=2*step_size;
    }
//Right edge
ux=1.0;
uy=u_step;
x+=PATCH_SIZE;
y=yoffset;
column=squares_per_side;
    for(row=vertex_step;row<points_per_side;row+=2*vertex_step)
    {
    VERTEX_HEIGHT(row,squares_per_side)=(VERTEX_HEIGHT(row-vs,squares_per_side)+VERTEX_HEIGHT(row+vs,squares_per_side))/2;
    VERTEX_HEIGHT(row,squares_per_side)+=GetRandFromSeed(x,y,scale)*scale*BilinearInterp(r1,r2,r3,r4,ux,uy);
    uy+=u_step;
    y+=2*step_size;
    }


/*Square step. We set the height of the midpoint of each diamond to the average of its four vertices, and add
some random pertubation-excluding the edge vertices, those being a special case that has already been handled*/
uy=u_step;
y=yoffset;
//These are diamonds-not squares, so the starting offset of each row will change for each row
int row_offset=vertex_step;
    //squares_per_side is used in the iteration bounds to prevent the edge vertices being processed
    for(row=vertex_step;row<squares_per_side;row+=vertex_step)
    {
    ux=u_step;
    x=xoffset;
        if(row_offset)
        {
        ux+=u_step;
        x+=step_size;
        }
        for(column=vertex_step+row_offset;column<squares_per_side;column+=2*vertex_step)
        {
        //Interpolate height based on four surrounding vertices
        VERTEX_HEIGHT(row,column)=(VERTEX_HEIGHT(row-vs,column)+VERTEX_HEIGHT(row+vs,column)+VERTEX_HEIGHT(row,column+vs)+VERTEX_HEIGHT(row,column-vs))/4;
        //Add random pertubation
        VERTEX_HEIGHT(row,column)+=GetRandFromSeed(x,y,scale)*scale*BilinearInterp(r1,r2,r3,r4,ux,uy);
        ux+=u_step;
        x+=2*step_size;
        }
    //Calculate the new row_offset-it is alternately 0 and vertex_step
    row_offset=row_offset?0:vertex_step;
    uy+=u_step;
    y+=step_size;
    }
}




void AddNoiseLayer(TerrainPoint** heightMap,TerrainPatch* patch,int scale)
{
int squares_per_side=1<<patch->detail;
int points_per_side=squares_per_side+1;
float step_fraction=PATCH_SIZE/(float)(scale*squares_per_side);


//Calculate the roughness of each corner
float r1=(float)heightMap[patch->x][patch->y].roughness/255;
float r2=(float)heightMap[patch->x+1][patch->y].roughness/255;
float r3=(float)heightMap[patch->x][patch->y+1].roughness/255;
float r4=(float)heightMap[patch->x+1][patch->y+1].roughness/255;
//These values will be used for interpolating roughness
float ux,uy;
float u_step=1.0/squares_per_side;


float xoffset=(PATCH_SIZE*patch->x)/scale;
float yoffset=(PATCH_SIZE*patch->y)/scale;

float x=xoffset,y=yoffset;

int index=0;
int i,j,k;

uy=0;
    for(i=0;i<points_per_side;i++)
    {
    ux=0;
        for(j=0;j<points_per_side;j++)
        {
        patch->vertices[index].position.Y+=0.5*SimplexNoise(x,y,scale)*scale*BilinearInterp(r1,r2,r3,r4,ux,uy);
        index++;
        ux+=u_step;
        x+=step_fraction;
        }
    x=xoffset;
    uy+=u_step;
    y+=step_fraction;
    }

}


void CalculateNormals(TerrainPatch* terpatch)
{
int squares_per_side=1<<terpatch->detail;
int points_per_side=squares_per_side+1;


int i,j;
//Calculate vertex normals
int index=0;
    for(i=0;i<squares_per_side;i++)
    for(j=0;j<squares_per_side;j++)
    {
    //This loop runs once on each square; we add the vertices from each of the two triangle to all the vertices involved
    index=(i*points_per_side)+j;

    Vector U=VectorSubtract(terpatch->vertices[index+points_per_side].position,terpatch->vertices[index].position);
    Vector V=VectorSubtract(terpatch->vertices[index+1].position,terpatch->vertices[index].position);

    Vector N=VectorCrossProduct(U,V);

    terpatch->vertices[index].normal=VectorAdd(terpatch->vertices[index].normal,N);
    terpatch->vertices[index+1].normal=VectorAdd(terpatch->vertices[index+1].normal,N);
    terpatch->vertices[index+points_per_side].normal=VectorAdd(terpatch->vertices[index+points_per_side].normal,N);

    U=VectorSubtract(terpatch->vertices[index+1].position,terpatch->vertices[index+1+points_per_side].position);
    V=VectorSubtract(terpatch->vertices[index+points_per_side].position,terpatch->vertices[index+1+points_per_side].position);

    N=VectorCrossProduct(U,V);

    terpatch->vertices[index+1].normal=VectorAdd(terpatch->vertices[index+1].normal,N);
    terpatch->vertices[index+points_per_side].normal=VectorAdd(terpatch->vertices[index+points_per_side].normal,N);
    terpatch->vertices[index+1+points_per_side].normal=VectorAdd(terpatch->vertices[index+1+points_per_side].normal,N);
    }
}

void GeneratePatchGeometry(Terrain* terrain,TerrainPatch* terpatch)
{
int i,j;

int squares_per_side=1<<terpatch->detail;
int points_per_side=squares_per_side+1;
float square_size=PATCH_SIZE/(float)squares_per_side;

terpatch->numVertices=points_per_side*points_per_side;
terpatch->vertices=realloc(terpatch->vertices,sizeof(Vertex)*terpatch->numVertices);


float h1=(float)terrain->heightMap[terpatch->x][terpatch->y].height/10;
float h2=(float)terrain->heightMap[terpatch->x+1][terpatch->y].height/10;
float h3=(float)terrain->heightMap[terpatch->x][terpatch->y+1].height/10;
float h4=(float)terrain->heightMap[terpatch->x+1][terpatch->y+1].height/10;
float ux,uy;
float u_step=1.0/squares_per_side;

float offsetx=terpatch->x*128;
float offsety=terpatch->y*128;

    uy=0.0;
    for(i=0;i<points_per_side;i++)
    {
    ux=0.0;
    for(j=0;j<points_per_side;j++)
    {
    terpatch->vertices[(i*points_per_side)+j].position.X=j*square_size+offsetx;
    terpatch->vertices[(i*points_per_side)+j].position.Y=BilinearInterp(h1,h2,h3,h4,ux,uy);
    terpatch->vertices[(i*points_per_side)+j].position.Z=i*square_size+offsety;
    //Initialise the normals to zero
    terpatch->vertices[(i*points_per_side)+j].normal.X=0.0;
    terpatch->vertices[(i*points_per_side)+j].normal.Y=0.0;
    terpatch->vertices[(i*points_per_side)+j].normal.Z=0.0;
    ux+=u_step;
    }
    uy+=u_step;
    }


//Add fractal detail
int scale=128;
  for(i=0;i<7;i++)
    {
    //DiamondSquare(terrain->heightMap,terpatch,scale);
    //AddNoiseLayer(terrain->heightMap,terpatch,scale);
    scale=scale>>1;
    }

CalculateNormals(terpatch);


glEnableClientState(GL_VERTEX_ARRAY);
glBindBuffer(GL_ARRAY_BUFFER,terpatch->VBO);
glBufferData(GL_ARRAY_BUFFER,sizeof(Vertex)*terpatch->numVertices,terpatch->vertices,GL_STATIC_DRAW);
glDisableClientState(GL_VERTEX_ARRAY);
}






TerrainPatch ConstructTerrainPatch(Terrain* terrain,int x,int y)
{
TerrainPatch newpatch;
newpatch.x=x;
newpatch.y=y;
//Set detail to minimum
newpatch.detail=0;

//Generate geometry
newpatch.vertices=NULL;
//Create VBO
glGenBuffers(1,&(newpatch.VBO));
//Create vertices
GeneratePatchGeometry(terrain,&newpatch);

//Calculate midpoint
newpatch.midPoint.X=(x*128)+64;
newpatch.midPoint.Y=(newpatch.vertices[0].position.Y+newpatch.vertices[1].position.Y+newpatch.vertices[2].position.Y+newpatch.vertices[3].position.Y)/4;
newpatch.midPoint.Z=(y*128)+64;

return newpatch;
}

void DestroyTerrainPatch(TerrainPatch* victim)
{
glDeleteBuffers(1,&victim->VBO);
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
        terrain.heightMap[i][j].height=0;//(int)(SimplexNoise(i/8.0,j/8.0,1)*2000+SimplexNoise(i/4.0,j/4.0,2)*1000+SimplexNoise(i/2.0,j/2.0,3)*500+SimplexNoise(i,j,3)*250);
        terrain.heightMap[i][j].roughness=i*10+j*10;
        }
    }

//Create terrain patches
terrain.patches=malloc(sizeof(TerrainPatch*)*width);
int x,y;
    for(x=0;x<width;x++)
    {
    terrain.patches[x]=malloc(sizeof(TerrainPatch)*height);
        for(y=0;y<height;y++)terrain.patches[x][y]=ConstructTerrainPatch(&terrain,x,y);
    }
return terrain;
}


void FinishTerrainSystem(Terrain* terrain)
{
int i,x,y;
for(x=0;x<terrain->width;x++)
for(y=0;y<terrain->height;y++)
{
DestroyTerrainPatch(&terrain->patches[x][y]);
}
}




int GetPatchDetail(Camera* camera,TerrainPatch* patch)
{
float xdiff=patch->midPoint.X-camera->Position.X;
float zdiff=patch->midPoint.Z-camera->Position.Z;
float planedist2=(xdiff*xdiff)+(zdiff*zdiff);
planedist2-=5776;//76^2
if(planedist2<0)planedist2=0;

float ydiff=patch->midPoint.Y-camera->Position.Y;
float dist=sqrt(planedist2+(ydiff*ydiff));
//printf("dist: %f\n",dist);
if(dist<300)return 7;
else if(dist<500)return 6;
else if(dist<800)return 5;
else if(dist<1200)return 4;
else if(dist<1700)return 3;
else if(dist<2300)return 2;
return 1;
}

void UpdateTerrain(Camera* camera,Terrain* terrain)
{
int x,y,detail;
    for(x=0;x<terrain->width;x++)
    for(y=0;y<terrain->height;y++)
    {
    detail=GetPatchDetail(camera,&terrain->patches[x][y]);
        if(detail!=terrain->patches[x][y].detail)
        {
        terrain->patches[x][y].detail=detail;
        GeneratePatchGeometry(terrain,&terrain->patches[x][y]);
        }
    }
}


//Rendering code

void SetUpTerrainRender(Camera* camera,Terrain* terrain)
{
glEnableClientState(GL_VERTEX_ARRAY);
glEnableClientState(GL_NORMAL_ARRAY);
glUseProgram(TerrainData.TerrainShader);
//printf("Shader %d %x\n",TerrainShader,glGetError());
}
void RenderPatch(TerrainPatch* patch)
{
glBindBuffer(GL_ARRAY_BUFFER,patch->VBO);
glNormalPointer(GL_FLOAT,32,12);
glVertexPointer(3,GL_FLOAT,32,0);

glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,TerrainData.IndexBufferHandles[patch->detail]);

glDrawElements(GL_TRIANGLE_STRIP,TerrainData.numIndices[patch->detail],GL_UNSIGNED_SHORT,0);
//printf("Rendering: %x\n",glGetError());
}
void ClearUpTerrainRender()
{
glDisableClientState(GL_VERTEX_ARRAY);
glDisableClientState(GL_NORMAL_ARRAY);
}


void RenderTerrain(Camera* camera,Terrain* terrain)
{
UpdateTerrain(camera,terrain);
SetUpTerrainRender(camera,terrain);
SetModelViewFromCamera(camera);
int x,y;
    for(x=0;x<terrain->width;x++)
    for(y=0;y<terrain->height;y++)
    {
    RenderPatch(&terrain->patches[x][y]);
    }
ClearUpTerrainRender();
}
//*********************************************************************************************
//*************************************END TERRAIN CODE****************************************
//*********************************************************************************************
