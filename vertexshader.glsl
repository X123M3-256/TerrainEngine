#version 130
uniform mat4 modelViewProjection;
uniform float scale;
uniform vec2 displacement;
uniform sampler2D heightmap;

varying float lambert;


const uint gradients[8]=uint[8](0xEu,0x4u,0xCu,0x8u,0xDu,0x5u,0xFu,0xAu);
const float skewFactor=0.366025404;//0.5*(sqrt(3.0)-1.0)
float unskewFactor=0.211324865;//(3.0-sqrt(3.0))/6.0



vec2 Skew(vec2 vec)
{
float s=(vec.x+vec.y)*skewFactor;
vec+=s;
return vec;
}

vec2 Unskew(vec2 vec)
{
float s=(vec.x+vec.y)*unskewFactor;
vec-=s;
return vec;
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

float SimplexNoise(in vec2 coords,in float scale,in float maxvalue,inout vec2 totalderivative)
{
coords/=scale;
//Transform the coordinates
vec2 skewed=Skew(coords);

//Get the corner point of the grid cell that we are in
vec2 skewedCorner;
skewedCorner.x=floor(skewed.x);
skewedCorner.y=floor(skewed.y);

vec2 corner=Unskew(skewedCorner);

vec2 points[3];
vec2 gradients[3];
//First and last points are independent of the simplex we are in
points[0]=corner;
gradients[0]=GetGradient(skewedCorner);
//Calculate which simplex we are in
points[1]=corner-unskewFactor;
    if(coords.x-corner.x>coords.y-corner.y)
    {
    //Lower simplex
    points[1].x+=1.0;
    gradients[1]=GetGradient(vec2(skewedCorner.x+1.0,skewedCorner.y));
    }
    else
    {
    //Upper simplex
    points[1].y+=1.0;
    gradients[1]=GetGradient(vec2(skewedCorner.x,skewedCorner.y+1.0));
    }
points[2]=corner+1.0-2*unskewFactor;
gradients[2]=GetGradient(skewedCorner+1.0);
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

totalderivative+=(100.0*maxvalue/scale)*derivative;
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

//Linear filtering may not work so we implement it here
ivec2 coords=ivec2(worldSpacePosition.xz/128.0);

float point00=texelFetch(heightmap,coords,0).r;
float point01=texelFetch(heightmap,coords+ivec2(0,1),0).r;
float point10=texelFetch(heightmap,coords+ivec2(1,0),0).r;
float point11=texelFetch(heightmap,coords+ivec2(1,1),0).r;
float point0=mix(point00,point01,fract(worldSpacePosition.z/128.0));
float point1=mix(point10,point11,fract(worldSpacePosition.z/128.0));
float point=mix(point0,point1,fract(worldSpacePosition.x/128.0));
//Compute gradient
gradient.x=mix(point11-point10,point01-point00,fract(worldSpacePosition.x/256))/128.0;
gradient.y=mix(point11-point01,point10-point00,fract(worldSpacePosition.z/256))/128.0;

displacedPosition.y+=point;

float roughness=0.1;
displacedPosition.y+=SimplexNoise(worldSpacePosition.xz,256.0,roughness*64.0,gradient);
displacedPosition.y+=SimplexNoise(worldSpacePosition.xz,64.0,roughness*16.0,gradient);
displacedPosition.y+=SimplexNoise(worldSpacePosition.xz,16.0,roughness*4.0,gradient);



vec3 normal=normalize(vec3(-gradient.x,1.0,-gradient.y));

lambert=clamp(dot(normal,vec3(0.0,0.866,0.5)),0.0,1.0);

gl_Position=modelViewProjection*displacedPosition;
}
