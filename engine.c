#include "engine.h"


//*********************************************************************************************
//*****************************************General engine code*********************************
//*********************************************************************************************


char* ReadFileString(char* filename)
{
FILE* file;
file=fopen(filename,"rb");
    if(file==NULL)return NULL;
fseek(file,0,SEEK_END);
long length=ftell(file);
fseek(file,0,SEEK_SET);
char* buf=malloc(length+1);
fread(buf,length,1,file);
fclose(file);
buf[length]=0;
return buf;
}


float GetRandomFloat()
{
return ((double)(rand()%65536))/65536;
}
