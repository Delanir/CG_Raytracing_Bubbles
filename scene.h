/*
    This file belongs to the Ray tracing tutorial of http://www.codermind.com/
    It is free to use for educational purpose and cannot be redistributed
    outside of the tutorial pages.
    Any further inquiry :
    mailto:info@codermind.com
 */

#ifndef __SCENE_H
#define __SCENE_H
#include <vector>
#include "raytrace.h"

struct scene {
    std::vector<material> materialContainer;
	std::vector<sphere>   sphereContainer;
	std::vector<light>    lightContainer;
    int sizex, sizey;
};

bool init(char* inputName, scene &myScene);

#endif // __SCENE_H
