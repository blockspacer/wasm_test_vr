#include "controllers.h"

#include <emscripten.h>

// clang-format off
EM_JS( int, get_gamepad_list, ( uint8_t** gamepad_list ), { return impl_get_gamepad_list( gamepad_list ); } );
// clang-format on

Controllers::Controllers()
    : gamepad_list_array_( nullptr )
    , gamepad_list_( nullptr ) {
}

Controllers::~Controllers() {
    if( gamepad_list_array_ ) {
        free( gamepad_list_array_ );
    }
}

bool Controllers::get_controllers( Controllers* controllers ) {
    // Fetch the gamepad list from JavaScript.
    int length = get_gamepad_list( &( controllers->gamepad_list_array_ ) );
    if( length <= 0 ) {
        return false;
    }

    // Verify that the buffer is properly formed.
    flatbuffers::Verifier verifier( controllers->gamepad_list_array_, length );
    if( !VerifyGamepadListBuffer( verifier ) ) {
        return false;
    }

    controllers->gamepad_list_ = GetGamepadList( controllers->gamepad_list_array_ );
    return true;
}

const GamepadList* Controllers::gamepad_list() const {
    return gamepad_list_;
}
