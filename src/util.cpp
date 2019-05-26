#include "util.h"

#include <emscripten.h>
#include <fstream>
#include <iostream>
#include <math.h>

#include "vr_state_generated.h"

// clang-format off
EM_JS( int, get_canvas_client_width, (), { return impl_get_canvas_client_width(); } );
EM_JS( int, get_canvas_client_height, (), { return impl_get_canvas_client_height(); } );
EM_JS( void, set_canvas_size, ( int width, int height ), { impl_set_canvas_size( width, height ); } );
// clang-format on

const char* true_false( bool value ) {
    return value ? "true" : "false";
}

void quaternion_to_gl_matrix4x4(
    double   qw,
    double   qx,
    double   qy,
    double   qz,
    double   x,
    double   y,
    double   z,
    GLfloat* matrix ) {
    // clang-format off
    double transform[4 * 4] = {
        1 - 2*qy*qy - 2*qz*qz,	    2*qx*qy - 2*qz*qw,	    2*qx*qz + 2*qy*qw,   x,
            2*qx*qy + 2*qz*qw,	1 - 2*qx*qx - 2*qz*qz,	    2*qy*qz - 2*qx*qw,   y,
            2*qx*qz - 2*qy*qw,	    2*qy*qz + 2*qx*qw,	1 - 2*qx*qx - 2*qy*qy,   z,
                          0.0,                    0.0,                    0.0, 1.0,
    };
    // clang-format on

    for( int i = 0; i < 16; ++i ) {
        matrix[i] = transform[i];
    }
}

void gl_matrix4x4_mac(
    GLfloat*       out,
    const GLfloat* a,
    const GLfloat* b,
    const GLfloat* c ) {
    // This is written in a way that lets us use an input as an output
    // and avoids pointer aliasing problems.
    GLfloat temp[4 * 4 * 4];

    for( int i = 0; i < 4; ++i ) {
        for( int k = 0; k < 4; ++k ) {
            for( int j = 0; j < 4; ++j ) {
                temp[16 * i + 4 * j + k] = a[4 * i + j] * b[4 * j + k];
            }
        }
    }
    for( int i = 0; i < 4; ++i ) {
        for( int k = 0; k < 4; ++k ) {
            out[4 * i + k] = c[4 * i + k];
            for( int j = 0; j < 4; ++j ) {
                out[4 * i + k] += temp[16 * i + 4 * j + k];
            }
        }
    }
}

int pose_dof( const VR::Pose* pose ) {
    // -1DoF is indeterminant.
    // 0DoF is fixed in space (i.e. a screen).
    // 3Dof can point around but not move (like Daydream or trackers with position lock lost).
    // 6DoF can point around and move (like HTC Vive or trackers with position lock).

    if( !( pose ) ) {
        return -1;
    }

    if( !( pose->position() ) ) {
        return 0;
    }

    if( !( pose->orientation() ) ) {
        return 3;
    }

    return 6;
}

void print_flatbuffers_float_matrix4xN( const flatbuffers::Vector<float>* matrix, int space_depth ) {
    if( !matrix ) {
        return;
    }

    int rollover = 0;
    for( const float i : *matrix ) {
        if( !( rollover % 4 ) ) {
            for( int j = 0; j < space_depth; ++j ) {
                printf( " " );
            }
        }
        printf( "%+6f, ", i );
        if( !( ( ++rollover ) % 4 ) ) {
            printf( "\n" );
        }
    }
}

void print_flatbuffers_float_vector( const flatbuffers::Vector<float>* vector ) {
    if( !vector ) {
        return;
    }

    for( const float i : *vector ) {
        printf( "%f, ", i );
    }
}

void print_flatbuffers_double_vector( const flatbuffers::Vector<double>* vector ) {
    if( !vector ) {
        return;
    }

    for( const double i : *vector ) {
        printf( "%lf, ", i );
    }
}

bool get_file_contents( const char* filename, std::string& contents ) {
    std::ifstream in( filename, std::ios::in | std::ios::binary );
    if( in ) {
        contents = std::string();
        in.seekg( 0, std::ios::end );
        contents.resize( in.tellg() );
        in.seekg( 0, std::ios::beg );
        in.read( &contents[0], contents.size() );
        in.close();
        return true;
    }
    return false;
}

const double PI = atan2( 0, -1 );

const GLfloat identity4[4 * 4] = {
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f,
};

const GLfloat identity4_rotation[4 * 4] = {
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f,
};

const GLfloat zero4[4 * 4] = {
    0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f,
};
