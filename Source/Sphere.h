#ifndef __H_SPHERE_H__
#define __H_SPHERE_H__

#include "Primitive.h"

class Sphere : public Primitive {
public:
    vec3 position;
    float radius;

    Sphere (vec3 position, float radius, vec3 color)
    : position(position), radius(radius) {
        this->isSphere = true;
        this->material.diffuse = color;
    }
};

#endif
