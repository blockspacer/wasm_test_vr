// https://emscripten.org/docs/porting/multimedia_and_graphics/OpenGL-support.html#webgl-friendly-subset-of-opengl-es-2-0-3-0

#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/vr.h>
#include <fstream>
#include <functional>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <vector>

#define STDOUT( text, ... ) printf( "%s:%d: " text "\n", __FILE__, __LINE__, ##__VA_ARGS__ )
#define STDERR( text, ... ) fprintf( stderr, "%s:%d: " text "\n", __FILE__, __LINE__, ##__VA_ARGS__ )

namespace {
    const int VR_NOT_SET = -1;
}

class UserContext {
public:
    GLint width;
    GLint height;

    EGLDisplay display;
    EGLContext context;
    EGLSurface surface;

    GLuint program;
    GLint  vec4_position;
    GLint  mat4_view;
    GLint  mat4_projection;

    void ( *draw_func )( UserContext& );
    void ( *update_func )( UserContext& );

    bool use_vr;

    int vr_display;

    UserContext()
        : width( 0 )
        , height( 0 )
        , display( 0 )
        , context( 0 )
        , surface( 0 )
        , program( 0 )
        , vec4_position( -1 )
        , mat4_view( -1 )
        , mat4_projection( -1 )
        , draw_func( nullptr )
        , update_func( nullptr )
        , use_vr( true )
        , vr_display( VR_NOT_SET ) {
    }
};

class Finally {
public:
    Finally( std::function<void( void )> functor )
        : functor_( functor ) {
    }
    ~Finally() {
        if( functor_ ) {
            functor_();
        }
    }
    void Clear() {
        functor_ = nullptr;
    }

private:
    std::function<void( void )> functor_;
};

bool es_initialize( UserContext& user_context ) {
    // Obtain a handle to an EGLDisplay object by calling eglGetDisplay.
    EGLDisplay display = eglGetDisplay( EGL_DEFAULT_DISPLAY );
    if( EGL_NO_DISPLAY == display ) {
        STDERR( "Failed to get display." );
        return false;
    }
    STDOUT( "Got display %p.", display );
    user_context.display = display;

    // Initialize EGL on that display by calling eglInitialize.
    EGLint major;
    EGLint minor;
    if( !eglInitialize( display, &major, &minor ) ) {
        STDERR( "Failed to initialize EGL." );
        return false;
    }
    STDOUT( "Initialized EGL %d.%d.", major, minor );

    // Call eglGetConfigs and/or eglChooseConfig one or multiple times to find the EGLConfig that represents the desired main render target parameters.
    // To examine the attributes of an EGLConfig, call eglGetConfigAttrib.
    EGLint attrib_list_choose_config[] = {
        EGL_RED_SIZE, 5,
        EGL_GREEN_SIZE, 6,
        EGL_BLUE_SIZE, 5,
        EGL_ALPHA_SIZE, /*( flags & ES_WINDOW_ALPHA ) ? 8 :*/ EGL_DONT_CARE,
        EGL_DEPTH_SIZE, /*( flags & ES_WINDOW_DEPTH ) ? 8 :*/ EGL_DONT_CARE,
        EGL_STENCIL_SIZE, /*( flags & ES_WINDOW_STENCIL ) ? 8 :*/ EGL_DONT_CARE,
        EGL_SAMPLE_BUFFERS, /*( flags & ES_WINDOW_MULTISAMPLE ) ? 1 :*/ 0,
        EGL_NONE};
    EGLConfig config;
    EGLint    num_config;

    if( !eglGetConfigs( display, NULL, 0, &num_config ) ) {
        STDERR( "Failed to get EGL configuration." );
        return false;
    }
    STDOUT( "Got EGL %d configurations.", num_config );

    if( !eglChooseConfig( display, attrib_list_choose_config, &config, 1, &num_config ) ) {
        STDERR( "Failed to choose EGL configuration." );
        return false;
    }
    STDOUT( "Got %d matching EGL configurations.", num_config );

    // At this point, one would use whatever platform-specific functions available (X11, Win32 API, ANativeWindow) to set up a native window to render to.
    // For Emscripten, this step does not apply, and can be skipped.

    // Create a main render target surface (EGLSurface) by calling eglCreateWindowSurface with a valid display and config parameters.
    // Set window and attribute list parameters to null.
    EGLSurface surface = eglCreateWindowSurface( display, config, 0, nullptr );
    if( EGL_NO_SURFACE == surface ) {
        STDERR( "Failed to create EGL windows surface with error 0x%x.", eglGetError() );
        return false;
    }
    STDOUT( "Created EGL window." );
    user_context.surface = surface;

    // Create a GLES2 rendering context (EGLContext) by calling eglCreateContext, followed by a call to eglMakeCurrent to activate the rendering context.
    // When creating the context, specify the context attribute EGL_CONTEXT_CLIENT_VERSION == 2.
    EGLint const attrib_list_create_context[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE, EGL_NONE};
    EGLContext context = eglCreateContext( display, config, EGL_NO_CONTEXT, attrib_list_create_context );
    if( EGL_NO_CONTEXT == context ) {
        STDERR( "Failed to create EGL context with error 0x%x.", eglGetError() );
        return false;
    }
    STDOUT( "Created EGL context." );
    user_context.context = context;

    if( !eglMakeCurrent( display, surface, surface, context ) ) {
        STDERR( "Failed to make EGL context current with error 0x%x.", eglGetError() );
        return false;
    }
    STDOUT( "Made EGL context current." );

    // // After these steps, you have a set of EGL objects EGLDisplay, EGLConfig, EGLSurface and EGLContext that represent the main GLES2 rendering context.
    return true;
}

