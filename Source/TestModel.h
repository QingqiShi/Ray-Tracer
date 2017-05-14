#ifndef TEST_MODEL_CORNEL_BOX_H
#define TEST_MODEL_CORNEL_BOX_H

// Defines a simple test model: The Cornel Box

#include <glm/glm.hpp>
#include <vector>
#include "Triangle.h"

// Loads the Cornell Box. It is scaled to fill the volume:
// -1 <= x <= +1
// -1 <= y <= +1
// -1 <= z <= +1
void LoadTestModel( std::vector<Triangle>& triangles )
{
	using glm::vec3;

	// Defines colors:
	vec3 red(    0.75f, 0.15f, 0.15f );
	vec3 yellow( 0.75f, 0.75f, 0.15f );
	vec3 green(  0.15f, 0.75f, 0.15f );
	vec3 cyan(   0.15f, 0.75f, 0.75f );
	vec3 blue(   0.15f, 0.15f, 0.75f );
	vec3 purple( 0.75f, 0.15f, 0.75f );
	vec3 white(  0.75f, 0.75f, 0.75f );

	triangles.clear();
	triangles.reserve( 5*2*3 );

	// ---------------------------------------------------------------------------
	// Room

	float L = 555;			// Length of Cornell Box side.

	vec3 A(L,0,0);
	vec3 B(0,0,0);
	vec3 C(L,0,L);
	vec3 D(0,0,L);

	vec3 E(L,L,0);
	vec3 F(0,L,0);
	vec3 G(L,L,L);
	vec3 H(0,L,L);

	// Floor:
	triangles.push_back( Triangle( C, B, A, white ) );
	triangles.push_back( Triangle( C, D, B, white ) );

	// Left wall
	triangles.push_back( Triangle( A, E, C, red ) );
	triangles.push_back( Triangle( C, E, G, red ) );

	// Right wall
	triangles.push_back( Triangle( F, B, D, green ) );
	triangles.push_back( Triangle( H, F, D, green ) );

	// Ceiling
	triangles.push_back( Triangle( E, F, G, white ) );
	triangles.push_back( Triangle( F, H, G, white ) );

	// Back wall
	triangles.push_back( Triangle( G, D, C, white ) );
	triangles.push_back( Triangle( G, H, D, white ) );

	// Front wall
	triangles.push_back( Triangle( B, E, A, white ) );
	triangles.push_back( Triangle( F, E, B, white ) );

	// ---------------------------------------------------------------------------
	// Short block

	A = vec3(290,0,114);
	B = vec3(130,0, 65);
	C = vec3(240,0,272);
	D = vec3( 82,0,225);

	E = vec3(290,165,114);
	F = vec3(130,165, 65);
	G = vec3(240,165,272);
	H = vec3( 82,165,225);

#ifdef DRAW_SHORT_BOX
	// Front
	triangles.push_back( Triangle(E,B,A,white) );
	triangles[triangles.size() - 1].material.texture = true;
	triangles[triangles.size() - 1].setUV(vec2(0, 0.8), vec2(0.8, 0), vec2(0, 0));
	triangles.push_back( Triangle(E,F,B,white) );
	triangles[triangles.size() - 1].material.texture = true;
	triangles[triangles.size() - 1].setUV(vec2(0, 0.8), vec2(0.8, 0.8), vec2(0.8, 0));

	// Front
	triangles.push_back( Triangle(F,D,B,white) );
	triangles[triangles.size() - 1].material.texture = true;
	triangles[triangles.size() - 1].setUV(vec2(0, 0.8), vec2(0.8, 0), vec2(0, 0));
	triangles.push_back( Triangle(F,H,D,white) );
	triangles[triangles.size() - 1].material.texture = true;
	triangles[triangles.size() - 1].setUV(vec2(0, 0.8), vec2(0.8, 0.8), vec2(0.8, 0));

	// BACK
	triangles.push_back( Triangle(H,C,D,white) );
	triangles[triangles.size() - 1].material.texture = true;
	triangles[triangles.size() - 1].setUV(vec2(0, 0.8), vec2(0.8, 0), vec2(0, 0));
	triangles.push_back( Triangle(H,G,C,white) );
	triangles[triangles.size() - 1].material.texture = true;
	triangles[triangles.size() - 1].setUV(vec2(0, 0.8), vec2(0.8, 0.8), vec2(0.8, 0));

	// LEFT
	triangles.push_back( Triangle(G,E,C,white) );
	triangles[triangles.size() - 1].material.texture = true;
	triangles[triangles.size() - 1].setUV(vec2(0, 0.8), vec2(0.8, 0.8), vec2(0, 0));
	triangles.push_back( Triangle(E,A,C,white) );
	triangles[triangles.size() - 1].material.texture = true;
	triangles[triangles.size() - 1].setUV(vec2(0.8, 0.8), vec2(0.8, 0), vec2(0, 0));

	// TOP
	triangles.push_back( Triangle(G,F,E,white) );
	triangles[triangles.size() - 1].material.texture = true;
	triangles[triangles.size() - 1].setUV(vec2(0, 0.8), vec2(0.8, 0), vec2(0, 0));
	triangles.push_back( Triangle(G,H,F,white) );
	triangles[triangles.size() - 1].material.texture = true;
	triangles[triangles.size() - 1].setUV(vec2(0, 0.8), vec2(0.8, 0.8), vec2(0.8, 0));
#endif

	// ---------------------------------------------------------------------------
	// Tall block

	A = vec3(423,0,247);
	B = vec3(265,0,296);
	C = vec3(472,0,406);
	D = vec3(314,0,456);

	E = vec3(423,330,247);
	F = vec3(265,330,296);
	G = vec3(472,330,406);
	H = vec3(314,330,456);

	const char* tex = "texture.bmp";
	const char* norm = "normal.bmp";

	SDL_Surface* texS = SDL_LoadBMP(tex);
	SDL_Surface* normS = SDL_LoadBMP(norm);

	// Front
	triangles.push_back( Triangle(E,B,A,blue) );
	triangles[triangles.size() - 1].material.isReflective = false;
	triangles[triangles.size() - 1].material.isRefractive = false;
	triangles[triangles.size() - 1].material.texture = true;
	triangles[triangles.size() - 1].material.normalMap = true;
	triangles[triangles.size() - 1].setUV(vec2(0, 0.8), vec2(0.4, 0), vec2(0, 0));
	triangles[triangles.size() - 1].material.textureImage = texS;
	triangles[triangles.size() - 1].material.normalMapImage = normS;
	triangles.push_back( Triangle(E,F,B,blue) );
	triangles[triangles.size() - 1].material.isReflective = false;
	triangles[triangles.size() - 1].material.isRefractive = false;
	triangles[triangles.size() - 1].material.texture = true;
	triangles[triangles.size() - 1].material.normalMap = true;
	triangles[triangles.size() - 1].setUV(vec2(0, 0.8), vec2(0.4, 0.8), vec2(0.4, 0));
	triangles[triangles.size() - 1].material.textureImage = texS;
	triangles[triangles.size() - 1].material.normalMapImage = normS;

	// Front
	triangles.push_back( Triangle(F,D,B,blue) );
	triangles[triangles.size() - 1].material.isReflective = false;
	triangles[triangles.size() - 1].material.isRefractive = false;
	triangles[triangles.size() - 1].material.texture = true;
	triangles[triangles.size() - 1].material.normalMap = true;
	triangles[triangles.size() - 1].setUV(vec2(0, 0.8), vec2(0.4, 0), vec2(0, 0));
	triangles[triangles.size() - 1].material.textureImage = texS;
	triangles[triangles.size() - 1].material.normalMapImage = normS;
	triangles.push_back( Triangle(F,H,D,blue) );
	triangles[triangles.size() - 1].material.isReflective = false;
	triangles[triangles.size() - 1].material.isRefractive = false;
	triangles[triangles.size() - 1].material.texture = true;
	triangles[triangles.size() - 1].material.normalMap = true;
	triangles[triangles.size() - 1].setUV(vec2(0, 0.8), vec2(0.4, 0.8), vec2(0.4, 0));
	triangles[triangles.size() - 1].material.textureImage = texS;
	triangles[triangles.size() - 1].material.normalMapImage = normS;

	// BACK
	triangles.push_back( Triangle(H,C,D,blue) );
	triangles[triangles.size() - 1].material.isReflective = false;
	triangles[triangles.size() - 1].material.isRefractive = false;
	triangles[triangles.size() - 1].material.texture = true;
	triangles[triangles.size() - 1].material.normalMap = true;
	triangles[triangles.size() - 1].setUV(vec2(0, 0.8), vec2(0.4, 0), vec2(0, 0));
	triangles[triangles.size() - 1].material.textureImage = texS;
	triangles[triangles.size() - 1].material.normalMapImage = normS;
	triangles.push_back( Triangle(H,G,C,blue) );
	triangles[triangles.size() - 1].material.isReflective = false;
	triangles[triangles.size() - 1].material.isRefractive = false;
	triangles[triangles.size() - 1].material.texture = true;
	triangles[triangles.size() - 1].material.normalMap = true;
	triangles[triangles.size() - 1].setUV(vec2(0, 0.8), vec2(0.4, 0.8), vec2(0.4, 0));
	triangles[triangles.size() - 1].material.textureImage = texS;
	triangles[triangles.size() - 1].material.normalMapImage = normS;

	// LEFT
	triangles.push_back( Triangle(G,E,C,blue) );
	triangles[triangles.size() - 1].material.isReflective = false;
	triangles[triangles.size() - 1].material.isRefractive = false;
	triangles[triangles.size() - 1].material.texture = true;
	triangles[triangles.size() - 1].material.normalMap = true;
	triangles[triangles.size() - 1].setUV(vec2(0, 0.8), vec2(0.4, 0.8), vec2(0, 0));
	triangles[triangles.size() - 1].material.textureImage = texS;
	triangles[triangles.size() - 1].material.normalMapImage = normS;
	triangles.push_back( Triangle(E,A,C,blue) );
	triangles[triangles.size() - 1].material.isReflective = false;
	triangles[triangles.size() - 1].material.isRefractive = false;
	triangles[triangles.size() - 1].material.texture = true;
	triangles[triangles.size() - 1].material.normalMap = true;
	triangles[triangles.size() - 1].setUV(vec2(0.4, 0.8), vec2(0.4, 0), vec2(0, 0));
	triangles[triangles.size() - 1].material.textureImage = texS;
	triangles[triangles.size() - 1].material.normalMapImage = normS;

	// TOP
	triangles.push_back( Triangle(G,F,E,blue) );
	triangles[triangles.size() - 1].material.isReflective = false;
	triangles[triangles.size() - 1].material.isRefractive = false;
	triangles[triangles.size() - 1].material.texture = true;
	triangles[triangles.size() - 1].material.normalMap = true;
	triangles[triangles.size() - 1].setUV(vec2(0, 0.4), vec2(0.4, 0), vec2(0, 0));
	triangles[triangles.size() - 1].material.textureImage = texS;
	triangles[triangles.size() - 1].material.normalMapImage = normS;
	triangles.push_back( Triangle(G,H,F,blue) );
	triangles[triangles.size() - 1].material.isReflective = false;
	triangles[triangles.size() - 1].material.isRefractive = false;
	triangles[triangles.size() - 1].material.texture = true;
	triangles[triangles.size() - 1].material.normalMap = true;
	triangles[triangles.size() - 1].setUV(vec2(0, 0.4), vec2(0.4, 0.4), vec2(0.4, 0));
	triangles[triangles.size() - 1].material.textureImage = texS;
	triangles[triangles.size() - 1].material.normalMapImage = normS;


	// ----------------------------------------------
	// Scale to the volume [-1,1]^3

	for( size_t i=0; i<triangles.size(); ++i )
	{
		triangles[i].v0 *= 2/L;
		triangles[i].v1 *= 2/L;
		triangles[i].v2 *= 2/L;

		triangles[i].v0 -= vec3(1,1,1);
		triangles[i].v1 -= vec3(1,1,1);
		triangles[i].v2 -= vec3(1,1,1);

		triangles[i].v0.x *= -1;
		triangles[i].v1.x *= -1;
		triangles[i].v2.x *= -1;

		triangles[i].v0.y *= -1;
		triangles[i].v1.y *= -1;
		triangles[i].v2.y *= -1;

		triangles[i].ComputeNormal();
	}
}

#endif
