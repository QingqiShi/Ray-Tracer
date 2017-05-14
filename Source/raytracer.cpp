#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include <SDL_thread.h>
#include <thread>
#include <random>
#include <chrono>
#include <atomic>
#include <algorithm>

using namespace std;
using namespace glm;

#define SCREEN_WIDTH 500
#define SCREEN_HEIGHT 500

#include "SDLauxiliary.h"
#include "TestModel.h"
#include "Primitive.h"
#include "Triangle.h"
#include "Sphere.h"
#include "Intersection.h"
#include "Light.h"
#include "Pixel.h"
#include "Camera.h"
#include "Ray.h"

/* ----------------------------------------------------------------------------*/
/* GLOBAL VARIABLES                                                            */

const int BUCKET_RATIO = 6;
const int NUM_THREAD = BUCKET_RATIO * BUCKET_RATIO;
const int SAMPLE = 32;
const bool INTERACTIVE = false;

/* Random generator for sampling */
default_random_engine generator;
uniform_real_distribution<float> distribution(0, 1);

/* Screen surface and buffer */
SDL_Surface* screen;
Pixel buffer[SCREEN_HEIGHT][SCREEN_WIDTH];
int bufferCount[SCREEN_HEIGHT][SCREEN_WIDTH];
bool pixelGrid[SCREEN_HEIGHT][SCREEN_WIDTH][SAMPLE][SAMPLE];
SDL_mutex* mut;
SDL_sem* sem;

/* Pin-hole camera */
Camera cam(1.75, 0, -4.5, SCREEN_HEIGHT / 0.6);

/* Object triangles and light */
vector<Primitive*> primitives;
FlatSquareLight light(
	vec3(0, -0.98, 0), 15.f * vec3(1.f, 1.f, 0.9f), 0.5
);

/* Timers */
int t, t2, dt;

/* Atomic counter for displaying progress */
atomic_int completedBucket(0);
atomic_int toExit(0);

/* ----------------------------------------------------------------------------*/
/* FUNCTIONS                                                                   */

void Update();
void Draw();
void DrawBox(int i);
void DrawBucket(int n, FlatSquareLight light, int sample);

int main( int argc, char* argv[] )
{
	// Initialise buffer counter
	for (int y = 0; y < SCREEN_HEIGHT; y++) {
		for (int x = 0; x < SCREEN_WIDTH; x++) {
			bufferCount[y][x] = 0;
			buffer[y][x].color = vec3(0, 0, 0);
			for (int i = 0; i < SAMPLE; i++) {
				for (int j = 0; j < SAMPLE; j++) {
					pixelGrid[y][x][i][j] = false;
				}
			}
		}
	}

	// Initialise screen
	screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT );

	// Load triangles (timed)
	t = SDL_GetTicks();

	vector<Triangle> triangles;
	LoadTestModel(triangles);
	for (unsigned int i = 0; i < triangles.size(); i++) {
		primitives.push_back(&triangles[i]);
	}

	// Glass ball
	Sphere s1(vec3(0.3, 0.7, -0.5), 0.20, vec3(1, 1, 1));
	s1.material.isRefractive = true;
	s1.material.refractRoughness = 0;
	s1.material.reflectStrength = 1.5;
	s1.material.specularExponent = 0;
	s1.material.ior = 1;
	primitives.push_back(&s1);

	// Metal ball
	Sphere s2(vec3(-0.5, 0.7, -0.5), 0.3, vec3(0.5,0.5,1));
	s2.material.isReflective = true;
	s2.material.reflectStrength = 1;
	s2.material.reflectRoughness = 0;
	s2.material.specularExponent = 0;
	primitives.push_back(&s2);

	// Diffuse ball
	Sphere s3(vec3(0.5, 0.75, 0.3), 0.25, vec3(1,1,1));
	primitives.push_back(&s3);

	t2 = SDL_GetTicks();
	dt = float(t2-t);
	cout << "Loaded model in: " << dt << " ms." << endl;

	cam.Rotate(-0.4);

	// Create screen mutex and threads
	thread threads[NUM_THREAD];
	mut = SDL_CreateMutex();
	sem = SDL_CreateSemaphore(BUCKET_RATIO * BUCKET_RATIO);

	for (int i = 0; i < NUM_THREAD; i++) {
		threads[i] = thread(DrawBox, i);
	}

	// Start event loop to listen for exit events
	t = SDL_GetTicks();
	while( NoQuitMessageSDL() )
	{
		// Update();
		if (completedBucket == BUCKET_RATIO * BUCKET_RATIO) {
			Draw();
			completedBucket = 0;
			for (int i = 0; i < BUCKET_RATIO * BUCKET_RATIO; i++) {
				SDL_SemPost(sem);
			}
		}
	}

	// Set exit variable and wait for threads to join
	toExit = 1;

	for (int i = 0; i < BUCKET_RATIO * BUCKET_RATIO; i++) {
		SDL_SemPost(sem);
	}

	for (int i = 0; i < NUM_THREAD; i++) {
		threads[i].join();
	}

	// Destroy mutex and save image
	SDL_DestroyMutex(mut);

	SDL_SaveBMP( screen, "screenshot.bmp" );

	return 0;
}

