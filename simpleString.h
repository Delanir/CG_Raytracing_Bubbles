/*
    This file belongs to the Ray tracing tutorial of http://www.codermind.com/
    It is free to use for educational purpose and cannot be redistributed
    outside of the tutorial pages.
    Any further inquiry :
    mailto:info@codermind.com
 */

#ifndef __SIMPLE_STRING_H
#define __SIMPLE_STRING_H

#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cstdio>
#include <climits>

// A simple string class that fills just what we need no more no less

#define COMPILE_TIME_ASSERT(pred)            \
    switch(0){case 0:case (pred):;}

const unsigned int DefaultPreferedStringSize = 1 << 8; 

class SimpleString{
private:
    char *m_Array;
    int m_ReservedSize;
    int m_Size;
    
    void _grow(int newReservedSize) {
        if (m_ReservedSize ==0) {
            m_ReservedSize = DefaultPreferedStringSize;
            while(newReservedSize > m_ReservedSize) {
                m_ReservedSize *= 2;// next power of two..
            }
            m_Array= new char[size_t(m_ReservedSize)];
            m_Array[0] = '\0';
            return;
        } else {
            while(newReservedSize > m_ReservedSize) {
                m_ReservedSize *= 2;// next power of two..
            }
            char *temp = new char[size_t(m_ReservedSize)];
            memcpy(temp, m_Array, size_t(m_Size + 1));
            delete [] m_Array;
            m_Array = temp;
            return;
        }
    };
    
public:
    typedef char* iterator;
    typedef const char* const_iterator;
    
    SimpleString() : m_Array(NULL), m_ReservedSize(0), m_Size(0) {
        _grow(m_ReservedSize);
    };
    
    SimpleString(const SimpleString &source) : m_Array(NULL), m_ReservedSize(0), m_Size(0) {
        assign(source);
    };
    
    /*explicit*/ SimpleString(const char * source) : m_Array(NULL), m_ReservedSize(0), m_Size(0) {
        assign(source);
    };
    
    SimpleString(const char *source, int length) : m_Array(NULL), m_ReservedSize(0), m_Size(0) {
        assign(source, length);
    };
    
    SimpleString(const_iterator first, const_iterator last) : m_Array(NULL), m_ReservedSize(0), m_Size(0) {
        assign(first, last);
    };
    
    SimpleString& operator = (const SimpleString &source) {
        return assign(source); 
    };
    
    SimpleString& operator = (const char * source) {
        return assign(source);
    };

    ~SimpleString() {
        delete [] m_Array;
    };
    
    SimpleString & assign(const char *source) {
        int length = int(strlen(source));
        if (length + 1> m_ReservedSize) {
            _grow(length + 1);
        }
        memcpy(m_Array, source, size_t(length + 1));
        m_Size = length;
        return *this;
    };

    SimpleString & assign(const char *source, int length) {
        if (length + 1> m_ReservedSize) {
            _grow(length + 1);
        }
        memcpy(m_Array, source, length + 1);
        m_Size = length;
        m_Array[length] = '\0';
        return *this;
    };

    SimpleString & assign(const_iterator first, const_iterator last) {
        int length = int(last - first);
        if (length < 0) length = 0;
        if (length + 1 > m_ReservedSize) {
            _grow(length + 1);
        }
        memcpy(m_Array, first, length);
        m_Size = length;
        m_Array[length] = '\0';
        return *this;
    };

    SimpleString & assign(const SimpleString &source) {
        int length = source.size();
        if (length + 1> m_ReservedSize) {
            _grow(length + 1);
        }        
        memcpy(m_Array, source.c_str(), length + 1);
        m_Size = length;
        return *this;
    };
    
    SimpleString & append(const char *source) {
        int length = int(strlen(source));
        if (m_Size + length + 1> m_ReservedSize) {
            _grow(m_Size + length + 1);
        }
        memcpy(m_Array + m_Size, source, size_t(length + 1));
        m_Size += length;
        return *this;
    };

    SimpleString & append(const char *source, int length) {
        if (m_Size + length + 1> m_ReservedSize) {
            _grow(m_Size +length + 1);
        }
        memcpy(m_Array + m_Size, source, length + 1);
        m_Size += length;
        m_Array[m_Size] = '\0';
        return *this;
    };

    SimpleString & append(const_iterator first, const_iterator last) {
        int length = int(last - first);
        if (length < 0) length = 0;
        if (m_Size + length + 1 > m_ReservedSize) {
            _grow(m_Size + length + 1);
        }
        memcpy(m_Array + m_Size, first, size_t(length));
        m_Size += length;
        m_Array[m_Size] = '\0';
        return *this;
    };

    SimpleString & append(const SimpleString &source) {
        int length = source.size();
        if (m_Size + length + 1> m_ReservedSize) {
            _grow(m_Size + length + 1);
        }        
        memcpy(m_Array + m_Size, source.c_str(), size_t(length + 1));
        m_Size += length;
        return *this;
    };

    SimpleString & append(char _c) {
        if (m_Size + 2> m_ReservedSize) {
            _grow(m_Size + 2);
        }
        m_Array[m_Size] = _c;
        m_Size++;
        m_Array[m_Size] = '\0';
        return *this;
    };

    SimpleString & append(int _i) {
        return append(static_cast<long>(_i));
    };
    
    SimpleString & append(unsigned int _i) {
        return append(static_cast<unsigned long>(_i));
    };

    SimpleString & append(long _i) {
        char buf[40]; // _itoa will store upto 33 bytes so it's safe!!
        COMPILE_TIME_ASSERT (LONG_MAX < 1e23);
        sprintf(buf, "%ld", _i);

        int length = int(strlen(buf));

        if (m_Size + length + 1> m_ReservedSize) {
            _grow(m_Size + length + 1);
        }
        memcpy(m_Array + m_Size, buf, size_t(length + 1));
        m_Size += length;
        return *this;
    };

    SimpleString & append(unsigned long _i) {
        char buf[40]; // _itoa will store upto 33 bytes so it's safe!!
        COMPILE_TIME_ASSERT (LONG_MAX < 1e23);
        sprintf(buf, "%lu", _i);

        int length = int(strlen(buf));

        if (m_Size + length + 1> m_ReservedSize) {
            _grow(m_Size + length + 1);
        }
        memcpy(m_Array + m_Size, buf, size_t(length + 1));
        m_Size += length;
        return *this;
    };

    const char * c_str() const{
        return m_Array;
    };

    int size() const{
        return m_Size;
    };

    bool empty() const{
        return (m_Size == 0);
    }
    
    int compare(const SimpleString &source) const {
        return strcmp(m_Array, source.c_str());
    }
    int compare(const char * source) const {
        return strcmp(m_Array, source);
    }

    void resize(int newSize) {
        if (newSize + 1 > m_ReservedSize) {
            _grow(newSize + 1);
        }        
        m_Size = newSize;
        m_Array[m_Size] = '\0';
    }

    iterator begin(){
        return m_Array;
    }

    iterator end() {
        return m_Array + m_Size;
    }

    SimpleString substr(int pos, int n) const{
        return SimpleString(m_Array + pos, m_Array + pos + n);
    };

    int find_last_of(char _c) const {
        int i = int(m_Size - 1);
        while (i >= 0) {
            if (_c == m_Array[i])
                return i;
            i--;
        }

        return -1; // not found
    }
};

inline bool operator <(const SimpleString& _str1, const SimpleString& _str2) {
    return strcmp( _str1.c_str(), _str2.c_str()) < 0;
}

#endif //__SIMPLE_STRING_H
