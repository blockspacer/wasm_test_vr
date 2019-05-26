// https://emscripten.org/docs/porting/multimedia_and_graphics/OpenGL-support.html#webgl-friendly-subset-of-opengl-es-2-0-3-0

#include "vr.h"

#include <functional>
#include <math.h>
#include <sys/time.h>
#include <vector>

#include "finally.h"
#include "user_context.h"
#include "util.h"

namespace {
    const char* CANVAS_ID = "webgl-canvas";
    const char* BUTTON_ID = "enter-vr";
}

// clang-format off
EM_JS( int, get_vr_state, ( uint8_t** vr_state, int vr_display_handle ), { return impl_get_vr_state( vr_state, vr_display_handle ); } );
// clang-format on

void print_vr_state( const VRState& state ) {
    const VR::State* root = state.view();
    if( !root ) {
        STDOUT( "null" );
    } else {
        STDOUT( "State {" );
        printf( "  timestamp: %lf,\n", root->timestamp() );
        if( root->hmd() ) {
            printf( "  hmd: HMD {\n" );
            if( root->hmd()->leftProjectionMatrix() ) {
                printf( "    leftProjectionMatrix:  [\n" );
                print_flatbuffers_float_matrix4xN( root->hmd()->leftProjectionMatrix(), 6 );
                printf( "    ],\n" );
            }
            if( root->hmd()->leftViewMatrix() ) {
                printf( "    leftViewMatrix:        [\n" );
                print_flatbuffers_float_matrix4xN( root->hmd()->leftViewMatrix(), 6 );
                printf( "    ],\n" );
            }
            if( root->hmd()->rightProjectionMatrix() ) {
                printf( "    rightProjectionMatrix: [\n" );
                print_flatbuffers_float_matrix4xN( root->hmd()->rightProjectionMatrix(), 6 );
                printf( "    ],\n" );
            }
            if( root->hmd()->rightViewMatrix() ) {
                printf( "    rightViewMatrix:       [\n" );
                print_flatbuffers_float_matrix4xN( root->hmd()->rightViewMatrix(), 6 );
                printf( "    ],\n" );
            }
            if( root->hmd()->pose() ) {
                const VR::Pose& pose = *( root->hmd()->pose() );
                printf( "    pose: {\n" );
                if( pose.position() ) {
                    printf( "      position:            [" );
                    print_flatbuffers_float_vector( pose.position() );
                    printf( "],\n" );
                }
                if( pose.linearVelocity() ) {
                    printf( "      linearVelocity:      [" );
                    print_flatbuffers_float_vector( pose.linearVelocity() );
                    printf( "],\n" );
                }
                if( pose.linearAcceleration() ) {
                    printf( "      linearAcceleration:  [" );
                    print_flatbuffers_float_vector( pose.linearAcceleration() );
                    printf( "],\n" );
                }
                if( pose.orientation() ) {
                    printf( "      orientation:         [" );
                    print_flatbuffers_float_vector( pose.orientation() );
                    printf( "],\n" );
                }
                if( pose.angularVeclocity() ) {
                    printf( "      angularVeclocity:    [" );
                    print_flatbuffers_float_vector( pose.angularVeclocity() );
                    printf( "],\n" );
                }
                if( pose.angularAcceleration() ) {
                    printf( "      angularAcceleration: [" );
                    print_flatbuffers_float_vector( pose.angularAcceleration() );
                    printf( "],\n" );
                }
                printf( "    },\n" );
            }
            printf( "  }\n" );
        }
        if( root->gamepads() ) {
            printf( "  gamepads: [\n" );

            for( const VR::Gamepad* ptr_gamepad : *( root->gamepads() ) ) {
                if( !ptr_gamepad ) {
                    printf( "    null,\n" );
                } else {
                    const VR::Gamepad& gamepad = *ptr_gamepad;
                    printf( "    Gamepad {\n" );
                    printf( "      id:        \"%s\",\n", gamepad.id()->c_str() );
                    printf( "      index:     %d,\n", gamepad.index() );
                    printf( "      connected: %s,\n", true_false( gamepad.connected() ) );
                    printf( "      mapping:   \"%s\",\n", gamepad.mapping()->c_str() );
                    if( gamepad.axes() ) {
                        printf( "      axes:      [" );
                        print_flatbuffers_double_vector( gamepad.axes() );
                        printf( "],\n" );
                    }
                    if( gamepad.buttons() ) {
                        printf( "      buttons:   [\n" );
                        for( const VR::GamepadButton* ptr_button : *( gamepad.buttons() ) ) {
                            if( !ptr_button ) {
                                printf( "        null,\n" );
                            } else {
                                const VR::GamepadButton& button = *ptr_button;
                                printf( "        GamepadButton {\n" );
                                printf( "          pressed: %s,\n", true_false( button.pressed() ) );
                                printf( "          touched: %s,\n", true_false( button.touched() ) );
                                printf( "          value:   %lf,\n", button.value() );
                                printf( "        },\n" );
                            }
                        }
                        printf( "      ],\n" );
                    }
                    if( gamepad.pose() ) {
                        const VR::Pose& pose = *( gamepad.pose() );
                        printf( "      pose: {\n" );
                        if( pose.position() ) {
                            printf( "        position:            [" );
                            print_flatbuffers_float_vector( pose.position() );
                            printf( "],\n" );
                        }
                        if( pose.linearVelocity() ) {
                            printf( "        linearVelocity:      [" );
                            print_flatbuffers_float_vector( pose.linearVelocity() );
                            printf( "],\n" );
                        }
                        if( pose.linearAcceleration() ) {
                            printf( "        linearAcceleration:  [" );
                            print_flatbuffers_float_vector( pose.linearAcceleration() );
                            printf( "],\n" );
                        }
                        if( pose.orientation() ) {
                            printf( "        orientation:         [" );
                            print_flatbuffers_float_vector( pose.orientation() );
                            printf( "],\n" );
                        }
                        if( pose.angularVeclocity() ) {
                            printf( "        angularVeclocity:    [" );
                            print_flatbuffers_float_vector( pose.angularVeclocity() );
                            printf( "],\n" );
                        }
                        if( pose.angularAcceleration() ) {
                            printf( "        angularAcceleration: [" );
                            print_flatbuffers_float_vector( pose.angularAcceleration() );
                            printf( "],\n" );
                        }
                        printf( "      },\n" );
                    }
                    printf( "    },\n" );
                }
            }

            printf( "  ]\n" );
        }
        printf( "}\n" );
    }
}

