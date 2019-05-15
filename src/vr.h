#ifndef VR_H
#define VR_H

#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/vr.h>
#include <fstream>
#include <iostream>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <vector>

class UserContext;

#define STDOUT( text, ... ) printf( "%s:%d: " text "\n", __FILE__, __LINE__, ##__VA_ARGS__ )
#define STDERR( text, ... ) fprintf( stderr, "%s:%d: " text "\n", __FILE__, __LINE__, ##__VA_ARGS__ )

double get_timestamp();
int    get_canvas_client_width();
int    get_canvas_client_height();
void set_canvas_size( int width, int height );

bool egl_initialize( UserContext& user_context );
void egl_cleanup();
bool get_file_contents( const char* filename, std::string& contents );
GLuint gles_load_shader( EGLenum type, const char* shader_source, const char* name );
bool gles_load_shaders( UserContext& user_context );
void gles_draw( UserContext& user_context );
void print_frame_data( const VRFrameData& frame_data );
void vr_gles_draw( UserContext& user_context );
void update( UserContext& user_context );
void vr_render_loop( void* arg );
void vr_present( void* arg );
void switch_to_vr( UserContext& user_context );
EM_BOOL on_click_switch_to_vr( int, const EmscriptenMouseEvent*, void* arg );
void init_loop( void* arg );
void on_vr_init( void* );

#endif // VR_H