// void es_cleanup() {
//     // Free up the currently active rendering context by calling eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT).
//     EGLBoolean eglMakeCurrent( EGLDisplay display,
//                                EGLSurface draw,
//                                EGLSurface read,
//                                EGLContext context );
//
//     // Deinitialize the EGLContext object by calling eglDestroyContext on it.
//     EGLBoolean eglDestroyContext( EGLDisplay display,
//                                   EGLContext context );
//
//     // Destroy all initialized EGLSurface objects by calling eglDestroySurface on them.
//     EGLBoolean eglDestroySurface( EGLDisplay display,
//                                   EGLSurface surface );
//
//     // Deinitialize EGL altogether by calling eglTerminate(display).
//     EGLBoolean eglTerminate( EGLDisplay display );
//
//     // Delete the native rendering window. This step does not apply for Emscripten.
// }

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

GLuint gles_load_shader( EGLenum type, const char* shaderSrc, const char* name ) {
    GLuint shader;
    GLint  compiled;

    // Create the shader object
    shader = glCreateShader( type );

    if( shader == 0 )
        return 0;

    // Load the shader source
    glShaderSource( shader, 1, &shaderSrc, NULL );

    // Compile the shader
    glCompileShader( shader );

    // Check the compile status
    glGetShaderiv( shader, GL_COMPILE_STATUS, &compiled );

    if( !compiled ) {
        GLint infoLen = 0;

        glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &infoLen );

        if( infoLen > 1 ) {
            char* infoLog = new char[infoLen];

            glGetShaderInfoLog( shader, infoLen, NULL, infoLog );
            STDERR( "Error compiling shader %s:\n%s", name, infoLog );
            STDERR( "%s", shaderSrc );

            delete[] infoLog;
        }

        glDeleteShader( shader );
        return 0;
    }

    return shader;
}

