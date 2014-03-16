varying float altitude;
varying vec3 normal;


void main()
{
const float ambient=0.15;
float lambert=clamp(dot(normal,vec3(0.0,0.866,0.5)),0.0,1.0);
float radiance=ambient+lambert*0.2;
gl_FragColor=vec4(radiance,radiance,radiance,0.0);
}
