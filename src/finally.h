#ifndef FINALLY_H
#define FINALLY_H

#include <functional>

class Finally {
public:
    Finally( std::function<void( void )> functor );
    ~Finally();
    void Clear();

private:
    std::function<void( void )> functor_;
};

#endif // FINALLY_H
