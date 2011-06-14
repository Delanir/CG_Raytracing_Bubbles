#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <limits>
#include <algorithm>
using namespace std;

#include "raytrace.h"
#include "scene.h"
#include "cubemap.h"

bool hitSphere(const ray &r, const sphere &s, float &t)
{ 
	// Intersection of a ray and a sphere
	// Check the articles for the rationale
	// NB : this is probably a naive solution
	// that could cause precision problems
	// but that will do it for now.
	vector2 dist = s.pos - r.start;
	float B = r.dir * dist;
	float D = B*B - dist * dist + s.size * s.size;
	if (D < 0.0f)
		return false;
	float t0 = B - sqrtf(D);
	float t1 = B + sqrtf(D);
	bool retvalue = false;
	if ((t0 > 0.1f) && (t0 < t))
	{
		t = t0;
		retvalue = true;
	}
	if ((t1 > 0.1f) && (t1 < t))
	{
		t = t1;
		retvalue = true;
	}
	return retvalue;
}

float srgbEncode(float c)
{
    if (c <= 0.0031308f)
    {
        return 12.92f * c; 
    }
    else
    {
        return 1.055f * powf(c, 0.4166667f) - 0.055f; // Inverse gamma 2.4
    }
}

color addRay(ray viewRay, scene &myScene, context myContext)
{
    color output = {0.0f, 0.0f, 0.0f}; 
    float coef = 1.0f;
    int level = 0;
    do 
    {
        point ptHitPoint;
        vector2 vNormal;
        material currentMat;
        {
            int currentSphere=-1;
            float t = 2000.0f;
            for (unsigned int i = 0; i < myScene.sphereContainer.size() ; ++i)
            {
                if (hitSphere(viewRay, myScene.sphereContainer[i], t))
                {
                    currentSphere = i;
                }
            }
			if (currentSphere != -1)
            {
                ptHitPoint  = viewRay.start + t * viewRay.dir;
                vNormal = ptHitPoint - myScene.sphereContainer[currentSphere].pos;
                float temp = vNormal * vNormal;
                if (temp == 0.0f)
                    break;
                temp = invsqrtf(temp);
                vNormal = temp * vNormal;
                currentMat = myScene.materialContainer[myScene.sphereContainer[currentSphere].materialId];
            }
            else
            {
                break;
            }
        }

        float bInside;

        if (vNormal * viewRay.dir > 0.0f)
        {
            vNormal = -1.0f * vNormal;
            bInside = true;
        }
        else
        {
            bInside = false;
        }

        /*if (currentMat.bump)
        {
            float noiseCoefx = float(noise(0.1 * double(ptHitPoint.x), 0.1 * double(ptHitPoint.y),0.1 * double(ptHitPoint.z)));
            float noiseCoefy = float(noise(0.1 * double(ptHitPoint.y), 0.1 * double(ptHitPoint.z),0.1 * double(ptHitPoint.x)));
            float noiseCoefz = float(noise(0.1 * double(ptHitPoint.z), 0.1 * double(ptHitPoint.x),0.1 * double(ptHitPoint.y)));
            
            vNormal.x = (1.0f - currentMat.bump ) * vNormal.x + currentMat.bump * noiseCoefx;  
            vNormal.y = (1.0f - currentMat.bump ) * vNormal.y + currentMat.bump * noiseCoefy;  
            vNormal.z = (1.0f - currentMat.bump ) * vNormal.z + currentMat.bump * noiseCoefz;  
            
            float temp = vNormal * vNormal;
            if (temp == 0.0f)
            break;
            temp = invsqrtf(temp);
            vNormal = temp * vNormal;
        }*/
        
        float fViewProjection = viewRay.dir * vNormal;
        float fReflectance, fTransmittance;
        float fCosThetaI, fSinThetaI, fCosThetaT, fSinThetaT;

        if(((currentMat.reflection != 0.0f) || (currentMat.refraction != 0.0f) ) && (currentMat.density != 0.0f))
        {
            // glass-like material, we're computing the fresnel coefficient.

            float fDensity1 = myContext.fRefractionCoef; 
            float fDensity2;
            if (bInside)
            {
                // We only consider the case where the ray is originating a medium close to the void (or air) 
                // In theory, we should first determine if the current object is inside another one
                // but that's beyond the purpose of our code.
                fDensity2 = context::getDefaultAir().fRefractionCoef;
            }
            else
            {
                fDensity2 = currentMat.density;
            }

            // Here we take into account that the light movement is symmetrical
            // From the observer to the source or from the source to the oberver.
            // We then do the computation of the coefficient by taking into account
            // the ray coming from the viewing point.
            fCosThetaI = fabsf(fViewProjection); 

            if (fCosThetaI >= 0.999f) 
            {
                // In this case the ray is coming parallel to the normal to the surface
                fReflectance = (fDensity1 - fDensity2) / (fDensity1 + fDensity2);
                fReflectance = fReflectance * fReflectance;
                fSinThetaI = 0.0f;
                fSinThetaT = 0.0f;
                fCosThetaT = 1.0f;
            }
            else 
            {
                fSinThetaI = sqrtf(1 - fCosThetaI * fCosThetaI);
                // The sign of SinThetaI has no importance, it is the same as the one of SinThetaT
                // and they vanish in the computation of the reflection coefficient.
                fSinThetaT = (fDensity1 / fDensity2) * fSinThetaI;
                if (fSinThetaT * fSinThetaT > 0.9999f)
                {
                    // Beyond that angle all surfaces are purely reflective
                    fReflectance = 1.0f ;
                    fCosThetaT = 0.0f;
                }
                else
                {
                    fCosThetaT = sqrtf(1 - fSinThetaT * fSinThetaT);
                    // First we compute the reflectance in the plane orthogonal 
                    // to the plane of reflection.
                    float fReflectanceOrtho = (fDensity2 * fCosThetaT - fDensity1 * fCosThetaI ) 
                        / (fDensity2 * fCosThetaT + fDensity1  * fCosThetaI);
                    fReflectanceOrtho = fReflectanceOrtho * fReflectanceOrtho;
                    // Then we compute the reflectance in the plane parallel to the plane of reflection
                    float fReflectanceParal = (fDensity1 * fCosThetaT - fDensity2 * fCosThetaI )
                        / (fDensity1 * fCosThetaT + fDensity2 * fCosThetaI);
                    fReflectanceParal = fReflectanceParal * fReflectanceParal;

                    // The reflectance coefficient is the average of those two.
                    // If we consider a light that hasn't been previously polarized.
                    fReflectance =  0.5f * (fReflectanceOrtho + fReflectanceParal);
                }
            }
        }
        else
        {
            // Reflection in a metal-like material. Reflectance is equal in all directions.
            // Note, that metal are conducting electricity and as such change the polarity of the
            // reflected ray. But of course we ignore that..
            fReflectance = 1.0f;
            fCosThetaI = 1.0f;
            fCosThetaT = 1.0f;
        }

        fTransmittance = currentMat.refraction * (1.0f - fReflectance);
        fReflectance = currentMat.reflection * fReflectance;

        float fTotalWeight = fReflectance + fTransmittance;
        bool bDiffuse = false;

        if (fTotalWeight > 0.0f)
        {
            float fRoulette = (1.0f / RAND_MAX) * rand();
        
            if (fRoulette <= fReflectance)
            {
                coef *= currentMat.reflection;

                float fReflection = - 2.0f * fViewProjection;

                viewRay.start = ptHitPoint;
                viewRay.dir += fReflection * vNormal;
            }
            else if(fRoulette <= fTotalWeight)
            {
                coef *= currentMat.refraction;
                float fOldRefractionCoef = myContext.fRefractionCoef;
                if (bInside) 
                {
                    myContext.fRefractionCoef = context::getDefaultAir().fRefractionCoef;
                }
                else
                {
                    myContext.fRefractionCoef = currentMat.density;
                }

                // Here we compute the transmitted ray with the formula of Snell-Descartes
                viewRay.start = ptHitPoint;

                viewRay.dir = viewRay.dir + fCosThetaI * vNormal;
                viewRay.dir = (fOldRefractionCoef / myContext.fRefractionCoef) * viewRay.dir;
                viewRay.dir += (-fCosThetaT) * vNormal;
            }
            else
            {
                bDiffuse = true;
            }
        }
        else
        {
            bDiffuse = true;
        }


        if (!bInside && bDiffuse)
        {
            // Now the "regular lighting"

            ray lightRay;
            lightRay.start = ptHitPoint;
            for (unsigned int j = 0; j < myScene.lightContainer.size() ; ++j)
            {
                light currentLight = myScene.lightContainer[j];

                lightRay.dir = currentLight.pos - ptHitPoint;
                float fLightProjection = lightRay.dir * vNormal;

                if ( fLightProjection <= 0.0f )
                    continue;

                float lightDist = lightRay.dir * lightRay.dir;
                {
                    float temp = lightDist;
                    if ( temp == 0.0f )
                        continue;
                    temp = invsqrtf(temp);
                    lightRay.dir = temp * lightRay.dir;
                    fLightProjection = temp * fLightProjection;
                }

                bool inShadow = false;
                {
                    float t = lightDist;
                    for (unsigned int i = 0; i < myScene.sphereContainer.size() ; ++i)
                    {
                        if (hitSphere(lightRay, myScene.sphereContainer[i], t))
                        {
                            inShadow = true;
                            break;
                        }
                    }
                }

                if (!inShadow && (fLightProjection > 0.0f))
                {

                    float lambert = (lightRay.dir * vNormal) * coef;

					output.red += lambert * currentLight.intensity.red * currentMat.diffuse.red;
					output.green += lambert * currentLight.intensity.green * currentMat.diffuse.green;
					output.blue += lambert * currentLight.intensity.blue * currentMat.diffuse.blue;


                    // Blinn 
                    // The direction of Blinn is exactly at mid point of the light ray 
                    // and the view ray. 
                    // We compute the Blinn vector and then we normalize it
                    // then we compute the coeficient of blinn
                    // which is the specular contribution of the current light.

                    vector2 blinnDir = lightRay.dir - viewRay.dir;
                    float temp = blinnDir * blinnDir;
                    if (temp != 0.0f )
                    {
                        float blinn = invsqrtf(temp) * max(fLightProjection - fViewProjection , 0.0f);
                        blinn = coef * powf(blinn, currentMat.power);
                        output += blinn *currentMat.specular  * currentLight.intensity;
                    }
                }
            }
            coef = 0.0f ;
        }

        level++;
    } while ((coef > 0.0f) && (level < 10));  

    if (coef > 0.0f)
    {
        output += coef * readCubemap(myScene.cm, viewRay);
    }
    return output;
}

