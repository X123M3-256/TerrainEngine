uniform sampler2D noise;

uniform sampler2D grass;
uniform sampler2D rock;

varying vec3 normal;
varying vec3 position;




vec4 select(float edge,float x,vec4 a,vec4 b)
{
return step(edge,x)*(a-b)+b;
}

void main()
{
const float ambient=0.25;

vec4 base_color=texture2D(rock,position.xz);
vec4 surface_color=texture2D(grass,position.xz);




float coverage=clamp((dot(normal,vec3(0.0,1.0,0.0))-0.5)*3.0,0.0,1.0);
float covered=step(texture2D(noise,position.xz).r,coverage);
vec4 color=mix(base_color,surface_color,covered);






float lambert=clamp(dot(normal,vec3(0.0,0.866,0.5)),0.0,1.0);

float radiance=ambient+lambert*0.2;
gl_FragColor=select(1.0,position.y,radiance*color,vec4(0.0,0.0,1.0,0.0));
}
