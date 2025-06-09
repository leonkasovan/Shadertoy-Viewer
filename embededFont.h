#ifndef EMBEDEDFONT_H
#define EMBEDEDFONT_H

#include <stdint.h>
#include "glad/glad.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct EmbedFontCtx EmbedFontCtx;

EmbedFontCtx* embedFontCreate(int screen_width, int screen_height);
void embedFontDestroy(EmbedFontCtx* ctx);

void embedFontSetScreenSize(EmbedFontCtx* ctx, int width, int height);

void embedFontBindState(EmbedFontCtx* ctx);
void embedFontSetColor(EmbedFontCtx* ctx, float r, float g, float b, float a);

// sx, sy: size in pixels for each glyph
void embedFontDrawText(EmbedFontCtx* ctx, const char* str, float x, float y, float sx, float sy);

GLuint embedFontGetShader(const EmbedFontCtx* ctx);
GLuint embedFontGetFontTexture(const EmbedFontCtx* ctx);

#ifdef __cplusplus
}
#endif

#endif // EMBEDEDFONT_H