void vr_gles_update( UserContext& user_context ) {
    VREyeParameters left_param;
    if( !emscripten_vr_get_eye_parameters( user_context.vr_display, VREyeLeft, &left_param ) ) {
        STDERR( "Failed to get VR left eye data." );
        return;
    }

    VREyeParameters right_param;
    if( !emscripten_vr_get_eye_parameters( user_context.vr_display, VREyeRight, &right_param ) ) {
        STDERR( "Failed to get VR right eye data." );
        return;
    }

    user_context.width  = left_param.renderWidth + right_param.renderWidth;
    user_context.height = std::max( left_param.renderHeight, right_param.renderHeight );
    set_canvas_size( user_context.width, user_context.height );
}

bool vr_state_get( VRState& vr_state, UserContext& user_context ) {
    if( !VRState::slab(
            &vr_state,
            [&]( uint8_t** ptr_slab ) -> int {
                return get_vr_state( ptr_slab, user_context.vr_display );
            },
            VR::VerifyStateBuffer,
            VR::GetState ) ) {
        STDERR( "Failed to get VRState." );
        return false;
    }

    return true;
}

void vr_gles_draw( UserContext& user_context ) {
    VRState vr_state;
    if( !vr_state_get( vr_state, user_context ) ) {
        STDERR( "Failed to get VR state." );
        return;
    }

    static int print_limit_counter = 0;
    if( print_limit_counter++ < 10 ) {
        print_vr_state( vr_state );
    }

    const VR::State* vr_state_view = vr_state.view();
    if( !vr_state_view ) {
        STDERR( "No content for VR state view." );
        return;
    }
    const VR::State& state = *vr_state_view;

    const VR::HMD* ptr_hmd = state.hmd();
    if( !ptr_hmd ) {
        STDERR( "No HMD defined in VR state." );
        return;
    }
    const VR::HMD& hmd = *ptr_hmd;

    {
        // Get a list of buffers to bind shader attributes to.
        const GLsizei vertex_shader_buffer_count = 1;
        GLuint        vertex_shader_buffers[vertex_shader_buffer_count];
        glGenBuffers( vertex_shader_buffer_count, vertex_shader_buffers );
        const GLuint vbuf_position = vertex_shader_buffers[0];

        // Set the viewport.
        glViewport( 0, 0, user_context.width, user_context.height );

        // Clear the color output buffer.
        glClear( GL_COLOR_BUFFER_BIT );

        // Use this shader program.
        glUseProgram( user_context.program );

        // Set model orientation.
        const double time_s = state.timestamp() / 1000.0;
        const double q      = PI * time_s / 2.0;
#define F( x ) static_cast<float>( x )
        // clang-format off
        GLfloat model_matrix_object[4 * 4] = {
            F( cos( q ) ), 0.0f, F( -sin( q ) ), F( 1.25 + sin( q ) ),
                     0.0f, 1.0f,           0.0f,                 0.0f,
            F( sin( q ) ), 0.0f, F(  cos( q ) ),                 0.0f,
                     0.0f, 0.0f,           0.0f,                 1.0f};
// clang-format on
#undef F
        bool    model_lcon_ok = false;
        bool    model_rcon_ok = false;
        GLfloat model_matrix_lcon[4 * 4];
        GLfloat model_matrix_rcon[4 * 4];

        // Pull in left and right controller matrices.
        if( state.gamepads() ) {
            for( const auto* ptr_gamepad : *( state.gamepads() ) ) {
                if( ptr_gamepad ) {
                    int index = ptr_gamepad->index();
                    if( !( ( 0 <= index ) && ( index < 2 ) ) ) {
                        continue;
                    }

                    const VR::Pose* ptr_pose = ptr_gamepad->pose();
                    if( ptr_pose ) {
                        bool six_dof = false;

                        const auto* ptr_position     = ptr_pose->position();
                        double      draw_position[3] = {0.0, 0.0, 0.0};
                        if( ptr_position && ( ptr_position->Length() == 3 ) ) {
                            flatbuffers_vector_to_native( ptr_position, draw_position );
                            six_dof = true;
                        }

                        const auto* ptr_orientation     = ptr_pose->orientation();
                        double      draw_orientation[4] = {0.0, 0.0, 0.0, 1.0}; // This API uses the wrong quaternion ordering.
                        if( ptr_orientation && ( ptr_orientation->Length() == 4 ) ) {
                            flatbuffers_vector_to_native( ptr_orientation, draw_orientation );
                        }

                        GLfloat* matrix = nullptr;
                        switch( index ) {
                        case 0:
                            matrix        = model_matrix_lcon;
                            model_lcon_ok = true;
                            break;
                        case 1:
                            matrix        = model_matrix_rcon;
                            model_rcon_ok = true;
                            break;
                        }
                        if( matrix ) {
                            quaternion_to_gl_matrix4x4(
                                draw_orientation[3], // This API uses the wrong quaternion ordering.
                                draw_orientation[0],
                                draw_orientation[1],
                                draw_orientation[2],
                                draw_position[0],
                                draw_position[1],
                                draw_position[2],
                                matrix );

                            // Add an offset to non-6DoF controllers.
                            if( !six_dof ) {
                                GLfloat offset[4 * 4] = {
                                    0.0f, 0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f, 1.0f,
                                    0.0f, 0.0f, 0.0f, 0.0f,
                                };
                                gl_matrix4x4_mac(
                                    matrix,
                                    identity4,
                                    matrix,
                                    offset );
                            }
                        }
                    }
                }
            }
        }

        // Define how to draw the scene so it can be later drawn for each eye.
        auto draw_scene = [&]() {
            const int DIMENSION = 3;

            const int VERTICES_OBJECT                              = 3;
            GLfloat   vertices_object[DIMENSION * VERTICES_OBJECT] = {
                0.0f, 0.5f, 0.0f,
                -0.5f, -0.5f, 0.0f,
                0.5f, -0.5f, 0.0f};
            // Load vertices into vertex shader buffer for vertices.
            glBindBuffer( GL_ARRAY_BUFFER, vbuf_position );
            glBufferData( GL_ARRAY_BUFFER, sizeof( vertices_object ), vertices_object, GL_STATIC_DRAW );
            // Point the shader attribute for position at the shader buffer for position.
            glBindBuffer( GL_ARRAY_BUFFER, vbuf_position );
            glVertexAttribPointer(
                user_context.vec4_position, // GLuint index
                DIMENSION,                  // GLint size (in number of vertex dimensions)
                GL_FLOAT,                   // GLenum type
                0,                          // GLboolean normalized (i.e. is-fixed-point)
                0,                          // GLsizei stride (i.e. byte offset between consecutive elements)
                0 );                        // const GLvoid * pointer (because GL_ARRAY_BUFFER is bound this is an offset into that bound buffer)
            glEnableVertexAttribArray( user_context.vec4_position );
            glUniformMatrix4fv(
                user_context.mat4_model, // GLint location
                1,                       // GLsizei count
                GL_TRUE,                 // GLboolean transpose
                model_matrix_object );   // const GLfloat* value
            glDrawArrays( GL_TRIANGLES, 0, 3 );

            if( model_lcon_ok ) {
                const int VERTICES                       = 3;
                GLfloat   vertices[DIMENSION * VERTICES] = {
                    0.0f, 0.05f, 0.0f,
                    -0.05f, -0.05f, 0.0f,
                    0.05f, -0.05f, 0.0f};
                // Load vertices into vertex shader buffer for vertices.
                glBindBuffer( GL_ARRAY_BUFFER, vbuf_position );
                glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices, GL_STATIC_DRAW );
                // Point the shader attribute for position at the shader buffer for position.
                glBindBuffer( GL_ARRAY_BUFFER, vbuf_position );
                glVertexAttribPointer(
                    user_context.vec4_position, // GLuint index
                    DIMENSION,                  // GLint size (in number of vertex dimensions)
                    GL_FLOAT,                   // GLenum type
                    0,                          // GLboolean normalized (i.e. is-fixed-point)
                    0,                          // GLsizei stride (i.e. byte offset between consecutive elements)
                    0 );                        // const GLvoid * pointer (because GL_ARRAY_BUFFER is bound this is an offset into that bound buffer)
                glEnableVertexAttribArray( user_context.vec4_position );
                glUniformMatrix4fv(
                    user_context.mat4_model, // GLint location
                    1,                       // GLsizei count
                    GL_TRUE,                 // GLboolean transpose
                    model_matrix_lcon );     // const GLfloat* value
                glDrawArrays( GL_TRIANGLES, 0, 3 );
            }

            if( model_rcon_ok ) {
                const int VERTICES                       = 3;
                GLfloat   vertices[DIMENSION * VERTICES] = {
                    0.0f, 0.05f, 0.0f,
                    -0.05f, -0.05f, 0.0f,
                    0.05f, -0.05f, 0.0f};
                // Load vertices into vertex shader buffer for vertices.
                glBindBuffer( GL_ARRAY_BUFFER, vbuf_position );
                glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices, GL_STATIC_DRAW );
                // Point the shader attribute for position at the shader buffer for position.
                glBindBuffer( GL_ARRAY_BUFFER, vbuf_position );
                glVertexAttribPointer(
                    user_context.vec4_position, // GLuint index
                    DIMENSION,                  // GLint size (in number of vertex dimensions)
                    GL_FLOAT,                   // GLenum type
                    0,                          // GLboolean normalized (i.e. is-fixed-point)
                    0,                          // GLsizei stride (i.e. byte offset between consecutive elements)
                    0 );                        // const GLvoid * pointer (because GL_ARRAY_BUFFER is bound this is an offset into that bound buffer)
                glEnableVertexAttribArray( user_context.vec4_position );
                glUniformMatrix4fv(
                    user_context.mat4_model, // GLint location
                    1,                       // GLsizei count
                    GL_TRUE,                 // GLboolean transpose
                    model_matrix_rcon );     // const GLfloat* value
                glDrawArrays( GL_TRIANGLES, 0, 3 );
            }
        };

        // Draw

        auto width_l = user_context.width / 2;
        auto width_r = user_context.width - width_l;

        GLfloat leftViewMatrix[16];
        GLfloat leftProjectionMatrix[16];
        GLfloat rightViewMatrix[16];
        GLfloat rightProjectionMatrix[16];
        flatbuffers_vector_to_native( hmd.leftViewMatrix(), leftViewMatrix );
        flatbuffers_vector_to_native( hmd.leftProjectionMatrix(), leftProjectionMatrix );
        flatbuffers_vector_to_native( hmd.rightViewMatrix(), rightViewMatrix );
        flatbuffers_vector_to_native( hmd.rightProjectionMatrix(), rightProjectionMatrix );

        // Draw left viewport.
        glUniformMatrix4fv(
            user_context.mat4_view, // GLint location
            1,                      // GLsizei count
            GL_FALSE,               // GLboolean transpose
            leftViewMatrix );       // const GLfloat* value
        glUniformMatrix4fv(
            user_context.mat4_projection, // GLint location
            1,                            // GLsizei count
            GL_FALSE,                     // GLboolean transpose
            leftProjectionMatrix );       // const GLfloat* value
        glViewport( 0, 0, width_l, user_context.height );
        draw_scene();

        // Draw right viewport.
        glUniformMatrix4fv(
            user_context.mat4_view, // GLint location
            1,                      // GLsizei count
            GL_FALSE,               // GLboolean transpose
            rightViewMatrix );      // const GLfloat* value
        glUniformMatrix4fv(
            user_context.mat4_projection, // GLint location
            1,                            // GLsizei count
            GL_FALSE,                     // GLboolean transpose
            rightProjectionMatrix );      // const GLfloat* value
        glViewport( width_l, 0, width_r, user_context.height );
        draw_scene();
    }

    if( !emscripten_vr_submit_frame( user_context.vr_display ) ) {
        STDERR( "Failed to submit frame to VR display." );
        return;
    }
}

