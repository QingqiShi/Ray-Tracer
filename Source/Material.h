#ifndef __H_Material_H__
#define __H_Material_H__

class Material {
public:
    vec3 diffuse;

    bool isReflective;
    float reflectStrength;
    float reflectRoughness;
    float specularExponent;

    bool isRefractive;
    float ior;
    float refractRoughness;

    bool texture;
    SDL_Surface* textureImage;

    bool normalMap;
    SDL_Surface* normalMapImage;

    Material () {
        isReflective = false;
        reflectStrength = 1;
        reflectRoughness = 0;
        specularExponent = 1;
        isRefractive = false;
        ior = 1;
        refractRoughness = 0.01;
        texture = false;
        textureImage = NULL;
        normalMap = false;
        normalMapImage = NULL;
    }

};

#endif