float AutoExposure(scene &myScene)
{
    #define ACCUMULATION_SIZE 16
    float exposure = -1.0f;
    float accufacteur = float(max(myScene.sizex, myScene.sizey));

    accufacteur = accufacteur / ACCUMULATION_SIZE;

    float mediumPoint = 0.0f;
    const float mediumPointWeight = 1.0f / (ACCUMULATION_SIZE*ACCUMULATION_SIZE);
    for (int y = 0; y < ACCUMULATION_SIZE; ++y) {
        for (int x = 0 ; x < ACCUMULATION_SIZE; ++x) {

            if (myScene.persp.type == perspective::orthogonal)
            {
                ray viewRay = { {float(x)*accufacteur, float(y) * accufacteur, -1000.0f}, { 0.0f, 0.0f, 1.0f}};
                color currentColor = addRay (viewRay, myScene, context::getDefaultAir());
                float luminance = 0.2126f * currentColor.red
                                + 0.715160f * currentColor.green
                                + 0.072169f * currentColor.blue;
                mediumPoint = mediumPoint + mediumPointWeight * (luminance * luminance);
            }
            else
            {
                vector2 dir = {(float(x)*accufacteur - 0.5f * myScene.sizex) * myScene.persp.invProjectionDistance, 
                                (float(y) * accufacteur - 0.5f * myScene.sizey) * myScene.persp.invProjectionDistance, 
                                1.0f}; 

                float norm = dir * dir;
                // I don't think this can happen but we've never too prudent
                if (norm == 0.0f) 
                    break;
                dir = invsqrtf(norm) * dir;

                ray viewRay = { {0.5f * myScene.sizex,  0.5f * myScene.sizey, 0.0f}, {dir.x, dir.y, dir.z} };
                color currentColor = addRay (viewRay, myScene, context::getDefaultAir());
                float luminance = 0.2126f * currentColor.red
                                + 0.715160f * currentColor.green
                                + 0.072169f * currentColor.blue;
                mediumPoint = mediumPoint + mediumPointWeight * (luminance * luminance);
            }
        }
    }
    
    float mediumLuminance = sqrtf(mediumPoint);

    if (mediumLuminance > 0.0f)
    {
        exposure = - logf(1.0f - myScene.tonemap.fMidPoint) / mediumLuminance;
    }

    return exposure;
}

