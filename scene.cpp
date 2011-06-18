#include "Scene.h"
#include "Config.h"
#include "Raytrace.h"
#include <iostream>
#include <cmath>
using namespace std;

const vector2 NullVector = { 0.0f,0.0f,0.0f };
const point Origin = { 0.0f,0.0f,0.0f };
const SimpleString emptyString("");

void GetMaterial(const Config &sceneFile, material &currentMat)
{    
    // diffuse color
    {
        float fScalar;
        fScalar =  float(sceneFile.GetByNameAsFloat("Diffuse", 0.0f)); 
        vector2 vColor = {fScalar, fScalar, fScalar};
        vColor = sceneFile.GetByNameAsVector("Diffuse", vColor);
        currentMat.diffuse.red   = vColor.x;
        currentMat.diffuse.green = vColor.y;
		currentMat.diffuse.blue  = vColor.z;
    }

    // Reflection color
    {
        float fScalar;
        fScalar =  float(sceneFile.GetByNameAsFloat("Reflection", 0.0f)); 
		currentMat.reflection = fScalar;
    }
	
	// Refraction color
    {
        float fScalar;
        fScalar =  float(sceneFile.GetByNameAsFloat("Refraction", 0.0f)); 
		currentMat.refraction = fScalar;
    }
	
	// Density color
    {
        float fScalar;
        fScalar =  float(sceneFile.GetByNameAsFloat("Density", 0.0f)); 
		currentMat.density = fScalar;
    }
}

void GetSphere(const Config &sceneFile, sphere &currentSph)
{
    currentSph.pos = sceneFile.GetByNameAsPoint("Center", Origin); 

    currentSph.size =  float(sceneFile.GetByNameAsFloat("Size", 0.0f)); 

    currentSph.materialId = sceneFile.GetByNameAsInteger("Material.Id", 0); 
}

void GetLight(const Config &sceneFile, light &currentLight)
{
    currentLight.pos = sceneFile.GetByNameAsPoint("Position", Origin); 

    // light color
    {
        float fScalar;
        fScalar =  float(sceneFile.GetByNameAsFloat("Intensity", 0.0f)); 
        vector2 vColor = {fScalar, fScalar, fScalar};
        vColor = sceneFile.GetByNameAsVector("Intensity", vColor);
		currentLight.intensity.red   = vColor.x;
        currentLight.intensity.green = vColor.y;
        currentLight.intensity.blue  = vColor.z;
    }
}

