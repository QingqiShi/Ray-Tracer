#include <iostream>
#include <vector>
// #include <random>
#include <algorithm>
#include <glm/glm.hpp>
#include <SDL.h>

using namespace std;
using namespace glm;

#define SCREEN_WIDTH 500
#define SCREEN_HEIGHT 500
#define SAMPLE 1
#define DRAW_SHORT_BOX

bool keyDown1 = false;
bool turnOnShadow = false;
bool keyDown2 = false;
bool turnOnFXAA = false;

#include "SDLauxiliary.h"
#include "TestModel.h"
#include "Pixel.h"
#include "Camera.h"
#include "PointLight.h"
#include "PostProcess.h"

/* ----------------------------------------------------------------------------*/
/* GLOBAL VARIABLES                                                            */

/* Screen surface and buffer */
SDL_Surface* screen;
Pixel buffer[SCREEN_HEIGHT][SCREEN_WIDTH];

/* Pin-hole camera */
Camera cam(0, 0, -3, SCREEN_HEIGHT);

/* Object triangles */
vector<Primitive*> primitives;

/* Light */
PointLight light(
	vec3(0, -0.97, -0.4), 20.f * vec3(1.f, 1.f, 1.f)
);

/* Timers */
int t, t2, dt;

/* Random generator */
default_random_engine generator;
uniform_real_distribution<float> distribution(0, 1);

/* Multi-threading data */
const int numThread = 8;
SDL_Thread * threads[numThread];
SDL_sem* threadFinish;
SDL_sem* full;
SDL_sem* empty;
SDL_mutex* bufferMutex;
SDL_mutex* workersMutex;
Triangle* queue[100];
int head = 0, tail = 0;
bool run = true;


/* ----------------------------------------------------------------------------*/
/* FUNCTIONS                                                                   */

void Update();
void Draw();
void ClearBuffer();
vec3 pointOnHemisphere(const vec3& normal);
int worker(void * data);

#include "PixelShader.h"

int main( int argc, char* argv[] )
{
	screen = InitializeSDL( SCREEN_WIDTH/SAMPLE, SCREEN_HEIGHT/SAMPLE );

	t = SDL_GetTicks();	// Set start value for timer.

	// Load model
	vector<Triangle> triangles;
	LoadTestModel(triangles);
	for (unsigned int i = 0; i < triangles.size(); i++) {
		primitives.push_back(&triangles[i]);
	}

	t2 = SDL_GetTicks();
	dt = float(t2-t);
	cout << "Loaded model in: " << dt << " ms.\n";

	light.primitives = &primitives;

	// Create mutex and semaphores
	bufferMutex = SDL_CreateMutex();
	workersMutex = SDL_CreateMutex();
	full = SDL_CreateSemaphore(0);
	empty = SDL_CreateSemaphore(100);
	threadFinish = SDL_CreateSemaphore(0);

	// Create threads
	for (int i = 0; i < numThread; i++) {
		threads[i] = SDL_CreateThread(worker, &i);
	}

	// Event loop
	t = SDL_GetTicks();
	while( NoQuitMessageSDL() )
	{
		Update();
		ClearBuffer();
		PixelShader::Shade(triangles, buffer, cam, light);
		PostProcess::Process(buffer);
		Draw();
	}

	run = false;

	// Wake up all threads
	for (int i = 0; i < 100; i++) {
		SDL_SemPost(full);
		SDL_SemPost(empty);
		SDL_SemPost(threadFinish);
	}

	// Join threads
	for (int i = 0; i < numThread; i++) {
		SDL_WaitThread(threads[i], NULL);
	}

	// Destroy mutex and semaphores
	SDL_DestroyMutex(bufferMutex);
	SDL_DestroyMutex(workersMutex);
	SDL_DestroySemaphore(full);
	SDL_DestroySemaphore(empty);
	SDL_DestroySemaphore(threadFinish);

	// Free textures
	for (size_t i = 0; i < triangles.size(); i++) {
		if (triangles[i].material.textureImage != NULL) {
			SDL_FreeSurface(triangles[i].material.textureImage);
			SDL_FreeSurface(triangles[i].material.normalMapImage);
			break;
		}
	}

	cout << "Finished.\n";

	SDL_SaveBMP( screen, "screenshot.bmp" );

	cout << "Saved.\n";

	return 0;
}

