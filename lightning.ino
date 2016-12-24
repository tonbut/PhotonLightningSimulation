// This #include statement was automatically added by the Particle IDE.
#include "RGBdriver.h"

#define CLK 1//pins definitions for the driver        
#define DIO 0


RGBdriver Driver(CLK,DIO);
#define CLOCK_MS 20
Timer morphTimer(CLOCK_MS, adjustLights);
String lastData;

//storm parameters

int delayMin=4;
int delayMax=22;
int preFlashMin=3;
int preFlashMax=8;
int preFlashPeriodMin=40/CLOCK_MS;
int preFlashPeriodMax=200/CLOCK_MS;
float fadeFactor=0.75f;

//storm state variables
int stormTimeRemaining=0; // number of cycles before storm ends
int stormTimeSinceStart=0; // number of cycles since start of storm
int stormBuildUp=0; // number of cycles to build to fade in and out
int delayMedian=(delayMax+delayMin)/2; //current avg time between flash sequences
int delayRemaining=0; //number of cycles to next flash sequence
int innerDelayRemaining=-1; // number of cycles to next flash within sequence
int preFlashRemaining=0; // position in flash sequence
float brightness=0.0f; // current instantaneous flash brightness
float stormIntensity=0.0f;
float dayLightIntensity=1.0f;

void adjustLights()
{
    if (stormTimeRemaining>0)
    {   stormTimeRemaining--;
        stormTimeSinceStart++;
        
        if(stormTimeRemaining==0)
        {   //end cleanup
            setColor(0,1);
        }
    
        if (delayRemaining>0)
        {   delayRemaining--;
        }
        else
        {   
            //float intensity;
            if (stormTimeSinceStart<stormBuildUp)
            {   stormIntensity=((float)stormTimeSinceStart)/((float)stormBuildUp);
            }
            else if (stormTimeRemaining<stormBuildUp)
            {   stormIntensity=((float)stormTimeRemaining)/((float)stormBuildUp);
            }
            else
            {   stormIntensity=1.0f;
            }
            dayLightIntensity=1.0f-stormIntensity;
            if (stormIntensity<0.5f) stormIntensity=0.0f;
            else stormIntensity=(stormIntensity-0.5f)*2.0f;

            if (preFlashRemaining==0 && innerDelayRemaining<0)
            {   preFlashRemaining=random(preFlashMin,preFlashMax);
                innerDelayRemaining=0;
            }
            
            if (innerDelayRemaining==0 && preFlashRemaining>0)
            {   //initiate new flash
            
                brightness=(preFlashRemaining==1)?stormIntensity:stormIntensity*0.2f;
                preFlashRemaining--;
                innerDelayRemaining=random(preFlashPeriodMin,preFlashPeriodMax);
            }
            if (innerDelayRemaining>0)
            {   
                innerDelayRemaining--;
            }
            if (preFlashRemaining==0 && innerDelayRemaining==0)
            {   
                int dmin=delayMedian/2;
                int dmax=delayMedian*2;
                int d=random(dmin,dmax);
                delayRemaining=d*d;
                delayMedian=(delayMedian*2+random(delayMin,delayMax))/3;
                innerDelayRemaining=-1;
            }
        }
        
        if (brightness>0.01f)
        {
            setColor(brightness,dayLightIntensity);
            brightness=brightness*fadeFactor;
        }
        else if (brightness>0.0f)
        {   
            brightness=0.0f;
            setColor(brightness,dayLightIntensity);
        }
    }
}

int startStorm(String data)
{
    int duration = (int)strtol(data, NULL, 10);
    int buildUp=random(duration/8, duration/2);
    
    stormTimeRemaining=duration/CLOCK_MS;
    stormTimeSinceStart=0;
    stormBuildUp=buildUp/CLOCK_MS;
    delayRemaining=0;
    innerDelayRemaining=-1;
    preFlashRemaining=0;
    brightness=0.0f;
    stormIntensity=0.0f;
    dayLightIntensity=1.0f;
}

void updateData()
{
    char c[256];
    sprintf(c,"<data stormTimeRemaining=\"%d\" intensity=\"%.2f\" daylight=\"%.2f\" />", stormTimeRemaining, stormIntensity, dayLightIntensity
        
        );
    lastData=c;
}


void setup()  
{ 
    Particle.function("startStorm", startStorm);
    Particle.variable("data",lastData);
    morphTimer.start();
}  




void loop()
{
    updateData();
    delay(1000);
}

void setColor(int red, int green, int blue)
{
    Driver.begin(); // begin
    Driver.SetColor(red,green,blue); //Red. first node data
    Driver.end();
}

void setColor(float b, float d)
{   int v=(int)(b*255);
    int v2=32+(int)(d*128);
    int v3=v+v2;
    if (v3>255) v3=255;
    Driver.begin(); // begin
    Driver.SetColor(v,v,v3); //Red. first node data
    Driver.end();
}
