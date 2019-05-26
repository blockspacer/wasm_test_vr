#include "gles.h"

#include "user_context.h"
#include "util.h"

GLuint gles_load_shader( GLenum type, const char* shader_source, const char* name ) {
    GLuint shader;
    GLint  compiled;

    // Create the shader object
    shader = glCreateShader( type );

    if( !shader ) {
        return 0;
    }

    // Load the shader source
    glShaderSource( shader, 1, &shader_source, nullptr );

    // Compile the shader
    glCompileShader( shader );

    // Check the compile status
    glGetShaderiv( shader, GL_COMPILE_STATUS, &compiled );

    if( !compiled ) {
        GLint info_log_length = 0;

        glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &info_log_length );

        if( info_log_length > 1 ) {
            char* info_log = new char[info_log_length];

            // Terminating null-character included.
            glGetShaderInfoLog( shader, info_log_length, nullptr, info_log );
            STDERR( "Error compiling shader %s:\n%s", name, info_log );
            STDERR( "%s", shader_source );

            delete[] info_log;
        }

        glDeleteShader( shader );
        return 0;
    }

    return shader;
}

bool gles_load_shaders( UserContext& user_context ) {
    std::string vert_glsl;
    if( !get_file_contents( "src_asset/stl.vert", vert_glsl ) ) {
        STDERR( "Failed to get vertex shader." );
        return false;
    }
    STDOUT( "Got vertex shader." );

    std::string frag_glsl;
    if( !get_file_contents( "src_asset/stl.frag", frag_glsl ) ) {
        STDERR( "Failed to get fragment shader." );
        return false;
    }
    STDOUT( "Got fragment shader." );

    // Load the vertex/fragment shaders
    GLuint vertex_shader = gles_load_shader( GL_VERTEX_SHADER, vert_glsl.c_str(), "vertex" );
    if( !vertex_shader ) {
        STDERR( "Failed to compile vertex shader." );
        return false;
    }
    STDOUT( "Compiled vertex shader." );

    GLuint fragment_shader = gles_load_shader( GL_FRAGMENT_SHADER, frag_glsl.c_str(), "fragment" );
    if( !fragment_shader ) {
        STDERR( "Failed to compile fragment shader." );
        return false;
    }
    STDOUT( "Compiled fragment shader." );

    // Create the program object
    GLuint program = glCreateProgram();
    if( !program ) {
        STDERR( "Failed to create program." );
        return false;
    }

    glAttachShader( program, vertex_shader );
    glAttachShader( program, fragment_shader );

    // Link the program
    glLinkProgram( program );

    // Check the link status
    GLint linked;
    glGetProgramiv( program, GL_LINK_STATUS, &linked );
    if( !linked ) {
        STDERR( "Failed to link." );
        return false;
    }

    user_context.program         = program;
    user_context.vec4_position   = glGetAttribLocation( user_context.program, "vec4_position" );
    user_context.mat4_model      = glGetUniformLocation( user_context.program, "mat4_model" );
    user_context.mat4_view       = glGetUniformLocation( user_context.program, "mat4_view" );
    user_context.mat4_projection = glGetUniformLocation( user_context.program, "mat4_projection" );
    STDOUT( "program         = %d", user_context.program );
    STDOUT( "vec4_position   = %d", user_context.vec4_position );
    STDOUT( "mat4_model      = %d", user_context.mat4_model );
    STDOUT( "mat4_view       = %d", user_context.mat4_view );
    STDOUT( "mat4_projection = %d", user_context.mat4_projection );

    return true;
}

void gles_update( UserContext& user_context ) {
    user_context.width  = get_canvas_client_width();
    user_context.height = get_canvas_client_height();
    set_canvas_size( user_context.width, user_context.height );
}

void gles_draw( UserContext& user_context ) {
    const int DIMENSION                      = 3;
    const int VERTICES                       = 3;
    GLfloat   vertices[DIMENSION * VERTICES] = {0.0f, 0.5f, 0.0f,
                                              -0.5f, -0.5f, 0.0f,
                                              0.5f, -0.5f, 0.0f};

    // Get a list of buffers to bind shader attributes to.
    const GLsizei vertex_shader_buffer_count = 2;
    GLuint        vertex_shader_buffers[vertex_shader_buffer_count];
    glGenBuffers( vertex_shader_buffer_count, vertex_shader_buffers );
    const GLuint vbuf_position = vertex_shader_buffers[0];
    const GLuint vbuf_model    = vertex_shader_buffers[1];

    // Load vertices into vertex shader buffer for vertices.
    glBindBuffer( GL_ARRAY_BUFFER, vbuf_position );
    glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices, GL_STATIC_DRAW );

    // Set the viewport.
    glViewport( 0, 0, user_context.width, user_context.height );

    // Clear the color output buffer.
    glClear( GL_COLOR_BUFFER_BIT );

    // Use this shader program.
    glUseProgram( user_context.program );

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
        GL_FALSE,                // GLboolean transpose
        identity4 );             // const GLfloat* value

    glUniformMatrix4fv(
        user_context.mat4_view, // GLint location
        1,                      // GLsizei count
        GL_FALSE,               // GLboolean transpose
        identity4 );            // const GLfloat* value

    glUniformMatrix4fv(
        user_context.mat4_projection, // GLint location
        1,                            // GLsizei count
        GL_FALSE,                     // GLboolean transpose
        identity4 );                  // const GLfloat* value

    // Draw.
    glDrawArrays(
        GL_TRIANGLES, // GLenum mode
        0,            // GLint first
        VERTICES );   // GLsizei count (in number of vertices in this case)
}