int worker(void * data) {
	Triangle* triangle;

	while (run) {
		SDL_SemWait(full);
		SDL_mutexP(workersMutex);

		triangle = queue[head];
		head = (head + 1) % 100;

		SDL_mutexV(workersMutex);
		SDL_SemPost(empty);

		PixelShader::Shade(*triangle, buffer, cam, light);

		SDL_SemPost(threadFinish);
	}
	return 0;
}

void Update()
{
	// Compute frame time:
	t2 = SDL_GetTicks();
	dt = float(t2-t);
	t = t2;

	cout << "Rendered in: " << dt << " ms (" << round(1000.f / dt) << " FPS).\n";

	// Mouse control
	int dx, dy, button;
	button = SDL_GetRelativeMouseState(&dx, & dy);
	if (button == SDL_BUTTON_LEFT) {
		cam.Rotate(dx * cam.rSpeed * (dt / 1000.0), -dy * cam.rSpeed * (dt / 1000.0));
	}

	Uint8* keyState = SDL_GetKeyState(0);

	// Forward and backward
	if (keyState[SDLK_w]) {
		cam.Translate(vec3(0, 0, cam.speed * (dt / 1000.0)));
	}
	if (keyState[SDLK_s]) {
		cam.Translate(vec3(0, 0, -cam.speed * (dt / 1000.0)));
	}

	// Yaw left and right
	if (keyState[SDLK_a]) {
		// cam.Rotate(cam.rSpeed * (dt / 1000.0));
		cam.Translate(vec3(-cam.speed * (dt / 1000.0), 0, 0));
	}
	if (keyState[SDLK_d]) {
		// cam.Rotate(-cam.rSpeed * (dt / 1000.0));
		cam.Translate(vec3(cam.speed * (dt / 1000.0), 0, 0));
	}

	// Step left and right
	if (keyState[SDLK_q]) {
		cam.Rotate(5 * cam.rSpeed * (dt / 1000.0));
	}
	if (keyState[SDLK_e]) {
		cam.Rotate(-5 * cam.rSpeed * (dt / 1000.0));
	}

	// Move light
	float lSpeed = 1.f;
	if (keyState[SDLK_RIGHT]) {
		light.position += cam.CameraToWorld(vec3(lSpeed*(dt/1000.f), 0, 0), false);
	}
	if (keyState[SDLK_LEFT]) {
		light.position += cam.CameraToWorld(vec3(-lSpeed*(dt/1000.f), 0, 0), false);
	}
	if (keyState[SDLK_UP]) {
		light.position += cam.CameraToWorld(vec3(0, -lSpeed*(dt/1000.f), 0), false);
	}
	if (keyState[SDLK_DOWN]) {
		light.position += cam.CameraToWorld(vec3(0, lSpeed*(dt/1000.f), 0), false);
	}

	// Reset
	if (keyState[SDLK_SPACE]) {
		cam.position = vec3(0, 0, -3);
		cam.yaw = 0;
		cam.pitch = 0;
		cam.Rotate(0, 0);
		light.position = vec3(0, -0.97, -0.3);
	}

	if (!keyState[SDLK_1]) {
		if (keyDown1) {
			turnOnShadow = !turnOnShadow;
		}
		keyDown1 = false;
	} else {
		keyDown1 = true;
	}
	if (!keyState[SDLK_2]) {
		if (keyDown2) {
			turnOnFXAA = !turnOnFXAA;
		}
		keyDown2 = false;
	} else {
		keyDown2 = true;
	}
}

