/*
    This file belongs to the Ray tracing tutorial of http://www.codermind.com/
    It is free to use for educational purpose and cannot be redistributed
    outside of the tutorial pages.
    Any further inquiry :
    mailto:info@codermind.com
 */

#ifndef __CUBEMAP_H
#define __CUBEMAP_H

color readTexture(const color* tab, float u, float v, int sizeU, int sizeV);

bool dummyTGAHeader(ifstream &currentfile, int &sizeX, int &sizeY);

color readCubemap(const cubemap & cm, const ray &myRay);

#endif  //__CUBEMAP_H
