#ifndef __H_POSTPROCESS_H__
#define __H_POSTPROCESS_H__

/*
    Credit to the original paper of FXAA from NVIDIA.
*/
class FXAA {
private:
    Pixel (&buffer)[SCREEN_HEIGHT][SCREEN_WIDTH];
    const vec2 N;
    const vec2 W;
    const vec2 M;
    const vec2 E;
    const vec2 S;
    const vec2 NW;
    const vec2 NE;
    const vec2 SW;
    const vec2 SE;
    const float EDGE_THRESHOLD = 1.f/16.f;
    const float EDGE_THRESHOLD_MIN = 1.f/12.f;
    const float SUBPIX = 1;
    const float SUBPIX_TRIM = 1.f/8.f;
    const float SUBPIX_CAP = 1.f;
    const int SEARCH_STEPS = SCREEN_WIDTH/2;
public:
    FXAA(Pixel (&buffer)[SCREEN_HEIGHT][SCREEN_WIDTH])
    : buffer(buffer), N(0,-1), W(-1,0), M(0,0), E(1,0), S(0,1), NW(-1,-1), NE(1,-1), SW(-1,1), SE(1,1) {}
    static float toLuma(const vec3& rgb);
    static Pixel* getBufferOffset(
        Pixel (&buffer)[SCREEN_HEIGHT][SCREEN_WIDTH],
        const vec2& pixel,
        const vec2& offset
    );
    vec3 process(Pixel* pixel);
};

vec3 filtered[SCREEN_HEIGHT][SCREEN_WIDTH];
class PostProcess {
public:
    static void Process(Pixel (&buffer)[SCREEN_HEIGHT][SCREEN_WIDTH]) {
        FXAA fxaa (buffer);
        Pixel* p;

        if (turnOnFXAA) {
            for (int i = 0; i < SCREEN_HEIGHT; i++) {
                for (int j = 0; j < SCREEN_WIDTH; j++) {
                    p = &buffer[i][j];

                    filtered[i][j] = fxaa.process(p);
                }
            }

            for (int i = 0; i < SCREEN_HEIGHT; i++) {
                for (int j = 0; j < SCREEN_WIDTH; j++) {
                    buffer[i][j].color = filtered[i][j];
                }
            }
        }
    }
};


float FXAA::toLuma(const vec3& rgb) {
    return rgb.y * (0.587/0.299) + rgb.x;
    // return sqrt(dot(rgb, vec3(0.299, 0.587, 0.114)));
}

Pixel* FXAA::getBufferOffset(
    Pixel (&buffer)[SCREEN_HEIGHT][SCREEN_WIDTH],
    const vec2& pixel,
    const vec2& offset
) {
    int x = std::min(std::max((int) (pixel.x + offset.x), 0), SCREEN_WIDTH-1);
    int y = std::min(std::max((int) (pixel.y + offset.y), 0), SCREEN_HEIGHT-1);
    return &buffer[y][x];
}

