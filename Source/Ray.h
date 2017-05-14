#ifndef __H_RAY_H__
#define __H_RAY_H__

class Ray {
public:
    vec3 s;
    vec3 d;

    Ray () {}

    Ray (const Ray& ray) {
        this->s = ray.s;
        this->d = ray.d;
    }

    Ray (vec3 start, vec3 dir)
    : s(start), d(dir) {
    }
};

#endif