void Update()
{
	// Compute frame time:
	t2 = SDL_GetTicks();
	dt = float(t2-t);
	t = t2;

	// cout << "Render time: " << dt << " ms." << endl;

	if (INTERACTIVE) {
		Uint8* keyState = SDL_GetKeyState(0);
		if (keyState[SDLK_UP]) {
			cam.Translate(vec3(0, 0, cam.speed * (dt / 1000.0)));
		}
		if (keyState[SDLK_DOWN]) {
			cam.Translate(vec3(0, 0, -cam.speed * (dt / 1000.0)));
		}
		if (keyState[SDLK_LEFT]) {
			cam.Rotate(cam.rSpeed * (dt / 1000.0));
		}
		if (keyState[SDLK_RIGHT]) {
			cam.Rotate(-cam.rSpeed * (dt / 1000.0));
		}
		// if (keyState[SDLK_w]) {
		//     light.position += cam.forward * (float)(dt / 1000.0);
		// }
		// if (keyState[SDLK_s]) {
		//     light.position -= cam.forward * (float)(dt / 1000.0);
		// }
		// if (keyState[SDLK_a]) {
		//     light.position -= cam.right * (float)(dt / 1000.0);
		// }
		// if (keyState[SDLK_d]) {
		//     light.position += cam.right * (float)(dt / 1000.0);
		// }
		// if (keyState[SDLK_q]) {
		//     light.position -= cam.down * (float)(dt / 1000.0);
		// }
		// if (keyState[SDLK_e]) {
		//     light.position += cam.down * (float)(dt / 1000.0);
		// }
	}
}

void Draw()
{
	t2 = SDL_GetTicks();
	dt = float(t2-t);
	cout << "Frame rendered in: " << dt << " ms." << endl;

	// Lock screen surface and draw buffer onto screen
	if( SDL_MUSTLOCK( screen ) )
	SDL_LockSurface( screen );

	for (int y = 0; y < SCREEN_HEIGHT; y++) {
		for (int x = 0; x < SCREEN_WIDTH; x++) {
			PutPixelSDL(screen, x, y, buffer[y][x].color / (float)(bufferCount[y][x]));
		}
	}

	if( SDL_MUSTLOCK( screen ) )
	SDL_UnlockSurface( screen );

	SDL_UpdateRect( screen, 0, 0, 0, 0 );

	t = SDL_GetTicks();
}

void DrawBox(int n)
{
	DrawBucket(n, light, SAMPLE);
}

void DrawBucket(int n, FlatSquareLight light, int sample)
{
	// Calculate the the top left and bottom right corners of the bucket
	int x1, x2, y1, y2;
	y1 = (n / BUCKET_RATIO) * SCREEN_HEIGHT / BUCKET_RATIO;
	y2 = (n / BUCKET_RATIO + 1) * SCREEN_HEIGHT / BUCKET_RATIO;
	x1 = (n % BUCKET_RATIO) * SCREEN_WIDTH / BUCKET_RATIO;
	x2 = (n % BUCKET_RATIO + 1) * SCREEN_WIDTH / BUCKET_RATIO;

	vec3 rayDir, color;
	bool found = false;
	Intersection pointIntersect;

	// Super sampling each pixel
	for (int s = 0; s < sample * sample && !toExit; s++) {

		int randS, gridX, gridY;
		do {
			randS = distribution(generator) * sample * sample;
			gridX = randS % sample;
			gridY = randS / sample;
		} while (pixelGrid[y1][x1][gridY][gridX] && !toExit);

		float dX = -0.5 + (randS % sample) / (float) sample;
		float dY = -0.5 + (randS / sample) / (float) sample;

		SDL_SemWait(sem);

		// Calculate color for every pixel in bucket
		for (int y = y1; y < y2 && !toExit; y++) {
			for (int x = x1; x < x2 && !toExit; x++) {

				pixelGrid[y][x][gridY][gridX] = true;

				// Calculate ray direction and create ray
				float randX = (1.f / sample) * distribution(generator);
				float randY = (1.f / sample) * distribution(generator);
				rayDir = vec3(
					x - SCREEN_WIDTH / 2.0 + dX + randX,
					y - SCREEN_HEIGHT / 2.0 + dY + randY,
					cam.focalLength
				);
				rayDir = cam.WorldToCamera(rayDir);
				Ray ray (cam.position, rayDir);

				// Calculate closest point intersected by the ray
				found = Intersection::ClosestIntersection(
					ray, primitives, pointIntersect, -1
				);

				// If found, calculate color using current quality level
				if (found) {
					// color += light.CalculateColor(
					buffer[y][x].color += light.CalculateColor(
						pointIntersect,
						primitives,
						0, 10, 1, 2
					);
				}
				bufferCount[y][x] ++;
			}
			// cout << "Worker " << n << " completed a row\n";
		}

		completedBucket++;

		// if( SDL_MUSTLOCK( screen ) )
		// 	SDL_LockSurface( screen );
		// for (int y = y1; y < y2 && !toExit; y++) {
		// 	for (int x = x1; x < x2 && !toExit; x++) {
		// 		PutPixelSDL(screen, x, y, buffer[y][x].color / (float)(bufferCount[y][x]));
		// 	}
		// }
		// if( SDL_MUSTLOCK( screen ) )
		// 	SDL_UnlockSurface( screen );
		//
		// SDL_mutexP(mut);
		// SDL_UpdateRect( screen, 0, 0, 0, 0 );
		// SDL_mutexV(mut);

		// cout << "Worker " << n << " progress: " << (float)(y + 1 - y1) / (float)(y2 - y1) * 100.f << "%" << endl;
		// cout << "Worker " << n << " sampled " << s+1 << "/" << sample * sample << "\n";
	}


}
