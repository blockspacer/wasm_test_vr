#include "finally.h"

Finally::Finally( std::function<void( void )> functor )
    : functor_( functor ) {
}

Finally::~Finally() {
    if( functor_ ) {
        functor_();
    }
}

void Finally::Clear() {
    functor_ = nullptr;
}
