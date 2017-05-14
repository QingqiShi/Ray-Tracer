#ifndef __H_WIREFRAMESHADER_H__
#define __H_WIREFRAMESHADER_H__

#include "Interpolation.h"
#include "VertexShader.h"

class WireframeShader {
public:
    /* Shade single triangle */
    static void Shade(
        Triangle& triangle,
        Pixel (&buffer)[SCREEN_HEIGHT][SCREEN_WIDTH],
        Camera& cam
    ) {
        // Shade each vertex
        vector<Pixel> pixels(3);
        VertexShader::Shade(triangle, pixels, buffer, cam);

        // Draw line between each vertices
        for (int i = 0; i < 3; i++) {
            DrawLine(buffer, pixels[i], pixels[(i+1)%3]);
        }
    }

    /* Shade vector of triangles */
    static void Shade(
        vector<Triangle>& triangles,
        Pixel (&buffer)[SCREEN_HEIGHT][SCREEN_WIDTH],
        Camera& cam
    ) {
        int numTri = triangles.size();
        for (int i = 0; i < numTri; i++) {
            Shade(triangles[i], buffer, cam);
        }
    }

    static void DrawLine(
        Pixel (&buffer)[SCREEN_HEIGHT][SCREEN_WIDTH],
        Pixel& a,
        Pixel& b
    ) {
        if (a.pos.x == -5000 || b.pos.x == -5000) {
            return;
        }
        // Interpolate line
        vector<Pixel> line = Interpolation::GetLine(a, b);

        // Draw line
        int pixels = line.size();
        for (int i = 0; i < pixels; i++) {
            if (
                line[i].pos.x >= 0 &&
                line[i].pos.y >= 0 &&
                line[i].pos.x < SCREEN_WIDTH &&
                line[i].pos.y < SCREEN_HEIGHT
            ) {
                buffer[line[i].pos.y][line[i].pos.x] = line[i];
            }
        }
    }
};

#endif
