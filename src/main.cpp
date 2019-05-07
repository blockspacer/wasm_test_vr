// https://emscripten.org/docs/porting/multimedia_and_graphics/OpenGL-support.html#webgl-friendly-subset-of-opengl-es-2-0-3-0

#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <emscripten.h>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <vector>

#define STDOUT( text, ... ) printf( "%s:%d: " text, __FILE__, __LINE__, ##__VA_ARGS__ )
#define STDERR( text, ... ) fprintf( stderr, "%s:%d: " text, __FILE__, __LINE__, ##__VA_ARGS__ )

struct UserContext {
    GLint width;
    GLint height;

    EGLDisplay display;
    EGLContext context;
    EGLSurface surface;

    GLuint program;

    void ( *draw_func )( UserContext& );
    void ( *update_func )( UserContext& );
};

bool es_initialize( UserContext& user_context ) {
    // Obtain a handle to an EGLDisplay object by calling eglGetDisplay.
    EGLDisplay display = eglGetDisplay( EGL_DEFAULT_DISPLAY );
    if( EGL_NO_DISPLAY == display ) {
        STDERR( "Failed to get display.\n" );
        return false;
    }
    STDOUT( "Got display %p.\n", display );
    user_context.display = display;

    // Initialize EGL on that display by calling eglInitialize.
    EGLint major;
    EGLint minor;
    if( !eglInitialize( display, &major, &minor ) ) {
        STDERR( "Failed to initialize EGL.\n" );
        return false;
    }
    STDOUT( "Initialized EGL %d.%d.\n", major, minor );

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
        STDERR( "Failed to get EGL configuration.\n" );
        return false;
    }
    STDOUT( "Got EGL %d configurations.\n", num_config );

    if( !eglChooseConfig( display, attrib_list_choose_config, &config, 1, &num_config ) ) {
        STDERR( "Failed to choose EGL configuration.\n" );
        return false;
    }
    STDOUT( "Got %d matching EGL configurations.\n", num_config );

    // At this point, one would use whatever platform-specific functions available (X11, Win32 API, ANativeWindow) to set up a native window to render to.
    // For Emscripten, this step does not apply, and can be skipped.

    // Create a main render target surface (EGLSurface) by calling eglCreateWindowSurface with a valid display and config parameters.
    // Set window and attribute list parameters to null.
    EGLSurface surface = eglCreateWindowSurface( display, config, 0, nullptr );
    if( EGL_NO_SURFACE == surface ) {
        STDERR( "Failed to create EGL windows surface with error 0x%x.\n", eglGetError() );
        return false;
    }
    STDOUT( "Created EGL window.\n" );
    user_context.surface = surface;

    // Create a GLES2 rendering context (EGLContext) by calling eglCreateContext, followed by a call to eglMakeCurrent to activate the rendering context.
    // When creating the context, specify the context attribute EGL_CONTEXT_CLIENT_VERSION == 2.
    EGLint const attrib_list_create_context[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE, EGL_NONE};
    EGLContext context = eglCreateContext( display, config, EGL_NO_CONTEXT, attrib_list_create_context );
    if( EGL_NO_CONTEXT == context ) {
        STDERR( "Failed to create EGL context with error 0x%x.\n", eglGetError() );
        return false;
    }
    STDOUT( "Created EGL context.\n" );
    user_context.context = context;

    if( !eglMakeCurrent( display, surface, surface, context ) ) {
        STDERR( "Failed to make EGL context current with error 0x%x.\n", eglGetError() );
        return false;
    }
    STDOUT( "Made EGL context current.\n" );

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
            STDERR( "Error compiling shader %s:\n%s\n", name, infoLog );
            STDERR( "%s\n", shaderSrc );

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
        STDERR( "Failed to get vertex shader.\n" );
        return false;
    }
    STDOUT( "Got vertex shader.\n" );

    std::string frag_glsl;
    if( !get_file_contents( "src_asset/stl.frag", frag_glsl ) ) {
        STDERR( "Failed to get fragment shader.\n" );
        return false;
    }
    STDOUT( "Got fragment shader.\n" );

    // Load the vertex/fragment shaders
    GLuint vertexShader = gles_load_shader( GL_VERTEX_SHADER, vert_glsl.c_str(), "vertex" );
    if( !vertexShader ) {
        STDERR( "Failed to compile vertex shader.\n" );
        return false;
    }
    STDOUT( "Compiled vertex shader.\n" );

    GLuint fragmentShader = gles_load_shader( GL_FRAGMENT_SHADER, frag_glsl.c_str(), "fragment" );
    if( !fragmentShader ) {
        STDERR( "Failed to compile fragment shader.\n" );
        return false;
    }
    STDOUT( "Compiled fragment shader.\n" );

    // Create the program object
    GLuint program = glCreateProgram();

    if( program == 0 ) {
        return 0;
    }

    glAttachShader( program, vertexShader );
    glAttachShader( program, fragmentShader );

    // Bind vPosition to attribute 0
    glBindAttribLocation( program, 0, "vPosition" );

    // Link the program
    glLinkProgram( program );

    // Check the link status
    GLint linked;
    glGetProgramiv( program, GL_LINK_STATUS, &linked );

    user_context.program = program;

    return true;
}

