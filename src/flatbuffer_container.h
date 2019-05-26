#ifndef WASMVR_FLATBUFFER_CONTAINER_H
#define WASMVR_FLATBUFFER_CONTAINER_H

#include <functional>

#include <flatbuffers/flatbuffers.h>

template <typename T>
class FlatbufferContainer {
public:
    FlatbufferContainer();
    virtual ~FlatbufferContainer();

    typedef std::function<int( uint8_t** )>               SlabInit;
    typedef std::function<const T*( const void* )>        ViewGet;
    typedef std::function<bool( flatbuffers::Verifier& )> ViewVerifier;

    static bool slab(
        FlatbufferContainer<T>* target,
        SlabInit                slab_init,
        ViewVerifier            view_verifier,
        ViewGet                 view_get );
    const T* view() const;

private:
    uint8_t* slab_;
    const T* view_;
};

template <typename T>
FlatbufferContainer<T>::FlatbufferContainer()
    : slab_( nullptr )
    , view_( nullptr ) {
}

template <typename T>
FlatbufferContainer<T>::~FlatbufferContainer() {
    if( slab_ ) {
        free( slab_ );
    }
}

template <typename T>
bool FlatbufferContainer<T>::slab(
    FlatbufferContainer<T>* target,
    SlabInit                slab_init,
    ViewVerifier            view_verifier,
    ViewGet                 view_get ) {
    int length = slab_init( &( target->slab_ ) );
    if( length <= 0 ) {
        return false;
    }

    // Verify that the buffer is properly formed.
    flatbuffers::Verifier verifier( target->slab_, length );
    if( !view_verifier( verifier ) ) {
        return false;
    }

    target->view_ = view_get( target->slab_ );
    return true;
}

template <typename T>
const T* FlatbufferContainer<T>::view() const {
    return view_;
}

#endif // WASMVR_FLATBUFFER_CONTAINER_H
