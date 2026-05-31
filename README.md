# Opengl-Alleyway-Scene

A custom 3D scene built in C++ and OpenGL. This project demonstrates advanced real-time rendering techniques to create a gritty, atmospheric night alleyway, complete with glowing neon signs, dynamic omnidirectional shadows, and procedural wet floor reflections.

## ✨ Rendering Features

* **HDR & Bloom (Post-Processing):** Utilizes Multiple Render Targets (MRT) to extract emissive light sources, passing them through a Ping-Pong Framebuffer system for Gaussian Blurring, before compositing with Reinhard Tone Mapping and Gamma Correction.
* **Omnidirectional Shadow Mapping:** Point light shadows calculated dynamically using Depth Cubemaps and Geometry Shaders to cast shadows in all 6 directions simultaneously.
* **Procedural Puddles (FBM Noise):** Fractal Brownian Motion (FBM) generated directly in the fragment shader creates an organic, randomized puddle mask. This mask dynamically alters the surface normals, specular highlights, and diffuse absorption of the floor texture.
* **Flickering Emissives:** Time-based sine wave and hash noise functions to simulate failing/flickering neon tubes.
* **First-Person Camera:** Includes smooth mouse-look, WASD movement, and basic AABB collision detection to keep the camera within the alley boundaries.

## 🛠️ Dependencies

This project relies on the following libraries (included in the `include` and `lib` directories):
* **GLFW** (Window creation and input handling)
* **GLAD** (OpenGL function pointers)
* **GLM** (Mathematics)
* **stb_image** (Texture loading)
* **cgltf** (GLTF object and texture loading)

## 🚀 Build and Run Instructions

This project is configured to compile via the command line using `g++` (MinGW) on Windows. 

Clone the repository and open your terminal in the root directory of the project.

**1. Navigate to the source folder:**
```bash
cd src



**2. Compile the project:**

```bash
g++ -std=c++17 test.cpp glad.c -o alleyway -I../include -L../lib -lglfw3 -lopengl32 -lgdi32

```

**3. Navigate back to the root directory:**
*(This ensures the executable can find the `models/`, `textures/`, and `shaders/` folders correctly).*

```bash
cd ..

```

**4. Run the executable:**

```bash
"src/alleyway.exe"

```

## 🎮 Controls

* **W, A, S, D:** Move Camera
* **Mouse:** Look around
* **ESC:** Close application

## 📂 Project Structure

* `src/` - Contains the main C++ source code (`test.cpp`, `glad.c`).
* `headers/` - Custom C++ header files for Shaders, Camera, and GLTF Models.
* `shaders/` - GLSL files (Vertex, Fragment, and Geometry shaders for lighting, shadows, and post-processing).
* `models/` - 3D models used to populate the scene.
* `textures/` - Environment textures.
* `include/` & `lib/` - Pre-compiled libraries for GLFW, GLAD, and GLM.
* `Tests/` - WebGl used to help position objects, camera and render scene without lighting effects, reducing compilation and composing time

## 📜 Credits & Asset Attribution

A massive thank you to the talented 3D artists and photographers who made their assets available under the Creative Commons Attribution license:

### 3D Models (Sketchfab)

* ["Old House"](https://skfb.ly/oQ9Ow) by iagoArt3D
* ["Retro Neon Sign for Pub/Bar"](https://skfb.ly/pFD9F) by kurtpeinke
* ["Neon Open Sign"](https://skfb.ly/pwzOF) by Logan S.
* ["Neon_lights"](https://skfb.ly/oAtvo) by 3D Joenish
* ["Victorian lamppost"](https://skfb.ly/6VJzK) by rebeccafuller
* ["Fluorescent Lamp/Light - 4096px²"](https://skfb.ly/pHnsC) by Mark Peters
* ["Dumpster - 4096px²"](https://skfb.ly/pGBQR) by Mark Peters
* ["Red Telephone (Emergency Phone)"](https://skfb.ly/6UWot) by TrickyEgor
* ["Weathered Concrete Barriers"](https://skfb.ly/pGID8) by Guy in a Poncho
* ["Road Barrier Collection"](https://skfb.ly/o8HL8) by Miguel Teixeira
* ["Broken Window 05"](https://skfb.ly/6U8ns) by Game Ready Art
* ["Police Tape #132067"](https://skfb.ly/pJG9s) by pngslol78
* ["Door"](https://skfb.ly/6qG7R) by Nikolayy

### Textures & Materials

* **Poly Haven:** [Damaged Plaster](https://polyhaven.com/a/damaged_plaster) by Amal Kumar
* **Poly Haven:** [Cobblestone Pavement](https://polyhaven.com/a/cobblestone_pavement) by Charlotte Baglioni
* **Poly Haven:** [Red Brick](https://polyhaven.com/a/red_brick) by Rob Tuytel
* **Magnific:** [Ground Texture Pattern](https://www.freepik.com/free-photo/photo-ground-texture-pattern_198162478.htm) by Mateus Andre
* **Magnific:** [Ground Texture Pattern](https://www.magnific.com/free-photo/photo-ground-texture-pattern_391158433.htm) by Mateus Andre
* **Magnific:** [Ground Texture Pattern](https://www.magnific.com/free-photo/wet-asphalt-background_1234642.htm) by montypeter


```
