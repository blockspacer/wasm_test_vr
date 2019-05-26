#include "egl.h"

#include "user_context.h"
#include "util.h"

bool egl_initialize( UserContext& user_context ) {
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
        EGL_ALPHA_SIZE, EGL_DONT_CARE,   // or 8
        EGL_DEPTH_SIZE, EGL_DONT_CARE,   // or 8
        EGL_STENCIL_SIZE, EGL_DONT_CARE, // or 8
        EGL_SAMPLE_BUFFERS, 0,           // or 1?
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
