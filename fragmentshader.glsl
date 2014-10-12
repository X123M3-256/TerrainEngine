varying float lambert;
varying float water;

void main()
{
const float ambient=0.15;
float radiance=ambient+lambert*0.2;


gl_FragColor=vec4((1.0-water)*radiance,(1.0-water)*radiance,radiance,0.0);
}
