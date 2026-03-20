# Buas Game Project
this game is my submission for my application for BUas Creative Media and Game Technologies Programming course. its a simple game made with my own gameengine made allongside the game. all in c++ and openGL.


## contents
- [How to Play](#how-to-play)
- [Controls](#controls)
- [Build Instructions](#build-instructions)
- [Project Structure](#project-structure)
- [Libraries](#libraries)
- [Assets Attribution](#assets-attributions)
- [Learning Sources](#learning-sources)
- [Story Lore](#story-lore)

## How to Play
you are a asteroid safety mannager, your job is to select astoids and destroy them, so the planet doesnt get destroyed. you do this by selecting a astroid and destroying it before it impacts earth

## Controls
| Action            | Key                           |
|-------------------|-------------------------------|
| Select asteroid   | L Mouse Button                |
| Destroy asteroid  | Space                         |
| Look              | R Mouse Button + Mouse move   |
| Pause             | ESC                           |

## Build Instructions
The project compiles using **Visual Studio Community 2026**.
Cause visual studio doesnt allow for working directory settings (sometimes machine defined / doesnt save sometimes). you ether need to build the game (the astroidavoidance project in vs2026) and then run it from the build folder by dubble clicking the exe. or change the working directiory **(Configuration Properties -> Debugging -> Working Directory)** on **'asteroidavoidance'** project to **"$(TargetDir)"**


## Project Structure
- /src > C++/.h files
- /Dependencies >  External libraries and headers (GLFW, GLAD, etc.)
- /assets > Models, textures, audio, shaders
- /engineassets > Models, textures, audio, shaders that my custom engine lib needs


## Libraries
- [GLFW](#glfw)
- [GLAD](#glad)
- [GLM](#glm)
- [Assimp](#assimp)
- [STB_Image](#stb_image)

### GLFW
I use GLFW for opening a window and setting up OpenGL. I also use it to get Input from the player like Mouse movements or Keyboard. And i use it for system events like resizing window etc.

### GLAD
I use GLAD to load opengl 4.4 functions.

### GLM
I use GLM for Math opperations and types like vectors and matricies, i made a wrapper for it so the game itself doesnt need to have any external dependencies linked (exept for the engine itself ofcourse)

### Assimp
I use Assimp for loading 3d Models like .obj files

### STB_Image
I use STB_IMAGE for loading images into the game like .png files 

## Assets Attributions
- [The Sun 3d model](https://sketchfab.com/3d-models/sun-9ef1c68fbb944147bcfcc891d3912645)
- [Room Textures](https://ambientcg.com/view?id=Plaster001)
- [Desk Textures](https://ambientcg.com/view?id=Wood051)
- [inside earth Textures](https://www.solarsystemscope.com/textures/)
- [Earth Textures](https://www.solarsystemscope.com/textures/)
- [Sun Textures](https://www.solarsystemscope.com/textures/)
- Duck by my friend Koen de graaff
- Other Models, Shaders, Textures made are by me.

## Graphical Features
- Skysphere
- Deffered rendering
- basic lighting
- instanced rendering
- transparency
- rendertextures
- boids

## Learning Sources
- [basic lighting](https://learnopengl.com/Lighting/Basic-Lighting)
- [Loading 3d models](https://learnopengl.com/Model-Loading/Assimp)
- [Dithering](https://shader-tutorial.dev/advanced/color-banding-dithering/?utm_source=chatgpt.com)
- [boids](https://www.youtube.com/watch?v=bqtqltqcQhw)


## Story Lore
you are at work in your office at FISH office (Fast Interception & Space Hazard-avoidance) the game views are on 2 screens. One you see the planet with the asteroids around it. on the other you see info about the asteroid selected in the main view. you also have a button you can press and that will blowup that asteroid.

## Decor and level building
- Fish logo looks like nasa
- office has a aquarium

Made by Ties de Bruijn