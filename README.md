This is a coursework created for the Computer Graphics module. For this coursework a ray tracer and a rasteriser was created using C++ and SDL. It received one of the highest marks and was commented as "one of the best I've ever seen" by the lecturer.

The ray tracer part is done using reverse ray tracing techniques, and consists of several extensions. Jittered super sampling was used for anti-aliasing; a jittered sampling method was also used for the area light to create soft shadows; the recursive path tracing technique was implemented for global illumination; triangles and spheres were created as primitives; reflection and refraction with blending modes and fresnel effects are also implemented; a lot of optimisation was done such as faster intersection algorithm, multi-threading, progressive-refinement and back-face culling. Texture mapping and normal mapping are also done as part of the rasteriser.

<img src="https://raw.githubusercontent.com/QingqiShi/Ray-Tracer/master/ray_tracer_render.bmp" width="500">

# Environment
Requires SDL 1.2, GLM latest, g++, Linux system.

# Compile
To compile, run the following command:

    $ make

This will produce an executable "raytracer" inside the Build folder.

# Usage
Go into the "Build" folder and execute "raytracer":

    $ cd Build
    $ ./raytracer

This program will take a coupe seconds to render the first preview, then progressively refine the image. The longer you wait, the more refined the preview will become.

This program was treated as a render, not an interactive program. For real-time interactive program, try the rasteriser:

    $ ./rasteriser
