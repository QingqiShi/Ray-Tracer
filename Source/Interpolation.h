#ifndef __H_INTERPOLATION_H__
#define __H_INTERPOLATION_H__

class Interpolation {
public:
    static void Interpolate(
        const vector<Pixel>& verts,
        const vector<float>& weights,
        Pixel& result
    ) {
        if (verts.size() != weights.size()) {
            return;
        }

        bool interpUV = (
            verts[0].triangle->material.texture ||
            verts[0].triangle->material.normalMap
        );

        // Initialise values for interpolation
        result.invZ = 0;
        result.pos3d = vec3(0, 0, 0);
        if (interpUV) {
            result.u = 0;
            result.v = 0;
        }

        for (unsigned int i = 0; i < verts.size(); i++) {
            result.invZ += verts[i].invZ * weights[i];
            result.pos3d += verts[i].pos3d * verts[i].invZ * weights[i];

            // Only interpolate uv coordinates if needed
            if (interpUV) {
                result.u += verts[i].u * verts[i].invZ * weights[i];
                result.v += verts[i].v * verts[i].invZ * weights[i];
            }
        }

        // Correct perspective errors
        result.pos3d /= result.invZ;
        result.triangle = verts[0].triangle;
        if (interpUV) {
            result.u /= result.invZ;
            result.v /= result.invZ;
        }

        // Calculate normal if normal map in use
        if (
            result.triangle->material.normalMap &&
            result.triangle->material.normalMapImage != NULL
        ) {
            int x = (int)(result.triangle->material.normalMapImage->w * result.u);
            int y = (int)(result.triangle->material.normalMapImage->h * result.v);
            vec3 normMapValue = GetPixelSDL(result.triangle->material.normalMapImage, x, y);
            normMapValue.x = normMapValue.x * 2 - 1;
            normMapValue.y = normMapValue.y * 2 - 1;
            normMapValue.z = normMapValue.z * 2 - 1;

            mat3 transform(
                result.triangle->uvToWorld(vec2(0, 1)),
                result.triangle->uvToWorld(vec2(1, 0)),
                result.triangle->normal
            );
            result.normal = normalize(result.triangle->normal.x + transform * normMapValue);

        } else {
            result.normal = result.triangle->normal;
        }
    }

    static void Interpolate(
        const vector<float>& vertices,
        const vector<float>& weights,
        float& result
    ) {
        if (vertices.size() != weights.size()) {
            return;
        }
        result = 0;
        for (unsigned int i = 0; i < vertices.size(); i++) {
            result += vertices[i] * weights[i];
        }
    }

    static void Interpolate( Pixel a, Pixel b, vector<Pixel>& result )
    {
        int N = result.size();

        vector<ivec2> pos(N);
        vector<float> invZ(N);
        vector<vec3> pos3d(N);
        vector<float> u(N);
        vector<float> v(N);

        Interpolate(a.pos, b.pos, pos);
        Interpolate(a.invZ, b.invZ, invZ);
        Interpolate(a.pos3d * (float)a.invZ, b.pos3d * (float)b.invZ, pos3d);
        Interpolate(a.u, b.u, u);
        Interpolate(a.v, b.v, v);

        for (int i = 0; i < N; i++) {
            result[i].pos = pos[i];
            result[i].invZ = invZ[i];
            result[i].pos3d = pos3d[i] / (float)invZ[i];
            result[i].triangle = a.triangle;
            result[i].u = u[i];
            result[i].v = v[i];
        }
    }

    static void Interpolate( ivec2 a, ivec2 b, vector<ivec2>& result )
    {
        int N = result.size();

        if (N == 1) {
            result[0] = a;
        } else {
            vec2 step = vec2(b - a) / (float) (N - 1);
            vec2 current (a);
            for (int i = 0; i < N; i++) {
                result[i] = current;
                current += step;
            }
        }
    }

    static void Interpolate( vec3 a, vec3 b, vector<vec3>& result )
    {
        int N = result.size();

        if (N == 1) {
            result[0] = a;
        } else {
            vec3 step = (b - a) / (float) (N - 1);
            vec3 current = a;
            for (int i = 0; i < N; i++) {
                result[i] = current;
                current += step;
            }
        }
    }

    static void Interpolate( float a, float b, vector<float>& result )
    {
        int N = result.size();

        if (N == 1) {
            result[0] = a;
        } else {
            float step = (b - a) / (float) (N - 1);
            float current = a;
            for (int i = 0; i < N; i++) {
                result[i] = current;
                current += step;
            }
        }
    }

    static vector<Pixel> GetLine(Pixel a, Pixel b) {
        // Number of pixels to draw
        ivec2 delta = glm::abs( a.pos - b.pos );
        int pixels = glm::max( delta.x, delta.y ) + 1;

        // Interpolate line
        vector<Pixel> line( pixels );
        Interpolate( a, b, line );

        return line;
    }
};

#endif
