#version 130
uniform mat4 modelViewProjection;
uniform float scale;
uniform vec2 displacement;

varying float altitude;
varying float lambert;


//My simplex noise
const uint gradients[8]=uint[8](0xEu,0x4u,0xCu,0x8u,0xDu,0x5u,0xFu,0xAu);
const float skewFactor=0.366025404;//0.5*(sqrt(3.0)-1.0)
float unskewFactor=0.211324865;//(3.0-sqrt(3.0))/6.0



vec2 Skew(vec2 vec)
{
float s=(vec.x+vec.y)*skewFactor;

vec.x+=s;
vec.y+=s;
return vec;
}

vec2 Unskew(float x,float y)
{
float s=(x+y)*unskewFactor;

vec2 v;
v.x=x-s;
v.y=y-s;
return v;
}


vec2 GetGradient(vec2 position)
{
//I don't know how it works, but it does
float rand=fract(sin(dot(position,vec2(12.9898,78.233))) * 43758.5453)*6.28;
vec2 gradient;
gradient.x=sin(rand);
gradient.y=cos(rand);
return gradient;
}

float SimplexNoise(in vec2 coords,in float scale,in float maxvalue,inout vec2 gradient)
{
coords/=scale;
//Transform the coordinates
vec2 skewed=Skew(coords);

//Get the corner point of the grid cell that we are in
vec2 corner;
corner.x=floor(skewed.x);
corner.y=floor(skewed.y);


vec2 points[3];

vec2 gradients[3];
//First and last points are independent of the simplex we are in
points[0]=Unskew(corner.x,corner.y);
gradients[0]=GetGradient(corner);

//Calculate which simplex we are in
    if(skewed.x-corner.x>skewed.y-corner.y)
    {
    //Lower simplex
    points[1]=Unskew(corner.x+1,corner.y);
    gradients[1]=GetGradient(corner+vec2(1,0));
    }
    else
    {
    //Upper simplex
    points[1]=Unskew(corner.x,corner.y+1);
    gradients[1]=GetGradient(corner+vec2(0,1));
    }
points[2]=Unskew(corner.x+1,corner.y+1);
gradients[2]=GetGradient(corner+vec2(1,1));

//We now have the three corner points of the simplex
float result=0.0;
vec2 derivative=vec2(0.0,0.0);

int i;
    for(i=0;i<3;i++)
    {
    vec2 rel_pos=coords-points[i];
    //Compute contribution of this Vector2D to result
    float t=0.5-rel_pos.x*rel_pos.x-rel_pos.y*rel_pos.y;
    //Only values of t greater than zero are considered
        if(t>0.0)
        {
        //Compute t^4
        float t2=t*t;
        float t4=t2*t2;
        float temp=-8.0f*t2*(gradients[i].x*rel_pos.x+gradients[i].y*rel_pos.y);
        derivative.x+=temp*rel_pos.x+t4*gradients[i].x;
        derivative.y+=temp*rel_pos.y+t4*gradients[i].y;
        result+=t4*dot(rel_pos,gradients[i]);
        }
    }

gradient+=(100.0*maxvalue/scale)*derivative;
return 100.0*maxvalue*result;
}



void main()
{
//Transform the vertex into world space
vec4 worldSpacePosition=vec4(scale*gl_Vertex.xyz,1.0);
worldSpacePosition.xz+=displacement;

vec4 displacedPosition=worldSpacePosition;
//Add simplex noise
vec2 gradient=vec2(0.0,0.0);
displacedPosition.y+=SimplexNoise(worldSpacePosition.xz,128.0,16.0,gradient);
displacedPosition.y+=SimplexNoise(worldSpacePosition.xz,64.0,8.0,gradient);
displacedPosition.y+=SimplexNoise(worldSpacePosition.xz,32.0,2.0,gradient);
displacedPosition.y+=SimplexNoise(worldSpacePosition.xz,16.0,1.0,gradient);
displacedPosition.y+=SimplexNoise(worldSpacePosition.xz,8.0,0.5,gradient);
displacedPosition.y+=SimplexNoise(worldSpacePosition.xz,4.0,0.25,gradient);

vec3 normal=normalize(vec3(-gradient.x,1.0,-gradient.y));

lambert=clamp(dot(normal,vec3(0.0,0.866,0.5)),0.0,1.0);

gl_Position=modelViewProjection*displacedPosition;
}
