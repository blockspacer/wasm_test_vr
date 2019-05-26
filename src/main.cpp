// https://emscripten.org/docs/porting/multimedia_and_graphics/OpenGL-support.html#webgl-friendly-subset-of-opengl-es-2-0-3-0

#include "egl.h"
#include "gles.h"
#include "user_context.h"
#include "util.h"
#include "vr.h"

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

    // Prepare use of VR.
    if( user_context.use_vr && ( user_context.vr_display == VR_NOT_SET ) ) {
        vr_prepare( user_context );
    }
}

int main() {
    // Note that emscripten_set_main_loop_arg asynchronously registers a callback and then exits.
    // Thus anything passed to it needs to be on the heap.
    UserContext& user_context = *( new UserContext() );

    if( !( egl_initialize( user_context ) && gles_load_shaders( user_context ) ) ) {
        STDERR( "Failed to set up program." );
        return -1;
    }
    STDOUT( "Set up program." );

    user_context.update_func = gles_update;
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
