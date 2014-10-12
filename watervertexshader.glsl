#version 130
uniform mat4 modelViewProjection;
uniform float scale;
uniform vec2 displacement;
uniform sampler2D heightmap;

varying float lambert;


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

float point00=texelFetch(heightmap,coords,0).g;
float point01=texelFetch(heightmap,coords+ivec2(0,1),0).g;
float point10=texelFetch(heightmap,coords+ivec2(1,0),0).g;
float point11=texelFetch(heightmap,coords+ivec2(1,1),0).g;
float point0=mix(point00,point01,fract(worldSpacePosition.z/128.0));
float point1=mix(point10,point11,fract(worldSpacePosition.z/128.0));
float point=mix(point0,point1,fract(worldSpacePosition.x/128.0));
//Compute gradient
gradient.x=mix(point11-point10,point01-point00,fract(worldSpacePosition.x/256))/128.0;
gradient.y=mix(point11-point01,point10-point00,fract(worldSpacePosition.z/256))/128.0;

displacedPosition.y+=point-1.0;


vec3 normal=normalize(vec3(-gradient.x,1.0,-gradient.y));
lambert=clamp(dot(normal,vec3(0.0,0.866,0.5)),0.0,1.0);

gl_Position=modelViewProjection*displacedPosition;
}