void vr_render_loop( void* arg ) {
    UserContext& user_context = *( reinterpret_cast<UserContext*>( arg ) );

    Finally cleanup( [&]() {
        STDERR( "Canceling use of VR." );
        user_context.use_vr = false;
        emscripten_vr_cancel_display_render_loop( user_context.vr_display );
    } );

    static bool setup = false;
    if( !setup ) {
        if( !emscripten_vr_display_presenting( user_context.vr_display ) ) {
            static int waiting = 0;
            if( 1000 < ++waiting ) {
                STDERR( "Stopping waiting for VR." );
                return;
            }
            STDOUT( "Waiting to begin VR presentation." );
        } else {
            STDOUT( "VR is presenting." );
            setup = true;
        }
    } else {
        if( user_context.update_func != nullptr ) {
            user_context.update_func( user_context );
        }
        if( user_context.draw_func != nullptr ) {
            user_context.draw_func( user_context );
        }
    }

    cleanup.Clear();
}

void vr_present( void* arg ) {
    UserContext& user_context = *( reinterpret_cast<UserContext*>( arg ) );
    STDOUT( "VR presentation granted." );

    Finally cleanup( [&]() {
        STDERR( "Canceling attempt to use VR." );
        user_context.use_vr = false;
    } );

    emscripten_cancel_main_loop();
    if( !emscripten_vr_set_display_render_loop_arg( user_context.vr_display, vr_render_loop, static_cast<void*>( &user_context ) ) ) {
        STDERR( "Failed set display VR render loop of device %d.", user_context.vr_display );
        return;
    }

    user_context.update_func = vr_gles_update;
    user_context.draw_func   = vr_gles_draw;

    cleanup.Clear();
}

