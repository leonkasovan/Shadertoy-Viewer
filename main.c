// Compile with:
// gcc main.c embededFont.c glad/glad.c -o shadertoy -lSDL2 -ldl -lm

#include <SDL2/SDL.h>
#ifdef OPENGLES
#include "glad/es/glad.h"
#else
#include "glad/glad.h"
#endif
#include "embededFont.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>  // Add at top with other includes

#ifdef OPENGLES
static const char* fragmentShaderHeader =
"#version 310 es\n"
"precision mediump float;\n"
"uniform vec3 iResolution;\n"
"uniform float iTime;\n"
"uniform float iTimeDelta;\n"
"uniform int iFrame;\n"
"uniform vec4 iMouse;\n"
"out vec4 fragColor;\n"
"void mainImage(out vec4 fragColor, in vec2 fragCoord);\n"
"void main() {\n"
"    vec2 fragCoord = gl_FragCoord.xy;\n"
"    mainImage(fragColor, fragCoord);\n"
"}\n";

static const char* vertexShaderSource =
"#version 310 es\n"
"precision mediump float;\n"
"in vec2 aPos;\n"
"void main() {\n"
"    gl_Position = vec4(aPos, 0.0, 1.0);\n"
"}\n";
#else
static const char* fragmentShaderHeader =
"#version 330\n"
"uniform vec3 iResolution;\n"
"uniform float iTime;\n"
"uniform float iTimeDelta;\n"
"uniform int iFrame;\n"
"uniform vec4 iMouse;\n"
"out vec4 fragColor;\n"
"void mainImage(out vec4 fragColor, in vec2 fragCoord);\n"  // Add function prototype
"void main() {\n"
"    vec2 fragCoord = gl_FragCoord.xy;\n"
"    mainImage(fragColor, fragCoord);\n"
"}\n";

static const char* vertexShaderSource =
"#version 330\n"
"in vec2 aPos;\n"
"void main() {\n"
"    gl_Position = vec4(aPos, 0.0, 1.0);\n"
"}\n";
#endif

GLuint loadShaderFromFile(const char* path) {
	// Load user shader source
	FILE* f = fopen(path, "rb");
	if (!f) {
		fprintf(stderr, "Failed to open shader file: %s\n", path);
		return 0;
	}

	fseek(f, 0, SEEK_END);
	long fileSize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char* userSource = malloc(fileSize + 1);
	if (fread(userSource, 1, fileSize, f) != fileSize) {
		fprintf(stderr, "Failed to read shader file completely.");
		free(userSource);
		fclose(f);
		return 0;
	}
	fclose(f);
	userSource[fileSize] = '\0';

	// Allocate and combine header + user shader
	size_t fullLen = strlen(fragmentShaderHeader) + strlen(userSource) + 2; // +2 for newline and null terminator
	char* fullSource = malloc(fullLen);
	snprintf(fullSource, fullLen, "%s\n%s", fragmentShaderHeader, userSource);
	free(userSource);

	// Compile shader
	GLuint shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(shader, 1, (const char**) &fullSource, NULL);
	glCompileShader(shader);
	free(fullSource);

	// Check for errors
	GLint success = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLint logLen = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
		char* log = malloc(logLen);
		glGetShaderInfoLog(shader, logLen, NULL, log);
		fprintf(stderr, "Shader \"%s\" compile error:\n%s\n", path, log);
		free(log);
		glDeleteShader(shader);
		shader = 0;
	}
	return shader;
}

GLuint createShaderProgram(const char* fragPath) {
	GLuint frag = loadShaderFromFile(fragPath);
	if (!frag) return 0;

	GLuint vert = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vert, 1, &vertexShaderSource, NULL);
	glCompileShader(vert);

	GLint success;
	glGetShaderiv(vert, GL_COMPILE_STATUS, &success);
	if (!success) {
		char log[512];
		glGetShaderInfoLog(vert, 512, NULL, log);
		fprintf(stderr, "Vertex shader compile error:\n%s\n", log);
		glDeleteShader(frag);
		return 0;
	}

	GLuint prog = glCreateProgram();
	glAttachShader(prog, vert);
	glAttachShader(prog, frag);
	glBindAttribLocation(prog, 0, "aPos");
	glLinkProgram(prog);

	glDeleteShader(vert);
	glDeleteShader(frag);
	return prog;
}

char** listShaderFiles(const char* dir, int* count) {
	DIR* d = opendir(dir);
	if (!d) return NULL;

	*count = 0;
	struct dirent* entry;
	while ((entry = readdir(d))) {
		if (strstr(entry->d_name, ".glsl")) (*count)++;
	}

	char** files = malloc(*count * sizeof(char*));
	int i = 0;

	rewinddir(d);
	while ((entry = readdir(d)) && i < *count) {
		if (strstr(entry->d_name, ".glsl")) {
			files[i] = malloc(strlen(dir) + strlen(entry->d_name) + 2);
			sprintf(files[i], "%s/%s", dir, entry->d_name);
			i++;
		}
	}

	closedir(d);
	return files;
}

void freeShaderFiles(char** files, int count) {
	for (int i = 0; i < count; i++) {
		free(files[i]);
	}
	free(files);
}

