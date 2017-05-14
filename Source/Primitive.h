#ifndef __H_PRIMITIVE_H__
#define __H_PRIMITIVE_H__

#include "Material.h"

class Primitive {
public:
    bool isTriangle;
    bool isSphere;
    Material material;

    Primitive() {
        isTriangle = false;
        isSphere = false;
    }
};

#endif
