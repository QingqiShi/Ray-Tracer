#ifndef __H_PIXELSHADER_H__
#define __H_PIXELSHADER_H__

#include "Interpolation.h"
#include "VertexShader.h"
#include "PointLight.h"

class PixelShader {
public:
    /* Shade single triangle */
    static void Shade(
        Triangle& triangle,
        Pixel (&buffer)[SCREEN_HEIGHT][SCREEN_WIDTH],
        Camera& cam,
        PointLight& light
    ) {
        // Shade each vertex
        vector<Pixel> pixels(3);
        VertexShader::Shade(triangle, pixels, buffer, cam, light);

        // check
        if (
            pixels[0].pos.x == -5000 ||
            pixels[1].pos.x == -5000 ||
            pixels[2].pos.x == -5000
        ) {
            return;
        }

        // Draw line between each vertices
        // vector<Pixel> left, right;
        // ComputePolygonRows(pixels, left, right);
        // DrawRows(buffer, left, right, light, cam);
        DrawTriangle(
            buffer,
            pixels,
            light,
            cam
        );
    }

    /* Shade vector of triangles */
    static void Shade(
        vector<Triangle>& triangles,
        Pixel (&buffer)[SCREEN_HEIGHT][SCREEN_WIDTH],
        Camera& cam,
        PointLight light
    ) {
        int numTri = triangles.size();
        for (int i = 0; i < numTri; i++) {
            // Shade(triangles[i], buffer, cam, light);
            SDL_SemWait(empty);

            queue[tail] = &triangles[i];
    		tail = (tail + 1) % 100;

            SDL_SemPost(full);
        }

        for (int i = 0; i < numTri; i++) {
            SDL_SemWait(threadFinish);
        }
    }

private:
    static void ComputeBoundingBox(
        const vector<Pixel>& vs,
        int* x1,
        int* x2,
        int* y1,
        int* y2
    ) {
        if (vs.size() != 3) {
            return;
        }

        *x1 = std::max(0, (int) min(vs[0].pos.x, vs[1].pos.x, vs[2].pos.x));
        *x2 = std::min(SCREEN_WIDTH-1, (int) max(vs[0].pos.x, vs[1].pos.x, vs[2].pos.x));
        *y1 = std::max(0, (int) min(vs[0].pos.y, vs[1].pos.y, vs[2].pos.y));
        *y2 = std::min(SCREEN_HEIGHT-1, (int) max(vs[0].pos.y, vs[1].pos.y, vs[2].pos.y));
    }

    static float EdgeFunction(const vec2& a, const vec2& b, const vec2& c) {
        return (c.x-a.x) * (b.y-a.y) - (c.y-a.y) * (b.x-a.x);
    }


    /* Check if the point is inside triangle (return true or false), then
        calculates the weights (lambdas) of the barycentric coordinates */
    static bool CheckInsideAndComputeWeights(
        const vector<Pixel>& verts, const vec2& p, vector<float>& weights
    ) {
        int n = verts.size();
        weights.resize(n);
        float totalArea = 0, weight = 0;

        for (int i = 0; i < n; i++) {
            // Calculate area*2
            weight = EdgeFunction(verts[i].pos, verts[(i+1)%n].pos, p);

            // Test within triangle
            if (weight > 0) {
                return false;
            }

            // Accumulate area*2
            totalArea += weight;
            weights[(i+2)%n] = weight;
        }

        // Here, within triangle, normalize weight
        for (int i = 0; i < n; i++) {
            weights[i] /= totalArea;
        }
        return true;
    }

    static void DrawTriangle(
        Pixel (&buffer)[SCREEN_HEIGHT][SCREEN_WIDTH],
        const vector<Pixel>& verts,
        PointLight light,
        Camera cam
    ) {
        int x1=0, x2=0, y1=0, y2=0;
        vector<float> weights (verts.size());

        ComputeBoundingBox(verts, &x1, &x2, &y1, &y2);

        for (int i = y1; i < y2; i++) {
            for (int j = x1; j < x2; j++) {
                if (CheckInsideAndComputeWeights(
                    verts, vec2(j, i), weights
                )) {
                    Pixel p;
                    p.pos.x = j;
                    p.pos.y = i;
                    Interpolation::Interpolate(verts, weights, p);

                    if (cam.WorldToCamera(p.pos3d).z < 0) {
                        continue;
                    }

                    // Calculate color for pixel
                    p.color = light.CalculateColor(p);

                    // SDL_mutexP(bufferMutex);
                    if (p.invZ > buffer[i][j].invZ) {
                        buffer[i][j] = p;
                    }
                    // SDL_mutexV(bufferMutex);
                }
            }
        }
    }

