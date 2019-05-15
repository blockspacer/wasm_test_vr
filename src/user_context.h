#ifndef USER_CONTEXT_H
#define USER_CONTEXT_H

#include <EGL/egl.h>
#include <GLES3/gl3.h>

extern const int VR_NOT_SET;

class UserContext {
public:
    GLint width;
    GLint height;

    EGLDisplay display;
    EGLContext context;
    EGLSurface surface;

    GLuint program;
    GLint  vec4_position;
    GLint  mat4_model;
    GLint  mat4_view;
    GLint  mat4_projection;

    void ( *draw_func )( UserContext& );
    void ( *update_func )( UserContext& );

    bool use_vr;

    int vr_display;

    UserContext();
};

#endif // USER_CONTEXT_H
