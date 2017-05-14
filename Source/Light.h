#ifndef __H_LIGHT_H
#define __H_LIGHT_H

#define GLM_FORCE_RADIANS

#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <random>
#include "Intersection.h"

#define TORAD(x) x * M_PI * 2.f

using namespace std;
using namespace glm;

class Light {
public:
    vec3 position;
    vec3 color;

    Light (vec3 position, vec3 color)
    : position(position), color(color) {
    }

    vec3 CalculateColor (
        const Intersection& intersect,
        const vector<Primitive*>& primitives,
        int depth,
        int maxDepth,
        int numRays
    ) {
        // End of recursion condition
        if (depth > maxDepth) {
            return vec3(0, 0, 0);
        }

        vec3 color, directLight, indirectLight;

        directLight = DirectLight(intersect, primitives);
        indirectLight = vec3(0.5, 0.5, 0.5);

        // color = (1.f / (float)(depth + 1) * indirectLight + directLight) * triangles[intersect.triangleIndex].color;
        color = (indirectLight + directLight) * intersect.primitive->material.diffuse;

        return color;
    }

    virtual vec3 DirectLight (
        const Intersection& pointIntersect, const vector<Primitive*>& primitives
    ) {
        Intersection lightIntersect;
        vec3 directLight;

        // Create ray towards light source
        Ray shadowRay (
            pointIntersect.position, this->position - pointIntersect.position
        );

        bool found = Intersection::ClosestIntersection(
            shadowRay, primitives, lightIntersect, pointIntersect.primitiveIndex
        );

        // If intersection exist, no direct light, else calculate direct light
        if (
            found &&
            distance(lightIntersect.position, pointIntersect.position) <
            distance(this->position, pointIntersect.position)
        ) {
            directLight = vec3(0, 0, 0);
        } else {
            vec3 R = position - pointIntersect.position;
            float r = length(R);
            float product = dot(R, pointIntersect.normal);
            if (product < 0) {
                product = 0;
            }

            directLight = (this->color * product) / (float) (4.0 * M_PI * r * r);
        }

        return directLight;
    }
};

class FlatSquareLight : public Light {
public:
    float width;
    vec3 v0, v1; // two corners of the square

    default_random_engine generator;
    uniform_real_distribution<float> distribution;

    FlatSquareLight (
        vec3 position,
        vec3 color,
        float width
    ) : Light(position, color), width(width), distribution(0, 1) {

        v0 = vec3(position.x - width / 2.f, position.y, position.z - width / 2.f);
        v1 = vec3(position.x + width / 2.f, position.y, position.z + width / 2.f);
    }

    vec3 CalculateColor (
        const Intersection& pointIntersect,
        const vector<Primitive*>& primitives,
        int depth,
        int maxDepth,
        int numRays,
        int sample
    ) {
        // End of recursion condition
        if (depth > maxDepth) {
            return vec3(0, 0, 0);
        }

        vec3 color, reflect, refract, diffuse;

        if (pointIntersect.primitive->material.isReflective) {
            reflect = CalculateReflective(
                pointIntersect, primitives, depth, maxDepth, numRays, sample
            );

            float reflectStrength = pointIntersect.primitive->material.reflectStrength;
            if (reflectStrength < 1.f) {
                diffuse = CalculateDiffuse(
                    pointIntersect, primitives, depth, maxDepth, numRays, sample
                );

                color = reflect * reflectStrength + diffuse * (1 - reflectStrength);
            } else {
                color = reflect;
            }
        } else if (pointIntersect.primitive->material.isRefractive) {
            refract = CalculateRefractive(
                pointIntersect, primitives, depth, maxDepth, numRays, sample
            );

            reflect = CalculateReflective(
                pointIntersect, primitives, depth, maxDepth, numRays, sample
            );

            float Fr = CalculateFresnel(
                pointIntersect.ray.d,
                pointIntersect.normal,
                pointIntersect.primitive->material.ior
            );
            float Ft = 1.f - Fr;

            color = refract * Ft + reflect * Fr;
            //color = vec3(1,1,1) * Fr;
        } else {
            color = CalculateDiffuse(
                pointIntersect, primitives, depth, maxDepth, numRays, sample
            );
        }

        return color;
    }

