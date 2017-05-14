#ifndef __H_TRIANGLE_H__
#define __H_TRIANGLE_H__

#include "Primitive.h"

class Triangle : public Primitive {
public:
	vec3 v0, v1, v2;
	vec3 normal;

	// UV coordinates
	vec2 uv0, uv1, uv2;

	Triangle(vec3 v0, vec3 v1, vec3 v2, vec3 color )
		: v0(v0), v1(v1), v2(v2)
	{
        this->isTriangle = true;
		this->material.diffuse = color;
		ComputeNormal();
	}

	void ComputeNormal()
	{
		glm::vec3 e1 = v1-v0;
		glm::vec3 e2 = v2-v0;
		normal = normalize(cross( e2, e1 ) );
	}

	void setUV(vec2 v0, vec2 v1, vec2 v2) {
		uv0 = v0;
		uv1 = v1;
		uv2 = v2;
	}

	vec3 uvToWorld(vec2 uv) {
		vec2 b = uv - uv0;
		mat2 A (uv1 - uv0, uv2 - uv0);
		vec2 x = inverse(A) * b;
		return v0 + x[0] * (v1 - v0) + x[1] * (v2 - v0);
	}

	vec2 worldToUV(vec3 world) {
		mat3 A (v0, v1 - v0, v2 - v0);
		vec3 x = inverse(A) * world;
		// cout << x[1] << " " << x[2] << endl;
		return uv0 + x[1] * (uv1 - uv0) + x[2] * (uv2 - uv0);
	}
};

#endif