bool init(char* inputName, scene &myScene)
{
	int nbMats, nbSpheres, nbLights;
	int i;
	Config sceneFile(inputName);
    if (sceneFile.SetSection("Scene") == -1)
    {
		cout << "Mal formed Scene file : No Scene section." << endl;
		return false;
    }

    myScene.sizex = sceneFile.GetByNameAsInteger("Image.Width", 640);
    myScene.sizey = sceneFile.GetByNameAsInteger("Image.Height", 480);

    myScene.tonemap.fMidPoint = float(sceneFile.GetByNameAsFloat("Tonemap.Midpoint", 0.6f));
    if (myScene.tonemap.fMidPoint <= 0.0f || myScene.tonemap.fMidPoint >= 1.0f)
    {
        cout << "Mal formed Scene file : Mid point must be between 0 and 1." << endl;
		return false;
    }
    myScene.tonemap.fPower    = float(sceneFile.GetByNameAsFloat("Tonemap.Power", 3.0f));
    if (myScene.tonemap.fPower < 2.0f)
    {
        cout << "Mal formed Scene file : Tonemap power must be bigger than two." << endl;
		return false;
    }
    myScene.tonemap.fBlack    = float(sceneFile.GetByNameAsFloat("Tonemap.Black", 0.1f));
    if (myScene.tonemap.fBlack < 0.0f)
    {
        cout << "Mal formed Scene file : Tonemap black level cannot be negative." << endl;
		return false;
    }

    if (myScene.tonemap.fBlack != 0.0f)
    {
        // if we have three user defined parameters
        // there is one parameter left. It's the final scale of the exponent 
        // we define it to be such that the midlevel gray stays the same, no matter what the power
        // or the black level is (arbitrary). midpoint = 1 - exp(normExp) = 1 - exp(scale * normExp^power / (black + normExp^(power - 1)));
        float normExp = - logf(1.0f - myScene.tonemap.fMidPoint);
        myScene.tonemap.fPowerScale = - (1.0f + myScene.tonemap.fBlack / powf(normExp, myScene.tonemap.fPower - 1.0f));
    }
    else
    {
        myScene.tonemap.fPowerScale = -1.0f;
    }

    myScene.cm.name[cubemap::up] = sceneFile.GetByNameAsString("Cubemap.Up", emptyString);
    myScene.cm.name[cubemap::down] = sceneFile.GetByNameAsString("Cubemap.Down", emptyString);
    myScene.cm.name[cubemap::right] = sceneFile.GetByNameAsString("Cubemap.Right", emptyString);
    myScene.cm.name[cubemap::left] = sceneFile.GetByNameAsString("Cubemap.Left", emptyString);
    myScene.cm.name[cubemap::forward] = sceneFile.GetByNameAsString("Cubemap.Forward", emptyString);
    myScene.cm.name[cubemap::backward] = sceneFile.GetByNameAsString("Cubemap.Backward", emptyString);
    myScene.cm.bExposed = sceneFile.GetByNameAsBoolean("Cubemap.Exposed", false);
    myScene.cm.bsRGB = sceneFile.GetByNameAsBoolean("Cubemap.sRGB", false);
    myScene.cm.exposure = float(sceneFile.GetByNameAsFloat("Cubemap.Exposure", 1.0f));

    if (!myScene.cm.Init())
    {
        cout << "No skybox file found" << endl;
    }
    myScene.complexity = sceneFile.GetByNameAsInteger("Complexity", 1);

    {

        SimpleString perspectiveType = sceneFile.GetByNameAsString("Perspective.Type", emptyString);

        if (perspectiveType.compare("conic") == 0)
        {
            myScene.persp.FOV = float(sceneFile.GetByNameAsFloat("Perspective.FOV", 45.0f));
            if (myScene.persp.FOV <= 0.0f || myScene.persp.FOV >= 189.0f)
            {
			    cout << "Mal formed Scene file : Out of range FOV." << endl;
		        return false;
            }

            myScene.persp.type = perspective::conic;
            myScene.persp.clearPoint = float(sceneFile.GetByNameAsFloat("Perspective.ClearPoint", 1.0f));
            myScene.persp.dispersion = float(sceneFile.GetByNameAsFloat("Perspective.Dispersion", 0.0f));

            // Determine the projection plane distance with the FOV 

            myScene.persp.invProjectionDistance = 1.0f / (0.5f * myScene.sizex / tanf (float(PIOVER180) * 0.5f * myScene.persp.FOV));
        }
        else
        {
            // default
            myScene.persp.type = perspective::orthogonal;
            myScene.persp.invProjectionDistance = 0.0f;
        }
    }

    nbMats = sceneFile.GetByNameAsInteger("NumberOfMaterials", 0);
    nbSpheres = sceneFile.GetByNameAsInteger("NumberOfSpheres", 0);
    nbLights = sceneFile.GetByNameAsInteger("NumberOfLights", 0);

	myScene.materialContainer.resize(nbMats);
	myScene.sphereContainer.resize(nbSpheres);
	myScene.lightContainer.resize(nbLights);

	for (i=0; i<nbMats; ++i)
    {   
        material &currentMat = myScene.materialContainer[i];
        SimpleString sectionName("Material");
        sectionName.append((unsigned long) i);
        if (sceneFile.SetSection( sectionName ) == -1)
        {
			cout << "Mal formed Scene file : Missing Material section." << endl;
		    return false;
        }
        GetMaterial(sceneFile, currentMat);
    }
	for (i=0; i<nbSpheres; ++i)
    {   
        sphere &currentSphere = myScene.sphereContainer[i];
        SimpleString sectionName("Sphere");
        sectionName.append((unsigned long) i);
        if (sceneFile.SetSection( sectionName ) == -1)
        {
			cout << "Mal formed Scene file : Missing Sphere section." << endl;
		    return false;
        }
        GetSphere(sceneFile, currentSphere);
        if (currentSphere.materialId >= nbMats)
        {
			cout << "Mal formed Scene file : Material Id not valid." << endl;
		    return false;
        }

    }
	for (i=0; i<nbLights; ++i)
    {   
        light &currentLight = myScene.lightContainer[i];
        SimpleString sectionName("Light");
        sectionName.append((unsigned long) i);
        if (sceneFile.SetSection( sectionName ) == -1)
        {
			cout << "Mal formed Scene file : Missing Light section." << endl;
		    return false;
        }
        GetLight(sceneFile, currentLight);
        
    }

	return true;
}