bool gles_load_shaders( UserContext& user_context ) {
    std::string vert_glsl;
    if( !get_file_contents( "src_asset/stl.vert", vert_glsl ) ) {
        STDERR( "Failed to get vertex shader." );
        return false;
    }
    STDOUT( "Got vertex shader." );

    std::string frag_glsl;
    if( !get_file_contents( "src_asset/stl.frag", frag_glsl ) ) {
        STDERR( "Failed to get fragment shader." );
        return false;
    }
    STDOUT( "Got fragment shader." );

    // Load the vertex/fragment shaders
    GLuint vertexShader = gles_load_shader( GL_VERTEX_SHADER, vert_glsl.c_str(), "vertex" );
    if( !vertexShader ) {
        STDERR( "Failed to compile vertex shader." );
        return false;
    }
    STDOUT( "Compiled vertex shader." );

    GLuint fragmentShader = gles_load_shader( GL_FRAGMENT_SHADER, frag_glsl.c_str(), "fragment" );
    if( !fragmentShader ) {
        STDERR( "Failed to compile fragment shader." );
        return false;
    }
    STDOUT( "Compiled fragment shader." );

    // Create the program object
    GLuint program = glCreateProgram();
    if( program == 0 ) {
        STDERR( "Failed to create program." );
        return false;
    }

    glAttachShader( program, vertexShader );
    glAttachShader( program, fragmentShader );

    // Link the program
    glLinkProgram( program );

    // Check the link status
    GLint linked;
    glGetProgramiv( program, GL_LINK_STATUS, &linked );
    if( !linked ) {
        STDERR( "Failed to link." );
        return false;
    }

    user_context.program         = program;
    user_context.vec4_position   = glGetAttribLocation( user_context.program, "vec4_position" );
    user_context.mat4_view       = glGetUniformLocation( user_context.program, "mat4_view" );
    user_context.mat4_projection = glGetUniformLocation( user_context.program, "mat4_projection" );
    STDOUT( "program         = %d", user_context.program );
    STDOUT( "vec4_position   = %d", user_context.vec4_position );
    STDOUT( "mat4_view       = %d", user_context.mat4_view );
    STDOUT( "mat4_projection = %d", user_context.mat4_projection );

    return true;
}

void gles_draw( UserContext& user_context ) {
    const int DIMENSION                      = 3;
    const int VERTICES                       = 3;
    GLfloat   vertices[DIMENSION * VERTICES] = {0.0f, 0.5f, 0.0f,
                                              -0.5f, -0.5f, 0.0f,
                                              0.5f, -0.5f, 0.0f};

    // Get a list of buffers to bind shader attributes to.
    const GLsizei vertex_shader_buffer_count = 1;
    GLuint        vertex_shader_buffers[vertex_shader_buffer_count];
    glGenBuffers( vertex_shader_buffer_count, vertex_shader_buffers );
    const GLuint vbuf_position = vertex_shader_buffers[0];

    // Load vertices into vertex shader buffer for vertices.
    glBindBuffer( GL_ARRAY_BUFFER, vbuf_position );
    glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices, GL_STATIC_DRAW );

    // Set the viewport.
    glViewport( 0, 0, user_context.width, user_context.height );

    // Clear the color output buffer.
    glClear( GL_COLOR_BUFFER_BIT );

    // Use this shader program.
    glUseProgram( user_context.program );

    // Point the shader attribute for position at the shader buffer for position.
    glBindBuffer( GL_ARRAY_BUFFER, vbuf_position );
    glVertexAttribPointer(
        user_context.vec4_position, // GLuint index
        DIMENSION,                  // GLint size (in number of vertex dimensions)
        GL_FLOAT,                   // GLenum type
        0,                          // GLboolean normalized (i.e. is-fixed-point)
        0,                          // GLsizei stride (i.e. byte offset between consecutive elements)
        0 );                        // const GLvoid * pointer (because GL_ARRAY_BUFFER is bound this is an offset into that bound buffer)
    glEnableVertexAttribArray( user_context.vec4_position );

    GLfloat identity4[4 * 4] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };

    glUniformMatrix4fv(
        user_context.mat4_view, // GLint location
        1,                      // GLsizei count
        GL_FALSE,               // GLboolean transpose
        identity4 );            // const GLfloat* value

    glUniformMatrix4fv(
        user_context.mat4_projection, // GLint location
        1,                            // GLsizei count
        GL_FALSE,                     // GLboolean transpose
        identity4 );                  // const GLfloat* value

    // Draw.
    glDrawArrays(
        GL_TRIANGLES, // GLenum mode
        0,            // GLint first
        VERTICES );   // GLsizei count (in number of vertices in this case)
}

