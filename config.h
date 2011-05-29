/*
    This file belongs to the Ray tracing tutorial of http://www.codermind.com/
    It is free to use for educational purpose and cannot be redistributed
    outside of the tutorial pages.
    Any further inquiry :
    mailto:info@codermind.com
 */

#ifndef __CONFIG_H
#define __CONFIG_H

// This is a simple config file parser.
// It does what we need no more no less.

#include "raytrace.h"
#include "SimpleString.h"

// Some code
class Config {
private:
    void * m_pVariables;
    void * m_pSections;
    const SimpleString m_sFileName;
    SimpleString m_sCurrentSection;
    bool m_bLoaded;
public:
    // When the variable called "sName" doesn't exit, you will get "default" 
    bool GetByNameAsBoolean(const SimpleString  & sName, bool bDefault) const;
    double GetByNameAsFloat(const SimpleString & sName, double fDefault) const;
    const SimpleString &GetByNameAsString(const SimpleString  &sName, const SimpleString  & sDefault) const;
    long GetByNameAsInteger(const SimpleString  &sName, long lDefault) const;
    vector2 GetByNameAsVector(const SimpleString &sName, const vector2& vDefault) const;
    point GetByNameAsPoint(const SimpleString &sName, const point& ptDefault) const;
    
    // SetSection will return -1 when the section wasn't found. 
    int SetSection(const SimpleString &sName);
    ~Config();
    Config(const SimpleString &sFileName);
};

#endif //__CONFIG_H
