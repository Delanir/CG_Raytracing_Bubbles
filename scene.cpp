/*
    This file belongs to the Ray tracing tutorial of http://www.codermind.com/
    It is free to use for educational purpose and cannot be redistributed
    outside of the tutorial pages.
    Any further inquiry :
    mailto:info@codermind.com
 */

#include "Scene.h"
#include "Config.h"
#include "Raytrace.h"
#include <iostream>
using namespace std;

#define SCENE_VERSION_MAJOR 1
#define SCENE_VERSION_MINOR 0

const vector2 NullVector = { 0.0f,0.0f,0.0f };
const point Origin = { 0.0f,0.0f,0.0f };
const SimpleString emptyString("");
const SimpleString diffuseString ("Diffuse");
const SimpleString intensityString ("Intensity");

void GetMaterial(const Config &sceneFile, material &currentMat)
{    
    // diffuse color
    {
        float fScalar;
        fScalar =  float(sceneFile.GetByNameAsFloat(diffuseString, 0.0f)); 
        vector2 vColor = {fScalar, fScalar, fScalar};
        vColor = sceneFile.GetByNameAsVector(diffuseString, vColor);
        currentMat.red   = vColor.x;
        currentMat.green = vColor.y;
		currentMat.blue  = vColor.z;
    }

    // Reflection color
    {
        float fScalar;
        fScalar =  float(sceneFile.GetByNameAsFloat("Reflection", 0.0f)); 
		currentMat.reflection = fScalar;
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
        fScalar =  float(sceneFile.GetByNameAsFloat(intensityString, 0.0f)); 
        vector2 vColor = {fScalar, fScalar, fScalar};
        vColor = sceneFile.GetByNameAsVector(intensityString, vColor);
		currentLight.red   = vColor.x;
        currentLight.green = vColor.y;
        currentLight.blue  = vColor.z;
    }
}

bool init(char* inputName, scene &myScene)
{
	int nbMats, nbSpheres, nbLights, versionMajor, versionMinor;
	int i;
	Config sceneFile(inputName);
    if (sceneFile.SetSection("Scene") == -1)
    {
		cout << "Mal formed Scene file : No Scene section." << endl;
		return false;
    }

	versionMajor = sceneFile.GetByNameAsInteger("Version.Major", 0);
	versionMinor = sceneFile.GetByNameAsInteger("Version.Minor", 0);

	if (versionMajor != SCENE_VERSION_MAJOR || versionMinor != SCENE_VERSION_MINOR)
	{
        cout << "Mal formed Scene file : Wrong scene file version." << endl;
		return false;
	}

    myScene.sizex = sceneFile.GetByNameAsInteger("Image.Width", 640);
    myScene.sizey = sceneFile.GetByNameAsInteger("Image.Height", 480);

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

