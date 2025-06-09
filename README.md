# Shadertoy

## Overview
Shadertoy is a simple application that allows users to view and interact with GLSL shaders using SDL2 and OpenGL. The application loads shader files from a specified directory and renders them in real-time.

## Project Structure
```
[c][sdl2][opengl]Shadertoy
├── src
│   ├── main.c          # Main entry point of the application
│   ├── embededFont.c   # Implementation of embedded font rendering
│   ├── embededFont.h   # Header for embedded font functions
│   └── glad
│       ├── glad.c      # GLAD implementation for OpenGL function pointers
│       └── glad.h      # GLAD header file
├── shader
│   └── *.glsl          # Shader files written in GLSL
├── Makefile            # Build instructions for the project
└── README.md           # Documentation for the project
```

## Building the Project
To build the project, navigate to the project directory and run the following command:

```bash
make
```

This will compile the source files and link the necessary libraries.

## Running the Application
After building the project, you can run the application with the following command:

```bash
./shadertoy [shader_file.glsl]
```

If no shader file is specified, the application will load the first shader found in the `shader` directory.

## Controls
- Press `SPACE` to change the shader.
- Press `ESC` to quit the application.

## Dependencies
- SDL2
- GLAD

Make sure to have these libraries installed on your system before building the project.