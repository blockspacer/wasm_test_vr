#ifndef WASMVR_EGL_H
#define WASMVR_EGL_H

#include <EGL/egl.h>

class UserContext;

bool egl_initialize( UserContext& user_context );

#endif // WASMVR_EGL_H
