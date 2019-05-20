#ifndef VR_H
#define VR_H

#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/vr.h>
#include <string>

#include "flatbuffer_container.h"
#include "vr_state_generated.h"

class UserContext;

#define STDOUT( text, ... ) printf( "%s:%d: " text "\n", __FILE__, __LINE__, ##__VA_ARGS__ )
#define STDERR( text, ... ) fprintf( stderr, "%s:%d: " text "\n", __FILE__, __LINE__, ##__VA_ARGS__ )

extern "C" {
int  get_canvas_client_width();
int  get_canvas_client_height();
void set_canvas_size( int width, int height );
int get_vr_state( uint8_t** vr_state, int vr_display_handle );
}

typedef FlatbufferContainer<VR::State> VRState;

const char* true_false( bool value );
void quaternion_to_gl_matrix4x4(
    double   qw,
    double   qx,
    double   qy,
    double   qz,
    double   x,
    double   y,
    double   z,
    GLfloat* matrix );
void gl_matrix4x4_mac(
    GLfloat*       out,
    const GLfloat* a,
    const GLfloat* b,
    const GLfloat* c );

bool egl_initialize( UserContext& user_context );
void egl_cleanup();

bool get_file_contents( const char* filename, std::string& contents );

GLuint gles_load_shader( EGLenum type, const char* shader_source, const char* name );
bool gles_load_shaders( UserContext& user_context );
void gles_update( UserContext& user_context );
void gles_draw( UserContext& user_context );

int pose_dof( const VR::Pose* pose );

void print_flatbuffers_float_matrix4xN( const flatbuffers::Vector<float>* matrix, int space_depth = 0 );
void print_flatbuffers_float_vector( const flatbuffers::Vector<float>* vector );
void print_flatbuffers_double_vector( const flatbuffers::Vector<double>* vector );
void print_vr_state( const VRState& state );

bool vr_state_get( VRState& vr_state, UserContext& user_context );
void vr_gles_draw( UserContext& user_context );
void vr_render_loop( void* arg );
void vr_present( void* arg );

void switch_to_vr( UserContext& user_context );
EM_BOOL on_click_switch_to_vr( int, const EmscriptenMouseEvent*, void* arg );
void init_loop( void* arg );
void on_vr_init( void* );

#endif // VR_H
