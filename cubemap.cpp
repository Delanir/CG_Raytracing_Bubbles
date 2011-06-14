/*
    This file belongs to the Ray tracing tutorial of http://www.codermind.com/
    It is free to use for educational purpose and cannot be redistributed
    outside of the tutorial pages.
    Any further inquiry :
    mailto:info@codermind.com
 */

#include <cmath>
#include <fstream>

using namespace std;

#include "raytrace.h"


color readTexture(const color* tab, float u, float v, int sizeU, int sizeV)
{
    u = fabsf(u);
    v = fabsf(v);
    int umin = int(sizeU * u);
    int vmin = int(sizeV * v);
    int umax = int(sizeU * u) + 1;
    int vmax = int(sizeV * v) + 1;
    float ucoef = fabsf(sizeU * u - umin);
    float vcoef = fabsf(sizeV * v - vmin);
    
    // The texture is being addressed on [0,1]
    // There should be an addressing type in order to 
    // determine how we should access texels when
    // the coordinates are beyond those boundaries.

    // Clamping is our current default and the only
    // implemented addressing type for now.
    // Clamping is done by bringing anything below zero
    // to the coordinate zero
    // and everything beyond one, to one.
    umin = min(max(umin, 0), sizeU - 1);
    umax = min(max(umax, 0), sizeU - 1);
    vmin = min(max(vmin, 0), sizeV - 1);
    vmax = min(max(vmax, 0), sizeV - 1);

    // What follows is a bilinear interpolation
    // along two coordinates u and v.

    color output = 
        (1.0f - vcoef) * 
        ((1.0f - ucoef) * tab[umin  + sizeU * vmin] 
        + ucoef * tab[umax + sizeU * vmin])
        +   vcoef * 
        ((1.0f - ucoef) * tab[umin  + sizeU * vmax] 
        + ucoef * tab[umax + sizeU * vmax]);
    return output;
}


bool dummyTGAHeader(ifstream &currentfile, int &sizeX, int &sizeY)
{
	char dummy;
	char temp;
	currentfile.get(dummy).get(dummy);
	currentfile.get(temp);                  /* uncompressed RGB */
	if (temp!=2)
		return false;
	currentfile.get(dummy).get(dummy);
	currentfile.get(dummy).get(dummy);
	currentfile.get(dummy);
	currentfile.get(dummy).get(dummy);           /* origin X */
	currentfile.get(dummy).get(dummy);           /* origin Y */
	currentfile.get(temp);
	sizeX = temp;
	currentfile.get(temp);
	sizeX += temp * 256;

	currentfile.get(temp);
	sizeY = temp;
	currentfile.get(temp);
	sizeY += temp * 256;

    currentfile.get(temp);                 /* 24 bit bitmap */
	currentfile.get(dummy);
	return true;
}

bool cubemap::Init()
{
    if (texture)
    {
        return false;
    }
    ifstream currentfile ;
    color * currentColor;
    int x,y, dummySizeX, dummySizeY;

    currentfile.open(name[up].c_str(), ios_base::binary);
    if ((!currentfile)||(!dummyTGAHeader(currentfile, sizeX, sizeY)))
        return false;
    if (sizeX <= 0 || sizeY <= 0)
        return false;
    texture = new color[size_t(sizeX * sizeY * 6)];

    int number = cubemap::up * sizeX * sizeY;
    currentColor = 	texture + number;
    for (y = 0; y < sizeY; y++)
    for (x = 0; x < sizeX; x++)
    {
        currentColor->blue = currentfile.get() /255.0f;
        currentColor->green = currentfile.get() /255.0f;
        currentColor->red = currentfile.get() /255.0f;
        currentColor++;
    }
    currentfile.close();
	
    for (unsigned i = cubemap::down; i <= cubemap::backward; ++i)
    {
        number = i * sizeX * sizeY;
        currentColor = 	texture + number;
        currentfile.open(name[i].c_str(), ios_base::binary);
        if ((!currentfile)||
            (!dummyTGAHeader(currentfile, dummySizeX, dummySizeY)) ||
            sizeX != dummySizeX || 
            sizeY != dummySizeY)
        {
            // The textures for each face have to be of the same size..
            delete [] texture;
            texture = 0;
            return false;
        }

        for (y = 0; y < sizeY; y++)
        for (x = 0; x < sizeX; x++)
        {
            currentColor->blue = currentfile.get() /255.0f;
            currentColor->green = currentfile.get() /255.0f;
            currentColor->red = currentfile.get() /255.0f;
            currentColor++;
        }
        currentfile.close();
    }

    return true;
}

