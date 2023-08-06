# Physics Programming

Physics applications made with C/C++, OpenGL, and SDL2 for PC at the university.

## Table of Contents

- [Projects](#projects)
  - [Particles](#particles)
  - [Cloth](#cloth)
  - [Rigid Object](#rigid-object)
  - [Fluids](#fluids)
- [Getting Started](#getting-started)
  - [Pre-requisites](#pre-requisites)
- [Installation](#installation)
  - [Windows](#windows)
- [Authors](#authors)
- [License](#license)


# Projects

## Particles

https://github.com/sFaith3/PhysicsProgramming/assets/14791312/3bdb0253-202e-4b84-9bf5-a037bab36093

The simulation of the particle system is carried out in a box. This system has two types of emitters: fountain and cascade. Its parameters can be adjusted by the user from the interface, such as the emission rate (no less than 100 particles per second) and the lifetime of each.

The main characteristics:
1. The Euler method is used.
2. Collisions with the planes of the walls and the floor.
3. Collisions with certain objects: sphere and capsule.

## Cloth

https://github.com/sFaith3/PhysicsProgramming/assets/14791312/4eeafc33-4ef9-4f47-9770-231670e9c286

The clothing simulation is carried out in a box. This model deformation system, Provot's model, consists of multiple resorts between particles. And its parameters are adjustable by the user from the interface, such as resorts and collisions.

## Rigid Object

https://github.com/sFaith3/PhysicsProgramming/assets/14791312/2b2ee260-8bcd-4f11-bf65-ba6f9b556be7

This simulation, on the handling of collisions in a rigid body, consists of throwing one by applying a force on it, varying the amount and the place where it is applied, so that it collides in different ways with its environment.

## Fluids

https://github.com/sFaith3/PhysicsProgramming/assets/14791312/bf217aaf-e10f-444c-8082-2523945a22b9

This basic fluid simulation uses Gerstner waves, one of the most common methods in video games. To make things more interesting, a sphere is added that must fall from a height and sink or float according to its mass. And its parameters are adjustable by the user from the interface, for example, the fluid density, the number of waves, the direction, amplitude and frequency of each, and the mass and radius of the sphere.

<div align="right">
  
[ [ ↑ to top ↑ ] ](#physics-programming)
  
</div>


# Getting Started

This project has been made with Windows. But you should be able to use it on Linux and Mac.

## Pre-requisites

Before you start, make sure you have an IDE/Compiler such as [Visual Studio](https://visualstudio.microsoft.com/downloads), [Code::Blocks](https://www.codeblocks.org/downloads) or [XCode](https://developer.apple.com/xcode).

<div align="right">
  
[ [ ↑ to top ↑ ] ](#physics-programming)
  
</div>


# Installation

> **Note** If you are on Windows, you can skip the first step

1. [Hello SDL](https://lazyfoo.net/tutorials/SDL/01_hello_SDL/index.php).
2. Download the last version of [SDL2](https://github.com/libsdl-org/SDL/releases), [SDL2_image](https://github.com/libsdl-org/SDL_image/releases) and [SDL2_ttf](https://github.com/libsdl-org/SDL_ttf/releases) corresponding to your operating system (if you are on Windows, see the next).
3. Download the last version of OpenGL: `glew32.dll` and `glew32.lib`.

## Windows

1. Download the `VC.zip` for Visual Studio or the `mingw.tar.gz` for Code::Blocks and MinGW.
2. Create a folder called "libs" in each root project folder: "Particles/libs", "Cloth/libs", "RigidObject/libs" and "Fluids/libs".
3. Put the next libraries in the previous folder: `SDL2.dll`, `SDL2.lib`, `SDL2main.lib`, `glew32.dll` and `glew32.lib`.
4. Compile and run the solution in `Debug-x64`.

<div align="right">
  
[ [ ↑ to top ↑ ] ](#physics-programming)
  
</div>


# Authors

| [<img src="https://user-images.githubusercontent.com/14791312/233219860-32856bfe-bfa3-4a68-b0c4-f4d7f6ab0730.png" width=115><br><sub>Samuel Balcells</sub>](https://github.com/sFaith3) | [<img src="https://user-images.githubusercontent.com/14791312/233445809-c78094da-439c-4a8e-90dc-7fff1878ae18.jpg" width=115><br><sub>Sergi Sánchez</sub>](https://github.com/gyoza14)
| :---: | :---: |


# License

[MIT License](./LICENSE)

<div align="right">
  
[ [ ↑ to top ↑ ] ](#physics-programming)
  
</div>