    /*
    Compute positions of edge pixels of a triangle and use as start and end
    of pixels for each row
    */
    // static void ComputePolygonRows(
    //     const vector<Pixel>& vertexPixels,
    //     vector<Pixel>& left,
    //     vector<Pixel>& right
    // ) {
    //     // Computer number of rows
    //     int maxY = min(max(vertexPixels[0].pos.y, vertexPixels[1].pos.y, vertexPixels[2].pos.y), SCREEN_HEIGHT);
    //     int minY = max(min(vertexPixels[0].pos.y, vertexPixels[1].pos.y, vertexPixels[2].pos.y), 0);
    //     int numRows = maxY - minY + 1;
    //
    //     // Resize left and right rows
    //     if (numRows < 1) {
    //         left.resize(0);
    //         left.resize(0);
    //         return;
    //     }
    //     left.resize(numRows);
    //     right.resize(numRows);
    //
    //     // Initialize vectors
    //     for(int i = 0; i < numRows; i++) {
    //         left[i].pos.x = numeric_limits<int>::max();
    //         right[i].pos.x = numeric_limits<int>::min();
    //     }
    //
    //     // Loop through all edges and fill vector
    //     vector<Pixel> edge;
    //     for (int i = 0; i < 3; i++) {
    //
    //         edge = Interpolation::GetLine(vertexPixels[i], vertexPixels[(i+1)%3]);
    //
    //         int N = edge.size();
    //         for (int j = 0; j < N; j++) {
    //             // Screen clip
    //             if (edge[j].pos.y < 0 || edge[j].pos.y >= SCREEN_HEIGHT) {
    //                 continue;
    //             }
    //
    //             int y = edge[j].pos.y - minY;
    //
    //             if (y < 0 || y >= numRows) {
    //                 continue;
    //             }
    //
    //             if (left[y].pos.x > edge[j].pos.x) {
    //                 left[y] = edge[j];
    //             }
    //             if (right[y].pos.x < edge[j].pos.x) {
    //                 right[y] = edge[j];
    //             }
    //         }
    //     }
    // }
    //
    // /* Interpolate and draw each row of the triangle */
    // static void DrawRows(
    //     Pixel (&buffer)[SCREEN_HEIGHT][SCREEN_WIDTH],
    //     const vector<Pixel>& leftPixels,
    //     const vector<Pixel>& rightPixels,
    //     PointLight light,
    //     Camera cam
    // ) {
    //     vector<Pixel> row;
    //     vec3 dir;
    //     float ang;
    //
    //     int numRows = leftPixels.size();
    //     for (int i = 0; i < numRows; i++) {
    //         // Interpolate each row
    //         int numCols = rightPixels[i].pos.x - leftPixels[i].pos.x + 1;
    //         row.resize (numCols);
    //         Interpolation::Interpolate(leftPixels[i], rightPixels[i], row);
    //
    //         // Loop through each pixel
    //         for (int j = 0; j < numCols; j++) {
    //             // Boundary check
    //             if (
    //                 row[j].pos.x < 0 ||
    //                 row[j].pos.y < 0 ||
    //                 row[j].pos.x >= SCREEN_WIDTH ||
    //                 row[j].pos.y >= SCREEN_HEIGHT ||
    //                 row[j].invZ <= buffer[(int) row[j].pos.y][(int) row[j].pos.x].invZ
    //             ) {
    //                 continue;
    //             }
    //
    //             // Backface culling
    //             dir = normalize(vec3(
    //                 row[j].pos.x - SCREEN_WIDTH / 2.0,
    //                 row[j].pos.y - SCREEN_HEIGHT / 2.0,
    //                 cam.focalLength
    //             ));
    //             ang = angle(
    //                 dir,
    //                 cam.WorldToCamera(row[j].triangle->normal, false)
    //             );
    //
    //             if (ang < M_PI / 2.0) {
    //                 continue;
    //             }
    //
    //             // Calculate color for pixel
    //             row[j].color = light.CalculateColor(
    //                 row[j].pos3d,
    //                 *row[j].triangle
    //             );
    //             buffer[(int) row[j].pos.y][(int) row[j].pos.x] = row[j];
    //
    //         }
    //
    //     }
    // }

    // static float max(float a, float b) {
    //     return a > b ? a : b;
    // }

    static float max(float a, float b, float c) {
        return std::max(std::max(a, b), c);
    }

    // static float min(float a, float b) {
    //     return a < b ? a : b;
    // }

    static float min(float a, float b, float c) {
        return std::min(std::min(a, b), c);
    }
};

#endif
