#ifndef __H_POINTLIGHT_H
#define __H_POINTLIGHT_H

#define TORAD(x) x * M_PI * 2.f

#include "Intersection.h"
#include "Ray.h"
#include "TestModel.h"

using namespace std;
using namespace glm;

class PointLight {
public:
    vec3 position;
    vec3 color;
    vector<Primitive*>* primitives;

    PointLight(vec3 position, vec3 color)
    : position(position), color(color) {
    }

    vec3 CalculateColor(Pixel& point) {
        vec3 color(0,0,0), directLight, indirectLight;

        point.luminance = DirectLight(point.pos3d, point.normal);
        indirectLight = vec3(0.5, 0.5, 0.5);

        // Texture mapping
        if (point.triangle->material.texture) {
            // Render using texture

            if (point.triangle->material.textureImage == NULL) {
                vec3 purple (.75f, 0, .75f);
                vec3 black (0, 0, 0);
                bool u = (int) (point.u * 10) % 2;
                bool v = (int) (point.v * 10) % 2;
                v = !v;
                point.diffuse = u != v ? purple : black;
            } else {
                int x = point.triangle->material.textureImage->w * point.u;
                int y = point.triangle->material.textureImage->h * point.v;
                point.diffuse = GetPixelSDL(point.triangle->material.textureImage, x, y);
            }
        } else {
            // Use triangle diffuse color
            point.diffuse = point.triangle->material.diffuse;
        }

        color = (point.luminance + indirectLight) * point.diffuse;
        return color;
    }

    vec3 DirectLight(
        const vec3& point,
        const vec3& normal
    ) {
        vec3 directLight;
        Intersection directIntersect;
        Intersection lightIntersect;
        bool found = false;

        if (turnOnShadow) {
            // Create ray towards light source
            Ray shadowRay (
                point + normal*0.0001f, this->position - point
            );

            found = Intersection::ClosestIntersection(
                shadowRay, *primitives, lightIntersect, -1
            );
        }

        // If intersection exist, no direct light, else calculate direct light
        if (
            found &&
            distance(lightIntersect.position, point) <
            distance(this->position, point)
        ) {
            directLight = vec3(0, 0, 0);
        } else {

            vec3 R = this->position - point;
            float r = sqrt(R[0] * R[0] + R[1] * R[1] + R[2] * R[2]);
            float product = dot(R, normal);
            if (product < 0) {
                product = 0;
            }

            directLight = product * this->color / (float) (4.0 * M_PI * r * r);
        }

        return directLight;
    }

    vec3 CalculateLuminance(
        const vec3& point,
        const vec3& normal,
        const vec3& dir
    ) {
        vec3 directLight;
        Intersection directIntersect;

        float r = distance(this->position, point);
        float product = dot(dir, normal);
        if (product < 0) {
            product = 0;
        }

        directLight = product * this->color / (float) (4.0 * M_PI * r * r);

        return directLight;
    }
};

#endif