    vec3 CalculateDiffuse(
        const Intersection& pointIntersect,
        const vector<Primitive*>& primitives,
        int depth,
        int maxDepth,
        int numRays,
        int sample
    ) {
        vec3 directLight, indirectLight;

        directLight = DirectLight(
            pointIntersect, primitives, depth, maxDepth, numRays, sample
        );
        indirectLight = IndirectLight(
            pointIntersect, primitives, depth, maxDepth, numRays, sample
        );

        return (indirectLight + directLight) * pointIntersect.primitive->material.diffuse;
    }

    vec3 CalculateReflective(
        const Intersection& pointIntersect,
        const vector<Primitive*>& primitives,
        int depth,
        int maxDepth,
        int numRays,
        int sample
    ) {
        float reflectRoughness = pointIntersect.primitive->material.reflectRoughness;
        vec3 dir = CalculateReflectionVector(
            pointIntersect.ray.d, pointIntersect.normal
        );

        float product = dot(normalize(this->position - pointIntersect.position), normalize(dir));
        float specular = pow(product > 0 ? product : 0, pointIntersect.primitive->material.specularExponent);

        vec3 reflect (0, 0, 0);
        for (int i = 0; i < numRays; i++) {
            dir = pointOnCone(dir, reflectRoughness);

            Ray ray (pointIntersect.position + dir * 0.0001f, dir);

            Intersection intersect;
            bool found = Intersection::ClosestIntersection(
                ray, primitives, intersect, -1
            );

            if (found) {
                reflect += CalculateColor(
                    intersect, primitives, depth + 1, maxDepth, numRays, sample
                );
            } else {
                reflect += vec3 (0, 0, 0);
            }

            // Only loop once if the material is mirror
            if (reflectRoughness == 0) {
                numRays = 1;
                break;
            }
        }
        reflect /= numRays;
        return reflect * specular;
    }

    vec3 CalculateRefractive(
        const Intersection& pointIntersect,
        const vector<Primitive*>& primitives,
        int depth,
        int maxDepth,
        int numRays,
        int sample
    ) {
        vec3 color(0, 0, 0);
        float refractRoughness = pointIntersect.primitive->material.refractRoughness;
        vec3 T = CalculateRefractionVector(
            pointIntersect.primitive->material.ior,
            pointIntersect.normal,
            pointIntersect.ray.d
        );

        for (int i = 0; i < numRays; i++) {
            vec3 dir = pointOnCone(T, refractRoughness);
            Ray ray (pointIntersect.position + dir * 0.0001f, dir);

            Intersection intersect;
            bool found = Intersection::ClosestIntersection(
                ray, primitives, intersect, -1
            );

            if (found) {
                color = CalculateColor(
                    intersect, primitives, depth + 1, maxDepth, numRays, sample
                );
            } else {
                color = vec3 (0, 0, 0);
            }

            if (refractRoughness == 0) {
                break;
            }
        }

        return color;
    }

    /*
        Calculates the direction vector for the reflection ray.
    */
    vec3 CalculateReflectionVector(const vec3& I, const vec3& N) {
        return I - 2.f * dot(N, I) * N;
    }

    vec3 CalculateRefractionVector(
        const float& ior,
        const vec3& N,
        const vec3& I
    ) {
        float eta;

        vec3 normI = normalize(I);
        vec3 normN = normalize(N);

        float c1 = dot(normI, normN);
        float c2;
        bool enter = c1 < 0 ? true : false;

        if (enter) {
            eta = 1.f / ior;
        } else {
            eta = ior;
        }

        float k = 1.f - eta * eta * (1.f - c1 * c1);
        if (k < 0) {
            return vec3(0, 0, 0);
        }

        c2 = sqrt(k);
        return eta * normI + (eta * c1 - c2) * normN;
    }