color readCubemap(const cubemap & cm, const ray &myRay)
{
    color * currentColor ;
    color outputColor = {0.0f,0.0f,0.0f};
    if(!cm.texture)
    {
        return outputColor;
    }
    if ((fabsf(myRay.dir.x) >= fabsf(myRay.dir.y)) && (fabsf(myRay.dir.x) >= fabsf(myRay.dir.z)))
    {
        if (myRay.dir.x > 0.0f)
        {
            currentColor = cm.texture + cubemap::right * cm.sizeX * cm.sizeY;
            outputColor = readTexture(currentColor,  
                1.0f - (myRay.dir.z / myRay.dir.x+ 1.0f) * 0.5f,  
                (myRay.dir.y / myRay.dir.x+ 1.0f) * 0.5f, cm.sizeX, cm.sizeY);
        }
        else if (myRay.dir.x < 0.0f)
        {
            currentColor = cm.texture + cubemap::left * cm.sizeX * cm.sizeY;
            outputColor = readTexture(currentColor,  
                1.0f - (myRay.dir.z / myRay.dir.x+ 1.0f) * 0.5f,
                1.0f - ( myRay.dir.y / myRay.dir.x + 1.0f) * 0.5f,  
                cm.sizeX, cm.sizeY);
        }
    }
    else if ((fabsf(myRay.dir.y) >= fabsf(myRay.dir.x)) && (fabsf(myRay.dir.y) >= fabsf(myRay.dir.z)))
    {
        if (myRay.dir.y > 0.0f)
        {
            currentColor = cm.texture + cubemap::up * cm.sizeX * cm.sizeY;
            outputColor = readTexture(currentColor,  
                (myRay.dir.x / myRay.dir.y + 1.0f) * 0.5f,
                1.0f - (myRay.dir.z/ myRay.dir.y + 1.0f) * 0.5f, cm.sizeX, cm.sizeY);
        }
        else if (myRay.dir.y < 0.0f)
        {
            currentColor = cm.texture + cubemap::down * cm.sizeX * cm.sizeY;
            outputColor = readTexture(currentColor,  
                1.0f - (myRay.dir.x / myRay.dir.y + 1.0f) * 0.5f,  
                (myRay.dir.z/myRay.dir.y + 1.0f) * 0.5f, cm.sizeX, cm.sizeY);
        }
    }
    else if ((fabsf(myRay.dir.z) >= fabsf(myRay.dir.x)) && (fabsf(myRay.dir.z) >= fabsf(myRay.dir.y)))
    {
        if (myRay.dir.z > 0.0f)
        {
            currentColor = cm.texture + cubemap::forward * cm.sizeX * cm.sizeY;
            outputColor = readTexture(currentColor,  
                (myRay.dir.x / myRay.dir.z + 1.0f) * 0.5f,  
                (myRay.dir.y/myRay.dir.z + 1.0f) * 0.5f, cm.sizeX, cm.sizeY);
        }
        else if (myRay.dir.z < 0.0f)
        {
            currentColor = cm.texture + cubemap::backward * cm.sizeX * cm.sizeY;
            outputColor = readTexture(currentColor,  
                (myRay.dir.x / myRay.dir.z + 1.0f) * 0.5f,  
                1.0f - (myRay.dir.y /myRay.dir.z+1) * 0.5f, cm.sizeX, cm.sizeY);
        }
    }
    if (cm.bsRGB)
    {
       // We make sure the data that was in sRGB storage mode is brought back to a 
       // linear format. We don't need the full accuracy of the sRGBEncode function
       // so a powf should be sufficient enough.
       outputColor.blue   = powf(outputColor.blue, 2.2f);
       outputColor.red    = powf(outputColor.red, 2.2f);
       outputColor.green  = powf(outputColor.green, 2.2f);
    }

    if (cm.bExposed)
    {
        // The LDR (low dynamic range) images were supposedly already
        // exposed, but we need to make the inverse transformation
        // so that we can expose them a second time.
        outputColor.blue  = -logf(1.001f - outputColor.blue);
        outputColor.red   = -logf(1.001f - outputColor.red);
        outputColor.green = -logf(1.001f - outputColor.green);
    }

    outputColor.blue  /= cm.exposure;
    outputColor.red   /= cm.exposure;
    outputColor.green /= cm.exposure;

    return outputColor;
}
