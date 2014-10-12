#include<stdio.h>
#include<SDL/SDL.h>
#include<SDL/SDL_opengl.h>


#include "camera.h"
#include "terrain.h"


int running=true;
int ScreenWidth=1000;
int ScreenHeight=800;
Matrix projectionMatrix;
double AspectRatio=0;
SDL_Surface* Display_Surface=NULL;



bool toggle;
double mousex=0;
double mousey=0;
bool click=0;
bool rightclick=0;


Camera camera;
Terrain testterrain;


int lastmilliseconds=0;
int curmilliseconds=0;
double deltaT;

void UpdateTimer()
{
lastmilliseconds=curmilliseconds;
curmilliseconds=SDL_GetTicks();
deltaT=(double)(curmilliseconds-lastmilliseconds)/1000;
}


int Init()
{
AspectRatio=(double)(ScreenWidth)/(double)ScreenHeight;
if(SDL_Init(SDL_INIT_EVERYTHING)<0)return false;
Display_Surface=SDL_SetVideoMode(ScreenWidth,ScreenHeight,32,SDL_HWSURFACE|SDL_OPENGL);
SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);
if(Display_Surface==NULL)return false;

projectionMatrix=ProjectionMatrix(-1,1,-1/AspectRatio,1/AspectRatio,1,100000);
glClearColor(0,0.4,0.8,1);

glEnable(GL_DEPTH_TEST);
glEnable(GL_CULL_FACE);
glFrontFace(GL_CW);

InitialiseTerrainSystem();

camera=CreateCamera();
YawCamera(&camera,2);

<<<<<<< HEAD
testterrain=CreateTerrain("mountains.data");
=======
testterrain=CreateTerrain("testheightmap.png");
>>>>>>> 5e6720bb403a722512a7f149f2298ed10498eeba

return true;
}

void ProcessEvents()
{
SDL_Event Event;
while(SDL_PollEvent(&Event))
{
switch(Event.type)
{
case SDL_QUIT:
running=false;
break;
case SDL_KEYDOWN:
toggle=toggle?false:true;
//running=false;
break;
case SDL_MOUSEMOTION:
mousex=(double)Event.motion.x/ScreenWidth;
mousey=(double)Event.motion.y/ScreenHeight;
//printf("Mouse: %f,%f\n",mousex,mousey);
break;
case SDL_MOUSEBUTTONDOWN:
if(Event.button.button==SDL_BUTTON_LEFT)
{
click=true;
}
else if(Event.button.button==SDL_BUTTON_RIGHT)
{
rightclick=true;
}
break;
case SDL_MOUSEBUTTONUP:
if(Event.button.button==SDL_BUTTON_LEFT)
{
click=false;
}
else if(Event.button.button==SDL_BUTTON_RIGHT)
{
rightclick=false;
}
break;
}
}
}

void RunPhysics()
{
UpdateTimer();
printf("FPS: %f\n",1/deltaT);

//Do camera movement
YawCamera(&camera,(mousex-0.5)*2*deltaT);
PitchCamera(&camera,(mousey-0.5)*2*deltaT);

    if(click)
    {
    Vector forward;
    forward.X=0;
    forward.Y=0;
    forward.Z=-5500*deltaT;
    TranslateCamera(&camera,forward);
    }
    else if(rightclick)
    {
    Vector backward;
    backward.X=0;
    backward.Y=0;
    backward.Z=5500*deltaT;
    TranslateCamera(&camera,backward);
    }
}



void RenderScreen()
{
glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);


RenderTerrain(&testterrain,&camera,MatrixMultiply(projectionMatrix,MatrixFromCamera(&camera)));
SDL_GL_SwapBuffers();
glFinish();
}

void Quit()
{
FinishTerrainSystem(&testterrain);
SDL_Quit();
}

int main(int argc,char* argv[])
{
FILE* generator=popen("../TerrainGenerator/bin/Debug/TerrainGenerator","r");
    if(generator==NULL)exit(1);
Init();
camera.Position.Y=5000;
while(running)
{
ProcessEvents();
RunPhysics();
RenderScreen();


//int i;
float data[256*256*4];
float buffer[256*256*2];
fread(buffer,sizeof(float),256*256*2,generator);
int i;
    for(i=0;i<256*256*2;i+=2)
    {
    data[i*2]=buffer[i];
    data[1+i*2]=buffer[i+1];
    }
glBindTexture(GL_TEXTURE_2D,testterrain.heightMapTex);
glTexSubImage2D(GL_TEXTURE_2D,0,0,0,256,256,GL_RGBA,GL_FLOAT,data);
}
Quit();
//pclose(generator);
return 0;
}



/*
Old addnoiselayer, possibly still of interest
//printf("patchx: %d\n patchy: %d\n",patchx,patchy);
int vertex_gap=128/(points_per_side-1);
int vertex_step=scale/vertex_gap;
//printf("vertex_step: %d\n",vertex_step);



int offsetx=patchx*128;
int offsety=patchy*128;



//printf("roughness: %f %f %f %f\n",r1,r2,r3,r4);


float ux=0,uy=0;

printf("testing %d\n",vertex_step);
//float du=1/(float)((points_per_side-1)/vertex_step);

//printf("%f\n",du);


int i,j;
//Generate noise
    for(i=0;i<points_per_side;i++)
    {
        for(j=0;j<points_per_side;j++)
        {
        patch->vertices[(i*points_per_side)+j].location[1]+=(j*vertex_gap)+offsetx,(i*vertex_gap)+offsety,scale)*scale;//*BilinearInterp(r1,r2,r3,r4,ux,uy);
        //ux+=du;
        }
  //  uy+=du;
   // ux=0;
    }

/*
//Interpolate rows
int index;
float h=0,dh=0;
    for(i=0;i<points_per_side;i+=vertex_step)
    for(j=0;j<points_per_side-1;j++)//No need to execute for the last vertex in a row, as the value is irrelevent
    {
    index=(i*points_per_side)+j;
        if(j%vertex_step!=0)
        {
        h+=dh;
        vertices[index].location[1]=h;
        }
        else
        {
        h=vertices[index].location[1];
        dh=(vertices[index+vertex_step].location[1]-h)/vertex_step;
        }
    }

    //Interpolate columns
    for(j=0;j<points_per_side;j++)
    for(i=0;i<points_per_side-1;i++)//No need to execute for the last vertex in a column, as the value is irrelevent
    {
    index=(i*points_per_side)+j;
        if(i%vertex_step!=0)
        {
        h+=dh;
        vertices[index].location[1]=h;
        }
        else
        {
        h=vertices[index].location[1];
        dh=(vertices[index+(vertex_step*points_per_side)].location[1]-h)/vertex_step;
        }
    }
*/