    /*
        Calculates fresnel equations and returns Fr the refraction ratio,
        Ft the transmision ratio is just 1 - Fr.
    */
    float CalculateFresnel(const vec3& I, const vec3& N, const float& ior) {
        float cosi = clamp(-1.f, 1.f, dot(normalize(I), normalize(N)));
        float etai = 1.f, etat = ior;

        if (cosi > 0) {
            float temp = etai;
            etai = etat;
            etat = temp;
        }

        float sini = 1.f - cosi * cosi;
        float sint = etai / etat * sqrt(0.f > sini ? 0.f : sini);
        if (sint >= 1) {
            return 1.f;
        } else {
            float cost = 1.f - sint * sint;
            // cost = sqrt(0.f > cost ? 0.f : cost);
            cosi = fabsf(cosi);

            float Fpar = pow((etat * cosi - etai * cost) / (etat * cosi + etai * cost), 2);
            float Fper = pow((etai * cost - etat * cosi) / (etai * cost + etat * cosi), 2);

            return (Fpar + Fper) / 2.f + 0.1f < 1.f ? (Fpar + Fper) / 2.f + 0.1f : 1;
        }
    }

    vec3 DirectLight(
        const Intersection& pointIntersect,
        const vector<Primitive*>& primitives,
        int depth,
        int maxDepth,
        int numRays,
        int sample
    ) {
        Intersection intersect;
        vec3 directLight(0, 0, 0);
        float gridWidth = width / (float) sample;

        for (float z = v0.z; z < v1.z; z += gridWidth) {
            for (float x = v0.x; x < v1.x; x += gridWidth) {
                float dX = gridWidth * distribution(generator);
                float dZ = gridWidth * distribution(generator);
                vec3 lightPos(x + dX, v0.y, z + dZ);

                // Create direct ray towards light source
                Ray shadowRay (
                    pointIntersect.position, lightPos - pointIntersect.position
                );

                bool found = Intersection::ClosestIntersection(
                    shadowRay, primitives, intersect, pointIntersect.primitiveIndex
                );

                // If intersection exist, no direct light, else calculate direct light
                if (
                    found &&
                    distance(intersect.position, pointIntersect.position) <
                    distance(lightPos, pointIntersect.position)
                ) {
                    // directLight += vec3(0, 0, 0);
                } else {
                    vec3 R = lightPos - pointIntersect.position;
                    float r = length(R);
                    float product = dot(R, pointIntersect.normal);
                    if (product < 0) {
                        product = 0;
                    }

                    directLight += (this->color * product) / (float) (4.0 * M_PI * r * r);
                }
            }
        }

        directLight /= (float) sample * sample;
        return directLight;
    }

    vec3 IndirectLight(
        const Intersection& pointIntersect,
        const vector<Primitive*>& primitives,
        int depth,
        int maxDepth,
        int numRays,
        int sample
    ) {
        vec3 color(0, 0, 0);

        // cout << numRays * (1.f / (depth + 1)) << endl;
        for (int i = 0; i < numRays; i++) {
            // Get random sample from hemisphere
            vec3 direction = pointOnHemisphere(pointIntersect.normal);

            Ray ray (
                pointIntersect.position,
                direction
            );

            Intersection inter;
            bool found = Intersection::ClosestIntersection(
                ray, primitives, inter, pointIntersect.primitiveIndex
            );

            if (found) {
                color += CalculateColor(inter, primitives, depth + 1, maxDepth, numRays, sample);
            } else {
                // color += vec3(0, 0, 0);
            }
        }

        color /= (float) numRays;
        return color;
    }
private:
    vec3 pointOnHemisphere(const vec3& normal) {
        vec3 sampledPoint;
        float rightAng = M_PI / 2.f;
        float theta;
        do {
            float u = distribution(generator)*2.f*M_PI;
            float v = distribution(generator)*2.f*M_PI;

            sampledPoint = rotateY(rotateZ(vec3(0, 1, 0), u), v);

            theta = angle(sampledPoint, normal);
        } while (theta >= rightAng);
        return sampledPoint;
    }

    vec3 pointOnCone(const vec3& centre, float spread) {
        vec3 point;
        vec3 normCentre = normalize(centre);
        vec3 up = vec3(0, 1, 0);

        float u = distribution(generator) * spread;
        float v = distribution(generator) * spread;

        float theta = angle(up, normCentre);
        vec3 axis = cross(up, normCentre);

        point = normalize(vec3(u, 1, v));

        if (length(axis) != 0) {
            point = rotate(point, theta, axis);
        }

        return point;
    }
};
#endif