vec3 FXAA::process(Pixel* pixel) {
    /* Check local contrast */
    vec3 rgbN = getBufferOffset(buffer, pixel->pos, N)->color;
    vec3 rgbW = getBufferOffset(buffer, pixel->pos, W)->color;
    vec3 rgbM = getBufferOffset(buffer, pixel->pos, M)->color;
    vec3 rgbE = getBufferOffset(buffer, pixel->pos, E)->color;
    vec3 rgbS = getBufferOffset(buffer, pixel->pos, S)->color;

    float lumaN = toLuma(rgbN);
    float lumaW = toLuma(rgbW);
    float lumaM = toLuma(rgbM);
    float lumaE = toLuma(rgbE);
    float lumaS = toLuma(rgbS);

    float rangeMin = std::min(
        lumaM,
        std::min(
            std::min(lumaN, lumaW),
            std::min(lumaS, lumaE)
        )
    );

    float rangeMax = std::max(
        lumaM,
        std::max(
            std::max(lumaN, lumaW),
            std::max(lumaS, lumaE)
        )
    );

    float range = rangeMax - rangeMin;

    if (range < std::max(EDGE_THRESHOLD_MIN, rangeMax * EDGE_THRESHOLD)) {
        return pixel->color;
    }

    /* Detect edge direction */
    vec3 rgbNW = getBufferOffset(buffer, pixel->pos, NW)->color;
    vec3 rgbNE = getBufferOffset(buffer, pixel->pos, NE)->color;
    vec3 rgbSW = getBufferOffset(buffer, pixel->pos, SW)->color;
    vec3 rgbSE = getBufferOffset(buffer, pixel->pos, SE)->color;

    float lumaNW = toLuma(rgbNW);
    float lumaNE = toLuma(rgbNE);
    float lumaSW = toLuma(rgbSW);
    float lumaSE = toLuma(rgbSE);

    float edgeVert = (
        abs((0.25 * lumaNW) + (-0.5 * lumaN) + (0.25 * lumaNE)) +
        abs((0.50 * lumaW) + (-0.5 * lumaM) + (0.50 * lumaE)) +
        abs((0.25 * lumaSW) + (-0.5 * lumaS) + (0.25 * lumaSE))
    );
    float edgeHorz = (
        abs((0.25 * lumaNW) + (0.50 * lumaN) + (0.25 * lumaNE)) +
        abs((-0.5 * lumaW) + (-0.5 * lumaM) + (-0.5 * lumaE)) +
        abs((0.25 * lumaSW) + (0.50 * lumaS) + (0.25 * lumaSE))
    );
    bool horzSpan = edgeHorz >= edgeVert;

    /* Choose edge orientation */
    float luma1 = horzSpan ? lumaS : lumaW;
    float luma2 = horzSpan ? lumaN : lumaE;

    float contrast1 = abs(luma1 - lumaM);
    float contrast2 = abs(luma2 - lumaM);

    bool oneHighest = contrast1 >= contrast2;

    float gradientNorm = 0.25 * std::max(contrast1, contrast2);

    // End-of-edge Search in both negative and positive directions
    bool doneN = false, doneP = false;
    float lumaEndN = lumaM, lumaEndP = lumaM;

    vec2 dir = horzSpan ? E : S;
    vec2 lumaOffset = horzSpan ? S : W;
    if (!oneHighest) {
        lumaOffset *= -1.f;
    }
    vec2 posN = pixel->pos - dir;
    vec2 posP = pixel->pos + dir;
    float lumaAvg = 0.5 * (
        lumaM +
        horzSpan ? (oneHighest ? lumaS : lumaN) : (oneHighest ? lumaW : lumaE)
    );

    for (int i = 0; i < SEARCH_STEPS; i++) {
        if (!doneN) {
            lumaEndN = toLuma(getBufferOffset(buffer, posN, M)->color);
            lumaEndN += toLuma(getBufferOffset(buffer, posN, lumaOffset)->color);
            lumaEndN *= 0.5;
        }
        if (!doneP) {
            lumaEndP = toLuma(getBufferOffset(buffer, posP, M)->color);
            lumaEndP += toLuma(getBufferOffset(buffer, posP, lumaOffset)->color);
            lumaEndP *= 0.5;
        }

        doneN = doneN || (abs(lumaEndN - lumaAvg) >= gradientNorm);
        doneP = doneP || (abs(lumaEndP - lumaAvg) >= gradientNorm);

        if (doneN && doneP) {
            break;
        }

        if (!doneN) {
            posN -= dir;
        }
        if (!doneP) {
            posP += dir;
        }
    }

    /* Transform distance into offset */
    float distanceN = horzSpan ? (pixel->pos.x - posN.x) : (pixel->pos.y - posN.y) ;
    float distanceP = horzSpan ? (posP.x - pixel->pos.x) : (posP.y - pixel->pos.y) ;
    float pixelOffset = - std::min(distanceN, distanceP) / (distanceN + distanceP) + 0.5;
    // pixelOffset = pixelOffset * pixelOffset * 2;

    // cout << distanceN << " " << distanceP << endl;

    // bool finalCheck = (lumaM < lumaAvg) != (((distanceN < distanceP) ? lumaEndN : lumaEndP) < 0.0);
    // pixelOffset = finalCheck ? pixelOffset : 0.0;

    // cout << pixelOffset << endl;

    /* Resample buffer */
    vec3 rgbResample;
    rgbResample += pixel->color * (1.f-pixelOffset);
    rgbResample += getBufferOffset(buffer, pixel->pos, lumaOffset)->color * (pixelOffset);

    /* Aliasing test */
    float lumaL = (lumaN + lumaW + lumaE + lumaS) * 0.25;
    float rangeL = abs(lumaL - lumaM);
    float blendL = std::max(0.f, (rangeL / range) - SUBPIX_TRIM);
    blendL = std::min(SUBPIX_CAP, blendL);

    /* Lowpass box filter */
    vec3 rgbL = rgbN + rgbW + rgbM + rgbE + rgbS;
    rgbL += (rgbNW + rgbNE + rgbSW + rgbSE);
    rgbL *= vec3(1.f/9.f, 1.f/9.f, 1.f/9.f);

    /* Blend */
    // return rgbResample * (1.f - blendL) + rgbL * (blendL);
    // return rgbResample;
    return pixel->color * (1.f - std::max(blendL, pixelOffset)) + rgbL * std::max(blendL, pixelOffset);
}

#endif
