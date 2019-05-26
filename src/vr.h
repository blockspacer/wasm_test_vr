#ifndef WASMVR_VR_H
#define WASMVR_VR_H

#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/vr.h>

#include "flatbuffer_container.h"
#include "vr_state_generated.h"

class UserContext;

extern "C" {
int get_vr_state( uint8_t** vr_state, int vr_display_handle );
}

typedef FlatbufferContainer<VR::State> VRState;

void print_vr_state( const VRState& state );

bool vr_state_get( VRState& vr_state, UserContext& user_context );
void vr_gles_draw( UserContext& user_context );
void vr_render_loop( void* arg );
void vr_present( void* arg );
void vr_prepare( UserContext& user_context );

void switch_to_vr( UserContext& user_context );
EM_BOOL on_click_switch_to_vr( int, const EmscriptenMouseEvent*, void* arg );
void on_vr_init( void* );

#endif // WASMVR_VR_H
