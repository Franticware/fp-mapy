#ifndef HLIDAC_LOAD_TEXTURE_H
#define HLIDAC_LOAD_TEXTURE_H

#include <GLES2/gl2.h>
#include "pict.h"

GLuint load_texture(const Pict& pict);
GLuint load_texture_cube_map(const Pict& pict);
void load_texture_6(GLuint tex[6], const Pict& pict);

#endif