void print_frame_data( const VRFrameData& frame_data ) {
    auto lpm  = frame_data.leftProjectionMatrix;
    auto rpm  = frame_data.rightProjectionMatrix;
    auto lvm  = frame_data.leftViewMatrix;
    auto rvm  = frame_data.rightViewMatrix;
    auto flag = frame_data.pose.poseFlags;
// clang-format off
#define MATARG( m )                                         \
    m[0 + 4 * 0], m[0 + 4 * 1], m[0 + 4 * 2], m[0 + 4 * 3], \
    m[1 + 4 * 0], m[1 + 4 * 1], m[1 + 4 * 2], m[1 + 4 * 3], \
    m[2 + 4 * 0], m[2 + 4 * 1], m[2 + 4 * 2], m[2 + 4 * 3], \
    m[3 + 4 * 0], m[3 + 4 * 1], m[3 + 4 * 2], m[3 + 4 * 3]
    // clang-format on
    STDOUT( "VRFrameData {\n"
            "  timestamp: %lf,\n"
            "  leftProjectionMatrix: [\n"
            "    [%+.6f, %+.6f, %+.6f, %+.6f],\n"
            "    [%+.6f, %+.6f, %+.6f, %+.6f],\n"
            "    [%+.6f, %+.6f, %+.6f, %+.6f],\n"
            "    [%+.6f, %+.6f, %+.6f, %+.6f],\n"
            "  ],\n"
            "  leftViewMatrix: [\n"
            "    [%+.6f, %+.6f, %+.6f, %+.6f],\n"
            "    [%+.6f, %+.6f, %+.6f, %+.6f],\n"
            "    [%+.6f, %+.6f, %+.6f, %+.6f],\n"
            "    [%+.6f, %+.6f, %+.6f, %+.6f],\n"
            "  ],\n"
            "  rightProjectionMatrix: [\n"
            "    [%+.6f, %+.6f, %+.6f, %+.6f],\n"
            "    [%+.6f, %+.6f, %+.6f, %+.6f],\n"
            "    [%+.6f, %+.6f, %+.6f, %+.6f],\n"
            "    [%+.6f, %+.6f, %+.6f, %+.6f],\n"
            "  ],\n"
            "  rightViewMatrix: [\n"
            "    [%+.6f, %+.6f, %+.6f, %+.6f],\n"
            "    [%+.6f, %+.6f, %+.6f, %+.6f],\n"
            "    [%+.6f, %+.6f, %+.6f, %+.6f],\n"
            "    [%+.6f, %+.6f, %+.6f, %+.6f],\n"
            "  ],\n"
            "  pose: {",
            frame_data.timestamp,
            MATARG( lpm ),
            MATARG( lvm ),
            MATARG( rpm ),
            MATARG( rvm ) );
    if( flag & VR_POSE_POSITION ) {
        printf( "    position: {\n"
                "      x: %+.6f,\n"
                "      y: %+.6f,\n"
                "      z: %+.6f,\n"
                "    },\n",
                frame_data.pose.position.x,
                frame_data.pose.position.y,
                frame_data.pose.position.z );
    }
    if( flag & VR_POSE_LINEAR_VELOCITY ) {
        printf( "    linearVelocity: {\n"
                "      x: %+.6f,\n"
                "      y: %+.6f,\n"
                "      z: %+.6f,\n"
                "    },\n",
                frame_data.pose.linearVelocity.x,
                frame_data.pose.linearVelocity.y,
                frame_data.pose.linearVelocity.z );
    }
    if( flag & VR_POSE_LINEAR_ACCELERATION ) {
        printf( "    linearAcceleration: {\n"
                "      x: %+.6f,\n"
                "      y: %+.6f,\n"
                "      z: %+.6f,\n"
                "    },\n",
                frame_data.pose.linearAcceleration.x,
                frame_data.pose.linearAcceleration.y,
                frame_data.pose.linearAcceleration.z );
    }
    if( flag & VR_POSE_ORIENTATION ) {
        printf( "    orientation: {\n"
                "      w: %+.6f,\n"
                "      x: %+.6f,\n"
                "      y: %+.6f,\n"
                "      z: %+.6f,\n"
                "    },\n",
                frame_data.pose.orientation.w,
                frame_data.pose.orientation.x,
                frame_data.pose.orientation.y,
                frame_data.pose.orientation.z );
    }
    if( flag & VR_POSE_ANGULAR_VELOCITY ) {
        printf( "    angularVelocity: {\n"
                "      x: %+.6f,\n"
                "      y: %+.6f,\n"
                "      z: %+.6f,\n"
                "    },\n",
                frame_data.pose.angularVelocity.x,
                frame_data.pose.angularVelocity.y,
                frame_data.pose.angularVelocity.z );
    }
    if( flag & VR_POSE_ANGULAR_ACCELERATION ) {
        printf( "    angularAcceleration: {\n"
                "      x: %+.6f,\n"
                "      y: %+.6f,\n"
                "      z: %+.6f,\n"
                "    },\n",
                frame_data.pose.angularAcceleration.x,
                frame_data.pose.angularAcceleration.y,
                frame_data.pose.angularAcceleration.z );
    }
    printf( "  }\n"
            "}\n" );
#undef MATARG
}

