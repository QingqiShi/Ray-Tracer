#ifndef __H_PIXEL_H__
#define __H_PIXEL_H__

#include "TestModel.h"

class Pixel {
public:
    // Pixel position in screen space
    vec2 pos;

    // Position in world space
    vec3 pos3d;

    // Inverse of depth of the point corresponding to pixel
    float invZ;

    // The triangle of the belonging point
    Triangle* triangle;

    // UV space coordinates for the triangle
    float u, v;

    // Normal of the point
    vec3 normal;

    // Luminance of the point
    vec3 luminance;

    // Diffuse color of the point
    vec3 diffuse;

    // Color for the Pixel
    vec3 color;

    Pixel() {
        triangle = NULL;
        invZ = 0;
        u = 0;
        v = 0;
    }

    Pixel& operator=(const Pixel& p) {
        this->pos = p.pos;
        this->pos3d = p.pos3d;
        this->invZ = p.invZ;
        this->triangle = p.triangle;
        this->u = p.u;
        this->v = p.v;
        this->normal = p.normal;
        this->luminance = p.luminance;
        this->diffuse = p.diffuse;
        this->color = p.color;

        return *this;
    }
};

#endif
