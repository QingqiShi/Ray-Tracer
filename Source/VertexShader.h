#ifndef __H_VERTEXSHADER_H__
#define __H_VERTEXSHADER_H__

#include <vector>
#include <glm/gtx/vector_angle.hpp>
#include "TestModel.h"
#include "Pixel.h"
#include "Camera.h"
#include "PointLight.h"

class VertexShader {
public:
    /* Shader takes single triangle, modifies the screenPos and colors as the
    screen positions of the vertices and their responding colors. The screen
    buffer is modified during the process too. */
    static void Shade(
        Triangle& triangle,
        vector<Pixel>& pixels,
        Pixel (&buffer)[SCREEN_HEIGHT][SCREEN_WIDTH],
        Camera& cam,
        PointLight& light
    ) {
        pixels.resize(3);

        vec3 verts[3];
        verts[0] = triangle.v0;
        verts[1] = triangle.v1;
        verts[2] = triangle.v2;
        vec2 uvs[3];
        if (triangle.material.texture || triangle.material.normalMap) {
            uvs[0] = triangle.uv0;
            uvs[1] = triangle.uv1;
            uvs[2] = triangle.uv2;
        }
        ivec2 pos;

        for (int i = 0; i < 3; i++) {
            pos = checkAndTransform(verts[i], cam);

            pixels[i].pos = pos;
            // pixels[i].color = light.CalculateColor(verts[i], triangle.normal, triangle.color);
            // pixels[i].color = triangle.color;
            pixels[i].invZ = 1.0 / cam.WorldToCamera(verts[i]).z;
            pixels[i].pos3d = verts[i];
            pixels[i].triangle = &triangle;
            if (triangle.material.texture || triangle.material.normalMap) {
                pixels[i].u = uvs[i][0];
                pixels[i].v = uvs[i][1];
            }
            pixels[i].normal = triangle.normal;

            // if (
            //     pos[0] >= 0 && pos[1] >= 0 &&
            //     pos[0] < SCREEN_WIDTH && pos[1] < SCREEN_HEIGHT &&
            //     pixels[i].invZ > buffer[pos.y][pos.x].invZ
            // ) {
            //     buffer[pos.y][pos.x] = pixels[i];
            // }
        }
    }

    /* Similar shader that takes a vector of triangles and iterates through them.
    The screenPos and color vectors will be modified to the length equal to the
    size of the triangles vector times 3. */
    static void Shade(
        vector<Triangle>& triangles,
        vector<Pixel>& pixels,
        Pixel (&buffer)[SCREEN_HEIGHT][SCREEN_WIDTH],
        Camera& cam,
        PointLight& light
    ) {
        int numTri = triangles.size();

        pixels.resize(numTri * 3);

        vector<Pixel> tempPixels(3);
        for (int i = 0; i < numTri; i++) {
            // Shade every triangle
            Shade(triangles[i], tempPixels, buffer, cam, light);

            for (int j = 0; j < 3; j++) {
                pixels[i*3+j] = tempPixels[j];
            }
        }
    }

private:
    static vec2 checkAndTransform(vec3 vertex, Camera& cam) {
        // if (cam.WorldToCamera(vertex).z >= 0) {
            return cam.WorldToScreen(vertex);
        // } else {
            // return vec2(-5000,-5000);
        // }
    }
};

#endif
