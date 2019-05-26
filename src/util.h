#ifndef WASMVR_UTIL_H
#define WASMVR_UTIL_H

#include <GLES3/gl3.h>
#include <stdio.h>
#include <string>

#define STDOUT( text, ... ) printf( "%s:%d: " text "\n", __FILE__, __LINE__, ##__VA_ARGS__ )
#define STDERR( text, ... ) fprintf( stderr, "%s:%d: " text "\n", __FILE__, __LINE__, ##__VA_ARGS__ )

namespace flatbuffers {
    template <typename T>
    class Vector;
}
namespace VR {
    class Pose;
}

extern "C" {
int  get_canvas_client_width();
int  get_canvas_client_height();
void set_canvas_size( int width, int height );
}

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

int pose_dof( const VR::Pose* pose );

void print_flatbuffers_float_matrix4xN( const flatbuffers::Vector<float>* matrix, int space_depth = 0 );
void print_flatbuffers_float_vector( const flatbuffers::Vector<float>* vector );
void print_flatbuffers_double_vector( const flatbuffers::Vector<double>* vector );

template <typename Source, typename Sink>
void flatbuffers_vector_to_native( const flatbuffers::Vector<Source>* source, Sink* sink ) {
    if( !source || !sink ) {
        return;
    }

    for( int i = 0; i < source->Length(); ++i ) {
        sink[i] = ( *source )[i];
    }
}

bool get_file_contents( const char* filename, std::string& contents );

extern const double  PI;
extern const GLfloat identity4[4 * 4];
extern const GLfloat identity4_rotation[4 * 4];
extern const GLfloat zero4[4 * 4];

#endif // WASMVR_UTIL_H