void vr_gles_draw( UserContext& user_context ) {
    VRFrameData frame_data;
    if( !emscripten_vr_get_frame_data( user_context.vr_display, &frame_data ) ) {
        STDERR( "Failed to get VR frame data." );
        return;
    }

    static int x = 0;
    if( !( x++ % 128 ) ) {
        // print_frame_data( frame_data );
    }

    {
        const int DIMENSION                      = 3;
        const int VERTICES                       = 3;
        GLfloat   vertices[DIMENSION * VERTICES] = {0.0f, 0.5f, 0.0f,
                                                  -0.5f, -0.5f, 0.0f,
                                                  0.5f, -0.5f, 0.0f};

        // Get a list of buffers to bind shader attributes to.
        const GLsizei vertex_shader_buffer_count = 1;
        GLuint        vertex_shader_buffers[vertex_shader_buffer_count];
        glGenBuffers( vertex_shader_buffer_count, vertex_shader_buffers );
        const GLuint vbuf_position = vertex_shader_buffers[0];

        // Load vertices into vertex shader buffer for vertices.
        glBindBuffer( GL_ARRAY_BUFFER, vbuf_position );
        glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices, GL_STATIC_DRAW );

        // Set the viewport.
        glViewport( 0, 0, user_context.width, user_context.height );

        // Clear the color output buffer.
        glClear( GL_COLOR_BUFFER_BIT );

        // Use this shader program.
        glUseProgram( user_context.program );

        // Point the shader attribute for position at the shader buffer for position.
        glBindBuffer( GL_ARRAY_BUFFER, vbuf_position );
        glVertexAttribPointer(
            user_context.vec4_position, // GLuint index
            DIMENSION,                  // GLint size (in number of vertex dimensions)
            GL_FLOAT,                   // GLenum type
            0,                          // GLboolean normalized (i.e. is-fixed-point)
            0,                          // GLsizei stride (i.e. byte offset between consecutive elements)
            0 );                        // const GLvoid * pointer (because GL_ARRAY_BUFFER is bound this is an offset into that bound buffer)
        glEnableVertexAttribArray( user_context.vec4_position );

        // Draw

        auto width_l = user_context.width / 2;
        auto width_r = user_context.width - width_l;

        // Draw left viewport.
        glUniformMatrix4fv(
            user_context.mat4_view,      // GLint location
            1,                           // GLsizei count
            GL_FALSE,                    // GLboolean transpose
            frame_data.leftViewMatrix ); // const GLfloat* value
        glUniformMatrix4fv(
            user_context.mat4_projection,      // GLint location
            1,                                 // GLsizei count
            GL_FALSE,                          // GLboolean transpose
            frame_data.leftProjectionMatrix ); // const GLfloat* value
        glViewport( 0, 0, width_l, user_context.height );
        glDrawArrays( GL_TRIANGLES, 0, 3 );

        // Draw right viewport.
        glUniformMatrix4fv(
            user_context.mat4_view,       // GLint location
            1,                            // GLsizei count
            GL_FALSE,                     // GLboolean transpose
            frame_data.rightViewMatrix ); // const GLfloat* value
        glUniformMatrix4fv(
            user_context.mat4_projection,       // GLint location
            1,                                  // GLsizei count
            GL_FALSE,                           // GLboolean transpose
            frame_data.rightProjectionMatrix ); // const GLfloat* value
        glViewport( width_l, 0, width_r, user_context.height );
        glDrawArrays( GL_TRIANGLES, 0, 3 );
    }

    if( !emscripten_vr_submit_frame( user_context.vr_display ) ) {
        STDERR( "Failed to submit frame to VR display." );
        return;
    }
}

