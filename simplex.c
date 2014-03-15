typedef struct
{
float X;
float Y;
}Vector2D;

const unsigned char gradients[8]={0xE,0x4,0xC,0x8,0xD,0x5,0xF,0xA};
const float skewFactor=0.5*(sqrt(3.0)-1.0);
const float unskewFactor=(3.0-sqrt(3.0))/6.0;


inline Vector2D Skew(float x,float y)
{
float s=(x+y)*skewFactor;

Vector2D v;
v.X=x+s;
v.Y=y+s;
return v;
}

inline Vector2D Unskew(float x,float y)
{
float s=(x+y)*unskewFactor;

Vector2D v;
v.X=x-s;
v.Y=y-s;
return v;
}

unsigned int randint(unsigned int seed)
{
return (seed*1103515245+12345)%451958711;
}

unsigned char GetGradient(unsigned int x,unsigned int y,unsigned int seed)
{
return gradients[(randint(x+randint(y+randint(seed)))>>16)&0x7];
}

inline float DotWithGradient(float x,float y,unsigned char gradient)
{
float result=0.0;
    if(gradient&0x8)
    {
        if(gradient&0x2)result-=x;
        else result+=x;
    }
    if(gradient&0x7)
    {
        if(gradient&0x1)result-=y;
        else result+=y;
    }
return result;
}


float SimplexNoise(float x,float y,unsigned int seed)
{
//int h;
//for(h=0;h<100;h++)printf("%d\n",(randint(h)>>9));
//Transform the coordinates
Vector2D skewed=Skew(x,y);

//Get the corner point of the grid cell that we are in
Vector2D corner;
corner.X=floor(skewed.X);
corner.Y=floor(skewed.Y);

int int_corner_x=(int)floor(corner.X);
int int_corner_y=(int)floor(corner.Y)
;
Vector2D points[3];
unsigned char gradients[3];
//First and last points are independent of the simplex we are in
points[0]=Unskew(corner.X,corner.Y);
gradients[0]=GetGradient(int_corner_x,int_corner_y,seed);
//Calculate which simplex we are in
    if(skewed.X-corner.X>skewed.Y-corner.Y)
    {
    //Lower simplex
    points[1]=Unskew(corner.X+1,corner.Y);
    gradients[1]=GetGradient(int_corner_x+1,int_corner_y,seed);
    }
    else
    {
    //Upper simplex
    points[1]=Unskew(corner.X,corner.Y+1);
    gradients[1]=GetGradient(int_corner_x,int_corner_y+1,seed);
    }
points[2]=Unskew(corner.X+1,corner.Y+1);
gradients[2]=GetGradient(int_corner_x+1,int_corner_y+1,seed);

//We now have the three corner points of the simplex
float result=0.0;
int i;
    for(i=0;i<3;i++)
    {
    float x_rel=x-points[i].X;
    float y_rel=y-points[i].Y;
    //Compute contribution of this Vector2D to result
    float t=0.5-x_rel*x_rel-y_rel*y_rel;
    //Only values of t greater than zero are considered
        if(t>0.0)
        {
        //Compute t^4
        t*=t;
        t*=t;
        result+=t*DotWithGradient(x_rel,y_rel,gradients[i]);
        }
    }
return 70.0*result;
}
