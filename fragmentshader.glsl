varying float lambert;


void main()
{
const float ambient=0.15;
float radiance=ambient+lambert*0.2;
gl_FragColor=vec4(radiance,radiance,radiance,0.0);
}