// clang-format off
EM_JS( int, get_canvas_width, (), {
    return Module.canvas.clientWidth;
} );

EM_JS( int, get_canvas_height, (), {
    return Module.canvas.clientHeight;
} );

EM_JS( void, set_canvas_size, (int width, int height), {
    var c = Module.canvas;

    var w = c.clientWidth;
    if( c.width !== w ) {
        c.width = w;
    }

    var h = c.clientHeight;
    if( c.height !== h ) {
        c.height = h;
    }
} );
// clang-format on

void update( UserContext& user_context ) {
    user_context.width  = get_canvas_width();
    user_context.height = get_canvas_height();
    set_canvas_size( user_context.width, user_context.height );
}

void vr_present( void* arg ) {
    UserContext& user_context = *( reinterpret_cast<UserContext*>( arg ) );
    STDOUT( "VR Present" );
}

void vr_render_loop( void* arg ) {
    UserContext& user_context = *( reinterpret_cast<UserContext*>( arg ) );

    Finally cleanup( [&]() {
        STDERR( "Canceling use of VR." );
        user_context.use_vr = false;
        emscripten_vr_cancel_display_render_loop( user_context.vr_display );
    } );

    static bool requested = false;
    static bool setup     = false;
    if( !requested ) {
        VRLayerInit init = {
            "webgl-canvas",
            VR_LAYER_DEFAULT_LEFT_BOUNDS,
            VR_LAYER_DEFAULT_RIGHT_BOUNDS};

        if( !emscripten_vr_request_present( user_context.vr_display, &init, 1, vr_present, static_cast<void*>( &user_context ) ) ) {
            STDERR( "Request present with default canvas failed." );
            return;
        }
        STDOUT( "Requested VR presentation." );

        requested = true;
    } else if( !setup ) {
        if( !emscripten_vr_display_presenting( user_context.vr_display ) ) {
            STDOUT( "Waiting to begin VR presentation." );
        } else {
            STDOUT( "VR is presenting." );
            setup = true;
        }
    } else {
        if( user_context.update_func != nullptr ) {
            user_context.update_func( user_context );
        }
        if( user_context.draw_func != nullptr ) {
            user_context.draw_func( user_context );
        }
    }

    cleanup.Clear();
}

void switch_to_vr( UserContext& user_context ) {
    Finally cleanup( [&]() {
        STDERR( "Canceling attempt to use VR." );
        user_context.use_vr = false;
    } );

    emscripten_cancel_main_loop();
    if( !emscripten_vr_set_display_render_loop_arg( user_context.vr_display, vr_render_loop, static_cast<void*>( &user_context ) ) ) {
        STDERR( "Failed set display VR render loop of device %d.", user_context.vr_display );
        return;
    }

    user_context.draw_func = vr_gles_draw;

    cleanup.Clear();
    STDOUT( "Ready for VR rendering loop." );
}

