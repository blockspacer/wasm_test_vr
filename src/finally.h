#ifndef WASMVR_FINALLY_H
#define WASMVR_FINALLY_H

#include <functional>

class Finally {
public:
    Finally( std::function<void( void )> functor );
    ~Finally();
    void Clear();

private:
    std::function<void( void )> functor_;
};

#endif // WASMVR_FINALLY_H
