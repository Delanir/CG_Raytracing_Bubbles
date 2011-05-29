/*
    This file belongs to the Ray tracing tutorial of http://www.codermind.com/
    It is free to use for educational purpose and cannot be redistributed
    outside of the tutorial pages.
    Any further inquiry :
    mailto:info@codermind.com
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <limits>
#include <algorithm>
using namespace std;

#include "raytrace.h"
#include "scene.h"

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

bool draw(char* outputName, scene &myScene)
{
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
	imageFile.put(24);       /* 24 bit bitmap */
	imageFile.put(0);
	// end of the TGA header

	// Scanning
	for (int y = 0; y < myScene.sizey; ++y) {
		for (int x = 0; x < myScene.sizex; ++x) {
			float red = 0, green = 0, blue = 0;
			float coef = 1.0f;
			int level = 0;
			// Cast the ray
			// Because we are not in conic perspective (point of origin)
			// but in orthographic perspective, there is no natural starting point
			// We have to put it far enough to enclose the whole scene
			// but not too far to avoid floating point precision problems (acne and other)
			// 1000.0f seems like a good compromise for now..
			ray viewRay = { {float(x), float(y), -1000.0f}, { 0.0f, 0.0f, 1.0f}};
			do
			{
				// Looking for the closest intersection
				float t = 2000.0f;
				int currentSphere= -1;

				for (unsigned int i = 0; i < myScene.sphereContainer.size(); ++i)
				{
					if (hitSphere(viewRay, myScene.sphereContainer[i], t))
					{
						currentSphere = i;
					}
				}

				if (currentSphere == -1)
					break;

				point newStart = viewRay.start + t * viewRay.dir;

				// What is the normal vector at the point of intersection ?
				// It's pretty simple because we're dealing with spheres
				vector2 n = newStart - myScene.sphereContainer[currentSphere].pos;
				float temp = n * n;
				if (temp == 0.0f)
					break;

				temp = 1.0f / sqrtf(temp);
				n = temp * n;

				material currentMat = myScene.materialContainer[myScene.sphereContainer[currentSphere].materialId];

				for (unsigned int j = 0; j < myScene.lightContainer.size(); ++j) {
					light current = myScene.lightContainer[j];
					vector2 dist = current.pos - newStart;
					if (n * dist <= 0.0f)
						continue;
					float t = sqrtf(dist * dist);
					if ( t <= 0.0f )
						continue;
					ray lightRay;
					lightRay.start = newStart;
					lightRay.dir = (1/t) * dist;
					// computation of the shadows
					bool inShadow = false;
					for (unsigned int i = 0; i < myScene.sphereContainer.size(); ++i) {
						if (hitSphere(lightRay, myScene.sphereContainer[i], t)) {
							inShadow = true;
							break;
						}
					}
					if (!inShadow) {
						// lambert
						float lambert = (lightRay.dir * n) * coef;
						red += lambert * current.red * currentMat.red;
						green += lambert * current.green * currentMat.green;
						blue += lambert * current.blue * currentMat.blue;
					}
				}

				// We iterate on the next reflection
				coef *= currentMat.reflection;
				float reflet = 2.0f * (viewRay.dir * n);
				viewRay.start = newStart;
				viewRay.dir = viewRay.dir - reflet * n;

				level++;
			}
			while ((coef > 0.0f) && (level < 10));

			imageFile.put((unsigned char)min(blue*255.0f,255.0f)).put((unsigned char)min(green*255.0f, 255.0f)).put((unsigned char)min(red*255.0f, 255.0f));
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
