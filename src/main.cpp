#include <EGL/egl.h>
#include <stdio.h>
#include <vector>

#define STDOUT( text, ... ) printf( "%s:%d: " text, __FILE__, __LINE__, ##__VA_ARGS__ )
#define STDERR( text, ... ) fprintf( stderr, "%s:%d: " text, __FILE__, __LINE__, ##__VA_ARGS__ )

bool initialize() {
    // Obtain a handle to an EGLDisplay object by calling eglGetDisplay.
    EGLDisplay display = eglGetDisplay( EGL_DEFAULT_DISPLAY );
    if( EGL_NO_DISPLAY == display ) {
        STDERR( "Failed to get display.\n" );
        return false;
    }
    STDOUT( "Got display %p.\n", display );

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

    if( !eglMakeCurrent( display, surface, surface, context ) ) {
        STDERR( "Failed to make  EGL context current with error 0x%x.\n", eglGetError() );
        return false;
    }
    STDOUT( "Made EGL context current.\n" );

    // // After these steps, you have a set of EGL objects EGLDisplay, EGLConfig, EGLSurface and EGLContext that represent the main GLES2 rendering context.
    return true;
}

// void cleanup() {
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

int main() {
    initialize();
}