bool draw(char* outputName, scene &myScene)
{
    int x, y;
    ofstream imageFile(outputName,ios_base::binary);
    if (!imageFile)
        return false;
    // Addition of the TGA header
    imageFile.put(0).put(0);
    imageFile.put(2);        /* RGB not compressed */

    imageFile.put(0).put(0);
    imageFile.put(0).put(0);
    imageFile.put(0);

    imageFile.put(0).put(0); /* origin X */ 
    imageFile.put(0).put(0); /* origin Y */

    imageFile.put((unsigned char)(myScene.sizex & 0x00FF)).put((unsigned char)((myScene.sizex & 0xFF00) / 256));
    imageFile.put((unsigned char)(myScene.sizey & 0x00FF)).put((unsigned char)((myScene.sizey & 0xFF00) / 256));
    imageFile.put(24);                 /* 24 bit bitmap */
    imageFile.put(0);
    // end of the TGA header 

    float exposure = AutoExposure(myScene);

    for (y = 0; y < myScene.sizey; ++y)
    for (x = 0 ; x < myScene.sizex; ++x)
    {
        if (y < 10)
        {
            // Use ten lines in the final image as an intensity calibration hint
            if ((x / 10) & 1)
            {
                imageFile.put((unsigned char)186).put((unsigned char)186).put((unsigned char)186);
            }
            else if ( y & 1)
            {
                imageFile.put((unsigned char)255).put((unsigned char)255).put((unsigned char)255);
            }
            else
            {
                imageFile.put((unsigned char)0).put((unsigned char)0).put((unsigned char)0);
            }
        }
        else
        {
            color output = {0.0f, 0.0f, 0.0f};
            for (float fragmentx = float(x) ; fragmentx < x + 1.0f; fragmentx += 0.5f )
            for (float fragmenty = float(y) ; fragmenty < y + 1.0f; fragmenty += 0.5f )
            {
                float sampleRatio = 0.25f;
                color temp = {0.0f, 0.0f, 0.0f};
                float fTotalWeight = 0.0f;

                if (myScene.persp.type == perspective::orthogonal)
                {
                    ray viewRay = { {fragmentx, fragmenty, -10000.0f}, { 0.0f, 0.0f, 1.0f}};
                    for (int i = 0; i < myScene.complexity; ++i)
                    {                  
                        color rayResult = addRay (viewRay, myScene, context::getDefaultAir());
                        fTotalWeight += 1.0f; 
                        temp += rayResult;
                    }
                    temp = (1.0f / fTotalWeight) * temp;
                }
                else
                {
                    vector2 dir = {(fragmentx - 0.5f * myScene.sizex) * myScene.persp.invProjectionDistance, 
                                (fragmenty - 0.5f * myScene.sizey) * myScene.persp.invProjectionDistance, 
                                1.0f}; 

                    float norm = dir * dir;
                    if (norm == 0.0f) 
                        break;
                    dir = invsqrtf(norm) * dir;
                    // the starting point is always the optical center of the camera
                    // we will add some perturbation later to simulate a depth of field effect
                    point start = {0.5f * myScene.sizex,  0.5f * myScene.sizey, 0.0f};
                    // The point aimed is one of the invariant of the current pixel
                    // that means that by design every ray that contribute to the current
                    // pixel must go through that point in space (on the "sharp" plane)
                    // of course the divergence is caused by the direction of the ray itself.
                    point ptAimed = start + myScene.persp.clearPoint * dir;

                    for (int i = 0; i < myScene.complexity; ++i)
                    {                  
                        ray viewRay = { {start.x, start.y, start.z}, {dir.x, dir.y, dir.z} };

                        if (myScene.persp.dispersion != 0.0f)
                        {
                            vector2 vDisturbance;                        
                            vDisturbance.x = (myScene.persp.dispersion / RAND_MAX) * (1.0f * rand());
                            vDisturbance.y = (myScene.persp.dispersion / RAND_MAX) * (1.0f * rand());
                            vDisturbance.z = 0.0f;

                            viewRay.start = viewRay.start + vDisturbance;
                            viewRay.dir = ptAimed - viewRay.start;
                            
                            norm = viewRay.dir * viewRay.dir;
                            if (norm == 0.0f)
                                break;
                            viewRay.dir = invsqrtf(norm) * viewRay.dir;
                        }
                        color rayResult = addRay (viewRay, myScene, context::getDefaultAir());
                        fTotalWeight += 1.0f;
                        temp += rayResult;
                    }
                    temp = (1.0f / fTotalWeight) * temp;
                }
                
                // pseudo photo exposure
                temp.blue   *= exposure;
                temp.red    *= exposure;
                temp.green  *= exposure;

                if (myScene.tonemap.fBlack > 0.0f)
                {
                    temp.blue   = 1.0f - expf(myScene.tonemap.fPowerScale * powf(temp.blue, myScene.tonemap.fPower)   
                                              / (myScene.tonemap.fBlack + powf(temp.blue, myScene.tonemap.fPower - 1.0f)) );
                    temp.red    = 1.0f - expf(myScene.tonemap.fPowerScale * powf(temp.red, myScene.tonemap.fPower)    
                                              / (myScene.tonemap.fBlack + powf(temp.red, myScene.tonemap.fPower - 1.0f)) );
                    temp.green  = 1.0f - expf(myScene.tonemap.fPowerScale * powf(temp.green, myScene.tonemap.fPower)  
                                              / (myScene.tonemap.fBlack + powf(temp.green, myScene.tonemap.fPower - 1.0f)) );
                }
                else
                {
                    // If the black level is 0 then all other parameters have no effect
                    temp.blue   = 1.0f - expf(myScene.tonemap.fPowerScale * temp.blue);
                    temp.red    = 1.0f - expf(myScene.tonemap.fPowerScale * temp.red);
                    temp.green  = 1.0f - expf(myScene.tonemap.fPowerScale * temp.green);
                }
                
                output += sampleRatio * temp;
            }
			
			// gamma correction
            output.blue = srgbEncode(output.blue);
            output.red = srgbEncode(output.red);
            output.green = srgbEncode(output.green);

            imageFile.put((unsigned char)min(output.blue*255.0f,255.0f)).put((unsigned char)min(output.green*255.0f, 255.0f)).put((unsigned char)min(output.red*255.0f, 255.0f));
        }
    }
    return true;
}

int __cdecl main(int argc, char* argv[])
{
    if (argc < 3)
    {
        cout << "Usage : Raytrace.exe Scene.txt Output.tga" << endl;
        return -1;
    }
    scene myScene;
    if (!init(argv[1], myScene))
    {
        cout << "Failure when reading the Scene file." << endl;
        return -1;
    }
    if (!draw(argv[2], myScene))
    {
        cout << "Failure when creating the image file." << endl;
        return -1;
    }
    return 0;
}