EM_BOOL on_click_switch_to_vr( int, const EmscriptenMouseEvent*, void* arg ) {
    UserContext& user_context = *( reinterpret_cast<UserContext*>( arg ) );
    switch_to_vr( user_context );
    return true;
}

void init_loop( void* arg ) {
    UserContext& user_context = *( reinterpret_cast<UserContext*>( arg ) );
    if( user_context.update_func != nullptr ) {
        user_context.update_func( user_context );
    }

    // Draw normally.
    if( user_context.draw_func != nullptr ) {
        user_context.draw_func( user_context );
    }
    eglSwapBuffers( user_context.display, user_context.surface );

    // Begin use of VR.
    if( user_context.use_vr && ( user_context.vr_display == VR_NOT_SET ) ) {
        if( !emscripten_vr_ready() ) {
            STDOUT( "VR not ready" );
            return;
        }

        Finally cleanup( [&]() {
            STDERR( "Canceling attempt to use VR." );
            user_context.use_vr = false;
        } );

        int display_count = emscripten_vr_count_displays();
        if( display_count < 1 ) {
            STDERR( "No VR displays found." );
            return;
        }
        STDOUT( "Found %d VR displays.", display_count );

        for( int i = 0; i < display_count; ++i ) {
            VRDisplayHandle handle = emscripten_vr_get_display_handle( i );
            const char*     name   = emscripten_vr_get_display_name( handle );

            VRDisplayCapabilities caps;
            if( !emscripten_vr_get_display_capabilities( handle, &caps ) ) {
                STDERR( "Failed to get display capabilities index=%d handle=%d.", i, handle );
                return;
            }

            STDOUT( "Display Capabilities of display %d \"%s\": {\n"
                    "  hasPosition: %d,\n"
                    "  hasExternalDisplay: %d,\n"
                    "  canPresent: %d,\n"
                    "  maxLayers: %lu\n"
                    "}",
                    i,
                    name,
                    caps.hasPosition,
                    caps.hasExternalDisplay,
                    caps.canPresent,
                    caps.maxLayers );

            if( user_context.vr_display == VR_NOT_SET ) {
                STDOUT( "Set VR display to handle %d.", handle );
                user_context.vr_display = handle;
            }
        }

        if( emscripten_vr_display_presenting( user_context.vr_display ) ) {
            STDOUT( "Already VR presenting, jump straight into VR." );
            switch_to_vr( user_context );
        } else {
            if( EMSCRIPTEN_RESULT_SUCCESS != emscripten_set_click_callback( "enter-vr", static_cast<void*>( &user_context ), false, on_click_switch_to_vr ) ) {
                STDERR( "Failed to attach callback for entering VR." );
            }

            STDOUT( "Ready for user to request VR rendering loop." );
            // The user must request VR rendering from here.
        }

        cleanup.Clear();
    }

    // STDOUT( "Init loop." );
}

void on_vr_init( void* ) {
    STDOUT( "VR Initialized." );
}

int main() {
    // Note that emscripten_set_main_loop_arg asynchronously registers a callback and then exits.
    // Thus anything passed to it needs to be on the heap.
    UserContext& user_context = *( new UserContext() );

    if( !( es_initialize( user_context ) && gles_load_shaders( user_context ) ) ) {
        STDERR( "Failed to set up program." );
        return -1;
    }
    STDOUT( "Set up program." );

    user_context.update_func = update;
    user_context.draw_func   = gles_draw;

    if( !emscripten_vr_init( on_vr_init, nullptr ) ) {
        STDERR( "Browser does not support WebVR." );
        user_context.use_vr = false;
    } else {
        STDOUT( "Browser is running WebVR version %d.%d.",
                emscripten_vr_version_major(), emscripten_vr_version_minor() );
    }

    emscripten_set_main_loop_arg(
        init_loop,
        static_cast<void*>( &user_context ),
        0, // use requestAnimationFrame
        false );
    STDOUT( "Exiting main." );
    return 0;
}