void vr_prepare( UserContext& user_context ) {
    if( !emscripten_vr_ready() ) {
        STDOUT( "VR not ready" );
        return;
    }

    Finally cleanup( [&]() {
        STDERR( "Canceling attempt to use VR." );
        user_context.use_vr = false;
    } );

    int display_count = emscripten_vr_count_displays();
    if( display_count < 1 ) {
        STDERR( "No VR displays found." );
        return;
    }
    STDOUT( "Found %d VR displays.", display_count );

    for( int i = 0; i < display_count; ++i ) {
        VRDisplayHandle handle = emscripten_vr_get_display_handle( i );
        const char*     name   = emscripten_vr_get_display_name( handle );

        VRDisplayCapabilities caps;
        if( !emscripten_vr_get_display_capabilities( handle, &caps ) ) {
            STDERR( "Failed to get display capabilities index=%d handle=%d.", i, handle );
            return;
        }

        STDOUT( "Display Capabilities of display %d \"%s\": {\n"
                "  hasPosition: %d,\n"
                "  hasExternalDisplay: %d,\n"
                "  canPresent: %d,\n"
                "  maxLayers: %lu\n"
                "}",
                i,
                name,
                caps.hasPosition,
                caps.hasExternalDisplay,
                caps.canPresent,
                caps.maxLayers );

        if( user_context.vr_display == VR_NOT_SET ) {
            STDOUT( "Set VR display to handle %d.", handle );
            user_context.vr_display = handle;
        }
    }

    if( emscripten_vr_display_presenting( user_context.vr_display ) ) {
        STDOUT( "Already VR presenting, jump straight into VR." );
        switch_to_vr( user_context );
    } else {
        if( EMSCRIPTEN_RESULT_SUCCESS != emscripten_set_click_callback( BUTTON_ID, static_cast<void*>( &user_context ), false, on_click_switch_to_vr ) ) {
            STDERR( "Failed to attach callback for entering VR." );
        }

        STDOUT( "Ready for user to request VR rendering loop." );
        // The user must request VR rendering from here.
    }

    cleanup.Clear();
}

void switch_to_vr( UserContext& user_context ) {
    Finally cleanup( [&]() {
        STDERR( "Canceling attempt to use VR." );
        user_context.use_vr = false;
    } );

    VRLayerInit init = {
        CANVAS_ID,
        VR_LAYER_DEFAULT_LEFT_BOUNDS,
        VR_LAYER_DEFAULT_RIGHT_BOUNDS};

    if( !emscripten_vr_request_present( user_context.vr_display, &init, 1, vr_present, static_cast<void*>( &user_context ) ) ) {
        STDERR( "Request present with default canvas failed." );
        return;
    }
    STDOUT( "Requested VR presentation." );

    cleanup.Clear();
    STDOUT( "Ready for VR rendering loop." );
}

EM_BOOL on_click_switch_to_vr( int, const EmscriptenMouseEvent*, void* arg ) {
    UserContext& user_context = *( reinterpret_cast<UserContext*>( arg ) );
    switch_to_vr( user_context );
    return true;
}

void on_vr_init( void* ) {
    STDOUT( "VR Initialized." );
}