void gles_draw( UserContext& user_context ) {
    GLfloat vVertices[] = {0.0f, 0.5f, 0.0f,
                           -0.5f, -0.5f, 0.0f,
                           0.5f, -0.5f, 0.0f};

    GLuint vertexPosObject;
    glGenBuffers( 1, &vertexPosObject );
    glBindBuffer( GL_ARRAY_BUFFER, vertexPosObject );
    glBufferData( GL_ARRAY_BUFFER, 9 * 4, vVertices, GL_STATIC_DRAW );

    // Set the viewport
    glViewport( 0, 0, user_context.width, user_context.height );

    // Clear the color buffer
    glClear( GL_COLOR_BUFFER_BIT );

    // Use the program object
    glUseProgram( user_context.program );

    // Load the vertex data
    glBindBuffer( GL_ARRAY_BUFFER, vertexPosObject );
    glVertexAttribPointer( 0 /* ? */, 3, GL_FLOAT, 0, 0, 0 );
    glEnableVertexAttribArray( 0 );

    glDrawArrays( GL_TRIANGLES, 0, 3 );
}

// clang-format off
EM_JS( int, get_canvas_width, (), {
    var c = Module.canvas;
    var w = c.clientWidth;
    if( c.width !== w ) {
        c.width = w;
    }
    return w;
} );

EM_JS( int, get_canvas_height, (), {
    var c = Module.canvas;
    var h = c.clientHeight;
    if( c.height !== h ) {
        c.height = h;
    }
    return h;
} );
// clang-format on

void update( UserContext& user_context ) {
    user_context.width  = get_canvas_width();
    user_context.height = get_canvas_height();
}

void render_loop( void* arg ) {
    UserContext& user_context = *( reinterpret_cast<UserContext*>( arg ) );
    if( user_context.update_func != nullptr ) {
        user_context.update_func( user_context );
    }
    if( user_context.draw_func != nullptr ) {
        user_context.draw_func( user_context );
    }

    eglSwapBuffers( user_context.display, user_context.surface );
}

int main() {
    // Note that emscripten_set_main_loop_arg asynchronously registers a callback and then exits.
    // Thus anything passed to it needs to be on the heap.
    UserContext& user_context = *( new UserContext() );
    memset( static_cast<void*>( &user_context ), 0, sizeof( user_context ) );

    if( !( es_initialize( user_context ) && gles_load_shaders( user_context ) ) ) {
        STDERR( "Failed to set up program.\n" );
        return -1;
    }
    STDOUT( "Set up program.\n" );

    user_context.update_func = update;
    user_context.draw_func   = gles_draw;

    emscripten_set_main_loop_arg(
        render_loop,
        static_cast<void*>( &user_context ),
        0 /*use requestAnimationFrame*/,
        false );
    STDOUT( "Exiting main.\n" );
    return 0;
}