int main(int argc, char* argv[]) {
	char** shaderFiles = NULL;
	int shaderCount = 0;
	int currentShader = 0;

	if (argc < 2) {
		shaderFiles = listShaderFiles("shader", &shaderCount);
		if (!shaderFiles || shaderCount == 0) {
			fprintf(stderr, "No .glsl files found in shader directory\n");
			return 1;
		}
	}

	SDL_Init(SDL_INIT_VIDEO);
	// Set OpenGL context version (this can be adjusted)
#ifdef OPENGLES
// Replace existing SDL_GL_SetAttribute calls with:
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#else
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif
	SDL_Window* win = SDL_CreateWindow("ShaderToy", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	SDL_GLContext ctx = SDL_GL_CreateContext(win);
	SDL_GL_SetSwapInterval(1);
#ifdef OPENGLES	
	gladLoadGLES2Loader((GLADloadproc) SDL_GL_GetProcAddress);
#else	
	gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress);
#endif
	 // Query and print GLSL version
	const GLubyte* glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
	const GLubyte* glVersion = glGetString(GL_VERSION);
	printf("OpenGL Version: %s\n", glVersion);
	printf("GLSL Version: %s\n", glslVersion);

	// Modify shader loading:
	GLuint prog;
	if (argc >= 2) {
		prog = createShaderProgram(argv[1]);
	} else {
		prog = createShaderProgram(shaderFiles[currentShader]);
	}
	if (!prog) return 1;

	float quad[] = { -1, -1, 1, -1, -1, 1, 1, 1 };
	GLuint vao, vbo;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	// Move uniform location lookups outside the main loop
	int locRes = glGetUniformLocation(prog, "iResolution");
	int locTime = glGetUniformLocation(prog, "iTime");
	int locDelta = glGetUniformLocation(prog, "iTimeDelta");
	int locFrame = glGetUniformLocation(prog, "iFrame");
	int locMouse = glGetUniformLocation(prog, "iMouse");

	// Pre-bind states that don't change
	glUseProgram(prog);
	glBindVertexArray(vao);

	int frame = 0;
	Uint64 last = SDL_GetPerformanceCounter();
	static float totalTime = 0;
	float mouse[4] = { 0 };
	int running = 1;
	int w = 800, h = 600;  // Initialize with default window size
	EmbedFontCtx* font = embedFontCreate(w, h);
	unsigned char showInfo = 1;

	static int fps = 0;
	static Uint32 fpsLastTime = 0;
	char strFPS[64] = "FPS:";

	while (running) {
		// Handle events
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			switch (e.type) {
			case SDL_QUIT:
				running = 0;
				break;
			case SDL_KEYDOWN:
				if (e.key.keysym.sym == SDLK_ESCAPE) {
					running = 0;
				} else if (e.key.keysym.sym == SDLK_SPACE && shaderFiles) {
					// Cleanup previous shader
					glDeleteProgram(prog);

					// Load next shader
					currentShader = (currentShader + 1) % shaderCount;
					prog = createShaderProgram(shaderFiles[currentShader]);
					if (!prog) {
						running = 0;
						break;
					}

					// Refresh uniform locations
					glUseProgram(prog);
					locRes = glGetUniformLocation(prog, "iResolution");
					locTime = glGetUniformLocation(prog, "iTime");
					locDelta = glGetUniformLocation(prog, "iTimeDelta");
					locFrame = glGetUniformLocation(prog, "iFrame");
					locMouse = glGetUniformLocation(prog, "iMouse");

					// Reset time and frame count
					totalTime = 0;
					frame = 0;
				} else if (e.key.keysym.sym == SDLK_F1) {
					showInfo = !showInfo;
				}
				break;
			case SDL_MOUSEMOTION:
				mouse[0] = e.motion.x;
				mouse[1] = h - e.motion.y;
				break;
			case SDL_WINDOWEVENT:
				if (e.window.event == SDL_WINDOWEVENT_RESIZED || e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
					w = e.window.data1;
					h = e.window.data2;
					glViewport(0, 0, w, h);
					embedFontSetScreenSize(font, w, h);
				}
				break;
			}
		}

		// Time and FPS calculations
		Uint64 now = SDL_GetPerformanceCounter();
		float elapsed = (now - last) / (float) SDL_GetPerformanceFrequency();
		totalTime += elapsed;
		last = now;

		// Batch uniform updates
		glUseProgram(prog);
		glBindVertexArray(vao);
		glUniform3f(locRes, (float) w, (float) h, 1.0f);
		glUniform1f(locTime, totalTime);
		glUniform1f(locDelta, elapsed);
		glUniform1i(locFrame, frame++);
		glUniform4fv(locMouse, 1, mouse);

		glClear(GL_COLOR_BUFFER_BIT);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		if (showInfo) {
			// Calculate FPS
			fps++;
			Uint32 fpsCurrentTime = SDL_GetTicks();
			if (fpsCurrentTime - fpsLastTime >= 1000) {
				snprintf(strFPS, sizeof(strFPS), "FPS: %d", fps);
				fps = 0;
				fpsLastTime = fpsCurrentTime;
			}
			// Draw font
			embedFontBindState(font);
			embedFontSetColor(font, 1.0f, 1.0f, 0.0f, 1.0f);
			embedFontDrawText(font, shaderFiles[currentShader], 146.0f, 10.0f, 8.0f, 8.0f);
			embedFontSetColor(font, 1.0f, 1.0f, 1.0f, 1.0f);
			embedFontDrawText(font, "Shadertoy Viewer:\nPress F1 to toggle this info\nPress SPACE to change shader\nPress ESC to quit", 10.0f, 10.0f, 8.0f, 8.0f);
			embedFontSetColor(font, 0.0f, 1.0f, 0.0f, 1.0f);
			embedFontDrawText(font, strFPS, 10.0f, 45.0f, 8.0f, 8.0f);
		}

		SDL_GL_SwapWindow(win);
	}
	embedFontDestroy(font);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
	glDeleteProgram(prog);
	SDL_GL_DeleteContext(ctx);
	SDL_DestroyWindow(win);
	if (shaderFiles) {
		freeShaderFiles(shaderFiles, shaderCount);
	}
	SDL_Quit();
	return 0;
}
