#include "user_context.h"

const int VR_NOT_SET = -1;

UserContext::UserContext()
    : width( 0 )
    , height( 0 )
    , display( 0 )
    , context( 0 )
    , surface( 0 )
    , program( 0 )
    , vec4_position( -1 )
    , mat4_model( -1 )
    , mat4_view( -1 )
    , mat4_projection( -1 )
    , draw_func( nullptr )
    , update_func( nullptr )
    , use_vr( true )
    , vr_display( VR_NOT_SET ) {
}
