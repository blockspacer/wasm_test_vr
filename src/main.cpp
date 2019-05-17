// https://emscripten.org/docs/porting/multimedia_and_graphics/OpenGL-support.html#webgl-friendly-subset-of-opengl-es-2-0-3-0

#include "user_context.h"
#include "vr.h"

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
