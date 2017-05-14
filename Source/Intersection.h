#ifndef __H_INTERSECTION_H__
#define __H_INTERSECTION_H__

#include <glm/glm.hpp>
#include <vector>
#include <limits>
#include "Primitive.h"
#include "Sphere.h"
#include "Ray.h"

using namespace std;
using namespace glm;

class Intersection {
public:
    Ray ray;
    vec3 position;
    vec3 normal;
    float distance;
    Primitive* primitive;
    int primitiveIndex;

    Intersection() {
        position = vec3(0,0,0);
        normal = vec3(0,0,0);
        distance = 0;
        primitive = NULL;
        primitiveIndex = -1;
    }

    // On successfully finding an intersection between the ray and any
    //of the triangle planes, true is returned and closestIntersection set.
    static bool ClosestIntersection(
        Ray ray,
        const vector<Primitive*>& primitives,
        Intersection& intersection,
        int ignoreIndex
    ) {
        float closest = numeric_limits<float>::max();

        int size = primitives.size();
        for (int i = 0; i < size; i++) {
            if (i == ignoreIndex) {
                continue;
            }

            if (primitives[i]->isTriangle) {
                IntersectTriangle(
                    ray,
                    (Triangle*) primitives[i],
                    intersection,
                    closest,
                    i
                );
            } else if (primitives[i]->isSphere) {
                IntersectSphere(
                    ray,
                    (Sphere*) primitives[i],
                    intersection,
                    closest,
                    i
                );
            }
        }

        if (closest < numeric_limits<float>::max()) {
            // Backface culling
            // if (
            //     !intersection.primitive->material.isRefractive &&
            //     dot(ray.d, intersection.normal) > 0
            // ) {
            //     return false;
            // } else {
                return true;
            // }
        } else {
            return false;
        }
    }

    static void IntersectTriangle(
        Ray ray,
        Triangle* triangle,
        Intersection& intersection,
        float& closest,
        int index
    ) {
        // Backface culling
        if (
            !triangle->material.isRefractive &&
            dot(ray.d, triangle->normal) > 0
        ) {
            return;
        }

        vec3 e1 = triangle->v1 - triangle->v0;
        vec3 e2 = triangle->v2 - triangle->v0;
        vec3 b = ray.s - triangle->v0;
        mat3 A (-(ray.d), e1, e2);
        float dA = determinant(A);
        if (dA == 0) {
            return;
        }

        vec3 x; // for (t, u, v)

        mat3 Ai (b, A[1], A[2]);
        x[0] = determinant(Ai) / dA;

        if (x[0] <= 0 || x[0] >= closest) {
            return;
        }

        Ai[0] = A[0];
        Ai[1] = b;
        x[1] = determinant(Ai) / dA;

        if (x[1] < 0) {
            return;
        }

        Ai[1] = A[1];
        Ai[2] = b;
        x[2] = determinant(Ai) / dA;

        if (x[2] < 0 || x[1] + x[2] > 1) {
            return;
        }

        intersection.position = ray.s + x[0] * ray.d;
        intersection.distance = glm::distance(ray.s, intersection.position);
        intersection.normal = triangle->normal;
        intersection.primitive = (Primitive*) triangle;
        intersection.primitiveIndex = index;
        intersection.ray = ray;

        closest = x[0];
    }

    static void IntersectSphere(
        Ray ray,
        Sphere* sphere,
        Intersection& intersection,
        float& closest,
        int index
    ) {
        // ax^2 + bx + c = 0
        float a = dot(ray.d, ray.d);
        vec3 S = ray.s - sphere->position;
        float b = 2 * dot(S, ray.d);
        float c = dot(S, S) - (sphere->radius * sphere->radius);

        float discriminant = b * b - 4 * a * c;

        // No intersect
        if (discriminant < 0) {
            return;
        }

        // Intersection exists
        float t;
        if (discriminant == 0) {
            t = -(b / (2 * a));
        } else {
            float t1 = (-b + sqrt(discriminant)) / (2 * a);
            float t2 = (-b - sqrt(discriminant)) / (2 * a);

            if (t1 <= 0) {
                t = t2;
            } else if (t2 <= 0) {
                t = t1;
            } else {
                t = t1 < t2 ? t1 : t2;
            }
        }

        if (t <= 0 || t >= closest) {
            return;
        }

        intersection.position = ray.s + t * ray.d;
        intersection.distance = glm::distance(ray.s, intersection.position);
        intersection.normal = normalize(intersection.position - sphere->position);
        intersection.primitive = (Primitive*) sphere;
        intersection.primitiveIndex = index;
        intersection.ray = ray;

        closest = t;
    }

private:
    static bool CheckConstraints(float t, float u, float v) {
        return (u >= 0) && (v >= 0) && (u + v <= 1) && (t >= 0);
    }
};

#endif
