#ifndef WASMVR_GLES_H
#define WASMVR_GLES_H

#include <GLES3/gl3.h>

class UserContext;

GLuint gles_load_shader( GLenum type, const char* shader_source, const char* name );
bool gles_load_shaders( UserContext& user_context );
void gles_update( UserContext& user_context );
void gles_draw( UserContext& user_context );

#endif // WASMVR_GLES_H