const int DO_DOWN_SAMPLE = 2;
vec3 DO_buffer[SCREEN_HEIGHT/DO_DOWN_SAMPLE][SCREEN_WIDTH/DO_DOWN_SAMPLE];
const int DO_SAMPLE = 16;
const float rMax = 0.5;
void PostProcess() {
	Pixel* p;
	vec3 sample;
	vec2 screenCoord;
	vec3 dir;


	// Calculate SSDO
	for (int i = 0; i < SCREEN_HEIGHT/DO_DOWN_SAMPLE; i++) {
		for (int j = 0; j < SCREEN_WIDTH/DO_DOWN_SAMPLE; j++) {
			p = &buffer[i*DO_DOWN_SAMPLE][j*DO_DOWN_SAMPLE];
			DO_buffer[i][j] = vec3(0, 0, 0);

			dir = normalize(light.position - p->pos3d);
			float occlusion = 0;

			for (int k = 0; p->color != vec3(0, 0, 0) && k < DO_SAMPLE; k++) {

				sample = p->pos3d + dir * distribution(generator) * rMax;

				// Project to screen space
				screenCoord = cam.WorldToScreen(sample);

				// Calculate direct light
				if (
					screenCoord.x >= 0 && screenCoord.x < SCREEN_WIDTH &&
					screenCoord.y >= 0 && screenCoord.y < SCREEN_HEIGHT &&
					distance(sample, cam.position) > 1.f / buffer[(int)screenCoord.y][(int)screenCoord.x].invZ
				) {
					occlusion += 1;
				}
			}

			occlusion = 1.f - occlusion / DO_SAMPLE;
			if (occlusion < 0.5) {
				DO_buffer[i][j] = p->luminance * occlusion;
			} else {
				DO_buffer[i][j] = p->luminance;
			}
		}
	}


	for (int i = 0; i < SCREEN_HEIGHT; i++) {
		for (int j = 0; j < SCREEN_WIDTH; j++) {
			buffer[i][j].luminance = DO_buffer[i/DO_DOWN_SAMPLE][j/DO_DOWN_SAMPLE];
			// buffer[i][j].color = buffer[i][j].luminance;
			buffer[i][j].color = (buffer[i][j].luminance + vec3(0.3, 0.3, 0.3)) * buffer[i][j].diffuse;
		}
	}
}

void Draw()
{
	if( SDL_MUSTLOCK( screen ) )
	SDL_LockSurface( screen );

	vec3 color = vec3(0,0,0);

	for (int y = 0; y < SCREEN_HEIGHT/SAMPLE; y++) {
		for (int x = 0; x < SCREEN_WIDTH/SAMPLE; x++) {
			color = vec3(0,0,0);
			for (int i = 0; i < SAMPLE; i++) {
				for (int j = 0; j < SAMPLE; j++) {
					color += buffer[y*SAMPLE+i][x*SAMPLE+j].color;
				}
			}
			color /= SAMPLE * SAMPLE;
			PutPixelSDL(screen, x, y, color);
		}
	}

	if( SDL_MUSTLOCK( screen ) )
	SDL_UnlockSurface( screen );

	SDL_UpdateRect( screen, 0, 0, 0, 0 );
}

void ClearBuffer() {
	for (int y = 0; y < SCREEN_HEIGHT; y++) {
		for (int x = 0; x < SCREEN_WIDTH; x++) {
			buffer[y][x].color = vec3(0, 0, 0);
			buffer[y][x].invZ = 0;
		}
	}
}

vec3 pointOnHemisphere(const vec3& normal) {
	vec3 sampledPoint;
	float rightAng = M_PI / 2.f;
	float theta;
	int i = 0;
	do {
		float u = distribution(generator)*2.f*M_PI;
		float v = distribution(generator)*2.f*M_PI;

		sampledPoint = rotateY(rotateZ(vec3(0, 1, 0), u), v);

		theta = angle(sampledPoint, normalize(normal));
	} while (theta >= rightAng && i++ < 10);
	return sampledPoint;
}
