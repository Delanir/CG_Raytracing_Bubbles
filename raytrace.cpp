
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <limits>
#include <algorithm>
#include <stack>
using namespace std;
#include "raytrace.h"
#include "scene.h"
#include "cubemap.h"



stack <float> refractionN;

int maxDepth=0;

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

int raytrace(ray viewRay, color& c_color, scene &myScene,int depth, float ni){
	color output = {0.0f, 0.0f, 0.0f}; 
    float coef = 1.0f;
	float reflectance;
	float transmitance;
	if(depth>maxDepth ){
		return -1;
	}
	
	point ptHitPoint;
	vector2 vNormal;
	material currentMat;
	bool bInside;
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
		if (temp == 0.0f){
			return -1;
		}
		temp = invsqrtf(temp);
		vNormal = temp * vNormal;
		currentMat = myScene.materialContainer[myScene.sphereContainer[currentSphere].materialId];
	}
	else
	{
		output += coef * readCubemap(myScene.cm, viewRay); // the ray ends up hitting the skybox
		c_color=output;
		return 0;
	}
	float nr ;
	if (vNormal * viewRay.dir > 0.0f)
	{
		vNormal = -1.0f * vNormal;
		bInside = true;
		
		nr=refractionN.top();
		refractionN.pop();
	}
	else
	{
		nr= currentMat.density;
		
		bInside = false;
	}
	
	reflectance=0.0f;
	transmitance=currentMat.refraction;
	
	float cos_i= vNormal*((-1)*viewRay.dir);
	float n = ni / nr;
	//cout << n<< endl;
	
	float cos_r;/*
	if(cos_i  >= 0.999f){
		//cout << "HERE" << endl;
		// In this case the ray is coming parallel to the normal to the surface
			
			reflectance=n*n;
			if(reflectance>1.0f)
				reflectance=1.0f;
			reflectance *= reflectance; 
			cos_r = 1.0f;
			transmitance*=(1-reflectance);
			//cout << transmitance<< endl;
	}
	*/ 
	if(transmitance>0.0f){
		float sin_r=(1.0f/n)*sin(acos(cos_i));
		cos_r= sqrtf( 1.0 -  (n*n *(1-cos_i*cos_i)) );
		
		/*if (sin_r*sin_r > 0.9999f)
		{
			// Beyond that angle all surfaces are purely reflective
			reflectance = 1.0f ;
			cos_r = 0.0f;
		}else{
			float fReflectanceOrtho = (nr * cos_r - ni * cos_i ) 
				/ (nr * cos_r + ni  * cos_i);
			fReflectanceOrtho = fReflectanceOrtho * fReflectanceOrtho;
			// Then we compute the reflectance in the plane parallel to the plane of reflection
			float fReflectanceParal = (ni * cos_r - nr*cos_i )
				/ (ni * cos_r - nr*cos_i);
			fReflectanceParal = fReflectanceParal * fReflectanceParal;

			// The reflectance coefficient is the average of those two.
			// If we consider a light that hasn't been previously polarized.
			reflectance =  0.5f * (fReflectanceOrtho + fReflectanceParal);
		
		}
		transmitance= transmitance * (1-reflectance);
		*/
	}
	
	if(transmitance > 0.0f ){

		ray refracted;
		refracted.start=ptHitPoint;
			
		
		refracted.dir = (n*viewRay.dir)+((cos_i*n)*vNormal)+((-cos_r)*vNormal);
		
		
		
		
		
		color refracted_color;
		refractionN.push(nr);
		if(raytrace(refracted ,refracted_color, myScene,depth+1, nr)!=-1){
			
			output= transmitance * refracted_color +output;
			
		}
	
	}
	reflectance=currentMat.reflection+transmitance*reflectance;
	if( reflectance >0.0f){
		ray reflected;
		reflected.start=ptHitPoint;
		reflected.dir=viewRay.dir+(2*cos_i)*vNormal;
		
		color reflected_color;
		if(raytrace(reflected ,reflected_color, myScene,depth+1, ni)!=-1){
			
			output= reflectance * reflected_color +output;
			//cout<< output.red;
		}
	
	
	}

	c_color=output;
	
	return 0;
	
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
                color currentColor;
				refractionN.push(1.0);
				raytrace (viewRay, currentColor, myScene, 0, 1.0);
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
                color currentColor;
				refractionN.push(1.0);
				raytrace (viewRay, currentColor, myScene, 0, 1.0);
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
	maxDepth = myScene.complexity;
	
    for (y = 0; y < myScene.sizey; ++y)
    for (x = 0 ; x < myScene.sizex; ++x)
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
                                    
					color rayResult;
					refractionN.push(1.0);
					raytrace(viewRay, rayResult, myScene, 0, 1.0);
					fTotalWeight += 1.0f; 
					temp += rayResult;
                    
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
					color rayResult;
		
					refractionN.push(1.0);
					raytrace(viewRay, rayResult, myScene, 0, 1.0);
					
					fTotalWeight += 1.0f;
					temp += rayResult;
                    
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
