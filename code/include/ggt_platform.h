//
// GGT PLATFORM - v5
//
// A cross-platform (though currently only for windows, linux and wasm -- using emscripten) platform layer
// for games and the such. It can open a window with opengl
// and provides some other useful functions.
// Feel free to use and/or modify this.
//
// Usage:
//  - You should have functions named ggtp_init, ggtp_loop and ggtp_draw that
//    will be called when the program is starting or every frame.
//    ggtp_init must call ggtp_create_window.
//  - To compile this, #define GGT_PLATFORM_IMPLEMENTATION and #include the
//    header
//
// Options:
//  - GGTP_PROGRAM_STATE {TYPE} if you want to have a variable of type
//     {TYPE} whose pointer is passed to the user functions
//  - GGTP_MAX_EVENTS_PER_LOOP, which is 256 by default
//
// Todo:
//  - Finish mouse implementation for non-windows systems

#ifndef GGT_PLATFORM_H
#define GGT_PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif
    
#define GGT_SUCCESS 1
#define GGT_FAILURE 0
    
#define GGT_C_SUCCESS 0
#define GGT_C_FAILURE 1
    
    typedef unsigned           char ggt_u8;
    typedef unsigned           int  ggt_u32;
    typedef unsigned long long int  ggt_u64;
    
#ifndef GGTP_MAX_EVENTS_PER_LOOP
#define GGTP_MAX_EVENTS_PER_LOOP  256
#endif
    
    typedef enum {
        GGTP_EVENT_KEY_DOWN,
        GGTP_EVENT_KEY_UP,
        GGTP_EVENT_MOUSE_LEFT_DOWN,
        GGTP_EVENT_MOUSE_LEFT_UP,
        GGTP_EVENT_MOUSE_RIGHT_DOWN,
        GGTP_EVENT_MOUSE_RIGHT_UP,
        GGTP_EVENT_MOUSE_MOVE,
        GGTP_EVENT_RESIZE,
        GGTP_EVENT_CLOSE,
        GGTP_EVENT_FOCUS_LOST,
    } ggt_platform_event_type;
    
    
    const char GGTP_KEY_SHIFT  = 0x10;
    const char GGTP_KEY_CTRL   = 0x11;
    const char GGTP_KEY_ALT    = 0x12;
    const char GGTP_KEY_BACKSPACE = 0x09;
    const char GGTP_KEY_RETURN = 0x0D;
    const char GGTP_KEY_ESC    = 0x1B;
    const char GGTP_KEY_SPACE  = 0x20;
    const char GGTP_KEY_LEFT   = 0x25;
    const char GGTP_KEY_UP     = 0x26;
    const char GGTP_KEY_RIGHT  = 0x27;
    const char GGTP_KEY_DOWN   = 0x28;
    
#define GGTP_TOTAL_KEYS 128
    
    typedef enum {
        GGTP_CURSOR_ARROW,
        GGTP_CURSOR_POINTER,
        GGTP_CURSOR_WAIT,
        GGTP_CURSOR_WRITE,
    } ggt_platform_cursor;
    void ggtp_set_cursor(ggt_platform_cursor cursor);
    
    typedef struct {
        int x, y;
    } ggt_vec2i;
    
    typedef struct {
        ggt_vec2i position, difference;
    } ggt_platform_mouse_movement;
    
    typedef union {
        char key;
        ggt_vec2i size;
        ggt_vec2i coords;
        ggt_platform_mouse_movement mouse_movement;
    } ggt_platform_event_info;
    
    typedef char                        _ggt_platform_event_type_of_key;
    typedef ggt_vec2i                   _ggt_platform_event_type_of_size;
    typedef ggt_vec2i                   _ggt_platform_event_type_of_coords;
    typedef ggt_platform_mouse_movement _ggt_platform_event_type_of_mouse_movement;
    
    typedef struct {
        ggt_platform_event_type type;
        ggt_platform_event_info info;
    } ggt_platform_event;
    
    typedef struct {
        ggt_u32 size;
        ggt_platform_event data[GGTP_MAX_EVENTS_PER_LOOP];
    } ggt_platform_events;
    
    typedef struct {
        ggt_vec2i position;
        int buttons[2]; // 0 is left, 1 is right
    } ggtp_mouse;
    
#ifdef GGTP_PROGRAM_STATE
    struct GGTP_PROGRAM_STATE;
    int  ggtp_init(GGTP_PROGRAM_STATE* program_state);
    int  ggtp_loop(GGTP_PROGRAM_STATE* program_state, ggt_u8 keys[GGTP_TOTAL_KEYS], ggtp_mouse mouse, ggt_platform_events events);
    void ggtp_draw(GGTP_PROGRAM_STATE* program_state);
#else
    int  ggtp_init();
    int  ggtp_loop(ggt_u8 keys[GGTP_TOTAL_KEYS], ggtp_mouse mouse, ggt_platform_events events);
    void ggtp_draw();
#endif
    
    int ggtp_create_window(int width, int height, const char *window_name, float *dpi_factor);
    
    void ggtp_program_file_path(const char *name, char *dst);
    void ggtp_user_file_path(const char *name, char *dst);
    ggt_u64 ggtp_file_modification_date(const char *filename);
    
#if defined(_WIN32)
#include <Windows.h>
    
    //
    // GL preprocessor and function definitions
    // (From https://www.khronos.org/registry/OpenGL/api/GL/glext.h)
    //
#include <gl/gl.h>
#include <stddef.h>
    
#ifndef __gl_glext_h_
    
    typedef char GLchar;
    typedef ptrdiff_t GLsizeiptr;
    typedef ptrdiff_t GLintptr;
    
    
#define GL_MAJOR_VERSION                  0x821B
#define GL_MINOR_VERSION                  0x821C
    
#define GL_STREAM_DRAW                    0x88E0
#define GL_STATIC_DRAW                    0x88E4
#define GL_DYNAMIC_DRAW                   0x88E8
    
#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_ARRAY_BUFFER_BINDING           0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING   0x8895
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING 0x889F
    
#define GL_FUNC_ADD                       0x8006
#define GL_FUNC_REVERSE_SUBTRACT          0x800B
#define GL_FUNC_SUBTRACT                  0x800A
    
#define GL_BLEND_DST_RGB                  0x80C8
#define GL_BLEND_SRC_RGB                  0x80C9
#define GL_BLEND_DST_ALPHA                0x80CA
#define GL_BLEND_SRC_ALPHA                0x80CB
#define GL_BLEND_EQUATION_RGB             0x8009
#define GL_BLEND_EQUATION_ALPHA           0x883D
    
#define GL_TEXTURE0                       0x84C0
#define GL_TEXTURE1                       0x84C1
#define GL_TEXTURE2                       0x84C2
#define GL_TEXTURE3                       0x84C3
#define GL_TEXTURE4                       0x84C4
#define GL_TEXTURE5                       0x84C5
#define GL_TEXTURE6                       0x84C6
#define GL_TEXTURE7                       0x84C7
#define GL_TEXTURE8                       0x84C8
#define GL_TEXTURE9                       0x84C9
#define GL_TEXTURE10                      0x84CA
#define GL_TEXTURE11                      0x84CB
#define GL_TEXTURE12                      0x84CC
#define GL_TEXTURE13                      0x84CD
#define GL_TEXTURE14                      0x84CE
#define GL_TEXTURE15                      0x84CF
#define GL_TEXTURE16                      0x84D0
#define GL_TEXTURE17                      0x84D1
#define GL_TEXTURE18                      0x84D2
#define GL_TEXTURE19                      0x84D3
#define GL_TEXTURE20                      0x84D4
#define GL_TEXTURE21                      0x84D5
#define GL_TEXTURE22                      0x84D6
#define GL_TEXTURE23                      0x84D7
#define GL_TEXTURE24                      0x84D8
#define GL_TEXTURE25                      0x84D9
#define GL_TEXTURE26                      0x84DA
#define GL_TEXTURE27                      0x84DB
#define GL_TEXTURE28                      0x84DC
#define GL_TEXTURE29                      0x84DD
#define GL_TEXTURE30                      0x84DE
#define GL_TEXTURE31                      0x84DF
#define GL_ACTIVE_TEXTURE                 0x84E0
    
#define GL_FRAMEBUFFER_COMPLETE           0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT 0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT 0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER 0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER 0x8CDC
#define GL_FRAMEBUFFER_UNSUPPORTED        0x8CDD
#define GL_MAX_COLOR_ATTACHMENTS          0x8CDF
#define GL_COLOR_ATTACHMENT0              0x8CE0
#define GL_COLOR_ATTACHMENT1              0x8CE1
#define GL_COLOR_ATTACHMENT2              0x8CE2
#define GL_COLOR_ATTACHMENT3              0x8CE3
#define GL_COLOR_ATTACHMENT4              0x8CE4
#define GL_COLOR_ATTACHMENT5              0x8CE5
#define GL_COLOR_ATTACHMENT6              0x8CE6
#define GL_COLOR_ATTACHMENT7              0x8CE7
#define GL_COLOR_ATTACHMENT8              0x8CE8
#define GL_COLOR_ATTACHMENT9              0x8CE9
#define GL_COLOR_ATTACHMENT10             0x8CEA
#define GL_COLOR_ATTACHMENT11             0x8CEB
#define GL_COLOR_ATTACHMENT12             0x8CEC
#define GL_COLOR_ATTACHMENT13             0x8CED
#define GL_COLOR_ATTACHMENT14             0x8CEE
#define GL_COLOR_ATTACHMENT15             0x8CEF
#define GL_COLOR_ATTACHMENT16             0x8CF0
#define GL_COLOR_ATTACHMENT17             0x8CF1
#define GL_COLOR_ATTACHMENT18             0x8CF2
#define GL_COLOR_ATTACHMENT19             0x8CF3
#define GL_COLOR_ATTACHMENT20             0x8CF4
#define GL_COLOR_ATTACHMENT21             0x8CF5
#define GL_COLOR_ATTACHMENT22             0x8CF6
#define GL_COLOR_ATTACHMENT23             0x8CF7
#define GL_COLOR_ATTACHMENT24             0x8CF8
#define GL_COLOR_ATTACHMENT25             0x8CF9
#define GL_COLOR_ATTACHMENT26             0x8CFA
#define GL_COLOR_ATTACHMENT27             0x8CFB
#define GL_COLOR_ATTACHMENT28             0x8CFC
#define GL_COLOR_ATTACHMENT29             0x8CFD
#define GL_COLOR_ATTACHMENT30             0x8CFE
#define GL_COLOR_ATTACHMENT31             0x8CFF
#define GL_DEPTH_ATTACHMENT               0x8D00
#define GL_STENCIL_ATTACHMENT             0x8D20
#define GL_DEPTH_STENCIL_ATTACHMENT       0x821A
#define GL_FRAMEBUFFER                    0x8D40
#define GL_RENDERBUFFER                   0x8D41
#define GL_READ_FRAMEBUFFER               0x8CA8
#define GL_DRAW_FRAMEBUFFER               0x8CA9
    
#define GL_DEPTH24_STENCIL8               0x88F0
#define GL_DEPTH_COMPONENT16              0x81A5
#define GL_DEPTH_COMPONENT24              0x81A6
#define GL_DEPTH_COMPONENT32              0x81A7
    
#define GL_INVALID_FRAMEBUFFER_OPERATION  0x0506
    
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_VALIDATE_STATUS                0x8B83
#define GL_INFO_LOG_LENGTH                0x8B84
#define GL_CURRENT_PROGRAM                0x8B8D
    
#define GL_VERTEX_ARRAY_BINDING           0x85B5
    
#define GL_MULTISAMPLE                    0x809D
#define GL_SAMPLE_BUFFERS                 0x80A8
#define GL_SAMPLES                        0x80A9
#define GL_TEXTURE_2D_MULTISAMPLE         0x9100
    
    
#endif
    
#define GGTP_GL_FUNCTION_LIST \
    GL_FUNCTION(void, glTexImage2DMultisample, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations) \
        GL_FUNCTION(void, glBindFramebuffer, GLenum target, GLuint framebuffer) \
        GL_FUNCTION(void, glGenFramebuffers, GLsizei n, GLuint* framebuffers) \
        GL_FUNCTION(void, glFramebufferTexture2D, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) \
        GL_FUNCTION(GLenum, glCheckFramebufferStatus, GLenum target) \
        GL_FUNCTION(void, glBlitFramebuffer, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter) \
        GL_FUNCTION(void, glAttachShader, GLuint program, GLuint shader) \
        GL_FUNCTION(void, glCompileShader, GLuint shader) \
        GL_FUNCTION(GLuint, glCreateProgram, void) \
        GL_FUNCTION(GLuint, glCreateShader, GLenum type) \
        GL_FUNCTION(void, glLinkProgram, GLuint program) \
        GL_FUNCTION(void, glShaderSource, GLuint shader, GLsizei count, const GLchar **string, const GLint *length) \
        GL_FUNCTION(void, glUseProgram, GLuint program) \
        GL_FUNCTION(void, glGetProgramInfoLog, GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog) \
        GL_FUNCTION(void, glGetShaderInfoLog, GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog) \
        GL_FUNCTION(void, glValidateProgram, GLuint program) \
        GL_FUNCTION(void, glGetShaderiv, GLuint shader, GLenum pname, GLint* params) \
        GL_FUNCTION(void, glGetProgramiv, GLuint program, GLenum pname, GLint* params) \
        GL_FUNCTION(GLint, glGetUniformLocation, GLuint program, const GLchar* name) \
        GL_FUNCTION(void, glUniform4fv, GLint location, GLsizei count, const GLfloat* value) \
        GL_FUNCTION(void, glUniformMatrix3fv, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) \
        GL_FUNCTION(void, glUniformMatrix4fv, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) \
        GL_FUNCTION(void, glUniform1i, GLint location, GLint v0) \
        GL_FUNCTION(void, glUniform1f, GLint location, GLfloat v0) \
        GL_FUNCTION(void, glUniform2fv, GLint location, GLsizei count, const GLfloat* value) \
        GL_FUNCTION(void, glUniform3f, GLint location, GLfloat v0, GLfloat v1, GLfloat v2) \
        GL_FUNCTION(void, glUniform3fv, GLint location, GLsizei count, const GLfloat* value) \
        GL_FUNCTION(void, glEnableVertexAttribArray, GLuint index) \
        GL_FUNCTION(void, glDisableVertexAttribArray, GLuint index) \
        GL_FUNCTION(GLint, glGetAttribLocation, GLuint program, const GLchar* name) \
        GL_FUNCTION(void, glVertexAttribPointer, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer) \
        GL_FUNCTION(void, glVertexAttribIPointer, GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer) \
        GL_FUNCTION(void, glBindVertexArray, GLuint array) \
        GL_FUNCTION(void, glGenVertexArrays, GLsizei n, GLuint* arrays) \
        GL_FUNCTION(void, glBindBuffer, GLenum target, GLuint buffer) \
        GL_FUNCTION(void, glGenBuffers, GLsizei n, GLuint* buffers) \
        GL_FUNCTION(void, glBufferData, GLenum target, GLsizeiptr size, const void* data, GLenum usage) \
        GL_FUNCTION(void, glBufferSubData, GLenum target, GLintptr offset, GLsizeiptr size, const void * data); \
    GL_FUNCTION(void, glBlendEquation, GLenum mode) \
        GL_FUNCTION(void, glBlendEquationSeparate, GLenum modeRGB, GLenum modeAlpha) \
        GL_FUNCTION(void, glBlendFuncSeparate, GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha) \
        GL_FUNCTION(void, glDeleteBuffers, GLsizei n, const GLuint *buffers) \
        GL_FUNCTION(void, glDeleteVertexArrays, GLsizei n, const GLuint *arrays) \
        GL_FUNCTION(void, glDetachShader, GLuint program, GLuint shader) \
        GL_FUNCTION(void, glActiveTexture, GLenum texture) \
        GL_FUNCTION(void, glDeleteProgram, GLuint program) \
        GL_FUNCTION(void, glDeleteShader, GLuint shader) \
        GL_FUNCTION(void, glDeleteFramebuffers, GLsizei n, const GLuint* framebuffers) \
        GL_FUNCTION(void, glDrawBuffers, GLsizei n, const GLenum* bufs) \
        GL_FUNCTION(void, glTexImage3D, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels) \
        GL_FUNCTION(void, glTexSubImage3D, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels) \
        GL_FUNCTION(void, glDrawElementsBaseVertex, GLenum mode, GLsizei count, GLenum type, const void* indices, GLint basevertex) \
        GL_FUNCTION(const GLubyte* WINAPI, glGetStringi, GLenum name, GLuint index) \
        GL_FUNCTION(void, glBindRenderbuffer, GLenum target, GLuint renderbuffer); \
    GL_FUNCTION(void, glDeleteRenderbuffers, GLsizei n, const GLuint *renderbuffers); \
    GL_FUNCTION(void, glGenRenderbuffers, GLsizei n, GLuint *renderbuffers); \
    GL_FUNCTION(void, glRenderbufferStorage, GLenum target, GLenum internalformat, GLsizei width, GLsizei height); \
    GL_FUNCTION(void, glFramebufferRenderbuffer, GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer); \
    GL_FUNCTION(void, glRenderbufferStorageMultisample, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height); \
    GL_FUNCTION(void, glBlitNamedFramebuffer, GLuint readFramebuffer, GLuint drawFramebuffer, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
    
    // Define gl function types
#define GL_FUNCTION(TYPE, NAME, ...) typedef TYPE WINAPI _ggtp_gl_type_##NAME(__VA_ARGS__);
    GGTP_GL_FUNCTION_LIST
    #undef GL_FUNCTION
    
    // Define the global variables for gl function pointers
#define GL_FUNCTION(TYPE, NAME, ...) extern _ggtp_gl_type_##NAME *NAME;
    GGTP_GL_FUNCTION_LIST
    #undef GL_FUNCTION
    
    //
    //
    //
    
	typedef struct {
        ggt_platform_events events;
        ggt_u8 keys[GGTP_TOTAL_KEYS];
        ggt_vec2i last_mouse_position;
        ggtp_mouse mouse;
        
        HWND hWnd;
        HDC hDC;
        HGLRC hRC;
        HINSTANCE hInstance;
    } GGT_PLATFORM_GLOBALS;
    extern GGT_PLATFORM_GLOBALS ggt_platform_globals;
    
#elif defined(__EMSCRIPTEN__)
    
#include <GL/gl.h>
#include <GL/glext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES3/gl3.h>
    
#define GGTP_OPENGL_ES 1
    
    void ggtp_toggle_dpi(int status);
    extern "C" {
        float ggtp_get_pixel_ratio(void);
    };
    
#elif defined(linux)
    
#include <SDL2/SDL.h>
#include <GL/glew.h>
    
#endif
    
    
    
#ifdef __cplusplus
}
#endif
#endif // defined(GGT_PLATFORM_H)


#ifdef GGT_PLATFORM_IMPLEMENTATION
#ifdef __cplusplus
//extern "C" {
#endif

#define ggt_globals ggt_platform_globals

#define GGT_PLATFORM_ADD_EVENT(type_id, field, ...) do{ if(ggt_globals.events.size < GGTP_MAX_EVENTS_PER_LOOP){ \
ggt_globals.events.data[ggt_globals.events.size  ].type = type_id; \
_ggt_platform_event_type_of_##field info = __VA_ARGS__ ; \
ggt_globals.events.data[ggt_globals.events.size++].info.field = info; \
} }while(0)



#if defined(_WIN32)
//
// Windows implementation
//

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "gdi32.lib")

#include <initguid.h>
#include <KnownFolders.h>
//#include <ShlObj.h>
#include <wchar.h>
#include <windowsx.h>
#include <WinUser.h>
#include <ShellScalingAPI.h>

#define GGT_PLATFORM_ALERT_ERROR(message, code) MessageBox(NULL, message "\n(ggt_platform error " code ")", "ERROR", MB_OK);

#ifdef GGTP_PROGRAM_STATE
#define GGTP_INIT() ggtp_init(&program_state)
#define GGTP_LOOP(a, b, c) ggtp_loop(&program_state, a, b, c)
#define GGTP_DRAW() ggtp_draw(&program_state)
#else
#define GGTP_INIT() ggtp_init()
#define GGTP_LOOP(a, b, c) ggtp_loop(a, b, c)
#define GGTP_DRAW() ggtp_draw()
#endif

//
// WGL preprocessor and function definitions
// (From https://www.khronos.org/registry/OpenGL/api/GL/wglext.h)
//

#ifndef __wgl_wglext_h_

// Context creation
#define WGL_CONTEXT_DEBUG_BIT_ARB         0x00000001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x00000002
#define WGL_CONTEXT_MAJOR_VERSION_ARB     0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB     0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB       0x2093
#define WGL_CONTEXT_FLAGS_ARB             0x2094
#define ERROR_INVALID_VERSION_ARB         0x2095

typedef HGLRC (WINAPI * PFNWGLCREATECONTEXTATTRIBSARBPROC) (HDC hDC, HGLRC hShareContext, const int *attribList);

// Pixel formats
typedef BOOL (WINAPI * PFNWGLCHOOSEPIXELFORMATARBPROC) (HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
#define WGL_NUMBER_PIXEL_FORMATS_ARB            0x2000
#define WGL_DRAW_TO_WINDOW_ARB                  0x2001
#define WGL_SUPPORT_OPENGL_ARB                  0x2010
#define WGL_DOUBLE_BUFFER_ARB                   0x2011
#define WGL_PIXEL_TYPE_ARB                      0x2013
#define WGL_COLOR_BITS_ARB                      0x2014
#define WGL_DEPTH_BITS_ARB                      0x2022
#define WGL_STENCIL_BITS_ARB                    0x2023
#define WGL_TYPE_RGBA_ARB                       0x202B
#define WGL_SAMPLE_BUFFERS_ARB                  0x2041
#define WGL_SAMPLES_ARB                         0x2042

// Swap interval
typedef BOOL (WINAPI * PFNWGLSWAPINTERVALEXTPROC) (int interval);
typedef int (WINAPI * PFNWGLGETSWAPINTERVALEXTPROC) (void);
BOOL WINAPI wglSwapIntervalEXT (int interval);
int WINAPI wglGetSwapIntervalEXT (void);

#endif

//
//
//

GGT_PLATFORM_GLOBALS ggt_globals;

ggt_u8 _ggt_platform_get_key_code(ggt_u32 code) {
    switch (code) {
        case VK_LEFT:
        return GGTP_KEY_LEFT;
        break;
        case VK_UP:
        return GGTP_KEY_UP;
        break;
        case VK_RIGHT:
        return GGTP_KEY_RIGHT;
        break;
        case VK_DOWN:
        return GGTP_KEY_DOWN;
        case VK_ESCAPE:
        return GGTP_KEY_ESC;
        case VK_LSHIFT:
        case VK_RSHIFT:
        case VK_SHIFT:
        return GGTP_KEY_SHIFT;
        case VK_CONTROL:
        return GGTP_KEY_CTRL;
        case VK_MENU:
        return GGTP_KEY_ALT;
        case VK_BACK:
        return GGTP_KEY_BACKSPACE;
        case VK_SPACE:
        return GGTP_KEY_SPACE;
        default:
        return (ggt_u8)code;
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){
    if(hWnd != ggt_globals.hWnd) // Dismiss events from hWnd_dummy
        return DefWindowProc(hWnd, msg, wParam, lParam);
    switch(msg){
        case WM_CREATE:
        break;
        
        case WM_SIZE:
        GGT_PLATFORM_ADD_EVENT(GGTP_EVENT_RESIZE, size, {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
        break;
        
        case WM_SYSKEYDOWN:
        case WM_KEYDOWN: {
            ggt_u8 value = _ggt_platform_get_key_code((ggt_u32)wParam);
            if(value < GGTP_TOTAL_KEYS)
                ggt_globals.keys[value] = 1;
            GGT_PLATFORM_ADD_EVENT(GGTP_EVENT_KEY_DOWN, key, value);
        } break;
        
        case WM_SYSKEYUP:
        case WM_KEYUP: {
            ggt_u8 value = _ggt_platform_get_key_code((ggt_u32)wParam);
            if(value < GGTP_TOTAL_KEYS)
                ggt_globals.keys[value] = 0;
            GGT_PLATFORM_ADD_EVENT(GGTP_EVENT_KEY_UP, key, value);
        } break;
        
        case WM_LBUTTONDOWN: {
            ggt_vec2i position = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            GGT_PLATFORM_ADD_EVENT(GGTP_EVENT_MOUSE_LEFT_DOWN, coords, position);
            ggt_globals.mouse.position = position;
            ggt_globals.mouse.buttons[0] = 1;
        } break;
        
        case WM_RBUTTONDOWN: {
            ggt_vec2i position = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            GGT_PLATFORM_ADD_EVENT(GGTP_EVENT_MOUSE_RIGHT_DOWN, coords, position);
            ggt_globals.mouse.position = position;
            ggt_globals.mouse.buttons[1] = 1;
        } break;
        
        case WM_LBUTTONUP: {
            ggt_vec2i position = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            GGT_PLATFORM_ADD_EVENT(GGTP_EVENT_MOUSE_LEFT_UP, coords, position);
            ggt_globals.mouse.position = position;
            ggt_globals.mouse.buttons[0] = 0;
        } break;
        
        case WM_RBUTTONUP: {
            ggt_vec2i position = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            GGT_PLATFORM_ADD_EVENT(GGTP_EVENT_MOUSE_RIGHT_UP, coords, position);
            ggt_globals.mouse.position = position;
            ggt_globals.mouse.buttons[1] = 0;
        } break;
        
        case WM_MOUSEMOVE: {
            ggt_vec2i position = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            GGT_PLATFORM_ADD_EVENT(GGTP_EVENT_MOUSE_MOVE, mouse_movement, {position.x, position.y, position.x - ggt_globals.last_mouse_position.x, position.y - ggt_globals.last_mouse_position.y});
            ggt_globals.last_mouse_position = position;
            ggt_globals.mouse.position = position;
        } break;
        
        case WM_DESTROY:
        case WM_QUIT:
        case WM_CLOSE:
        ggt_globals.events.data[ggt_globals.events.size++].type = GGTP_EVENT_CLOSE;
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

#define GGT_PLATFORM_WINDOW_CLASS_NAME "WINDOWCLASS"

#define GL_FUNCTION(TYPE, NAME, ...) _ggtp_gl_type_##NAME *NAME;
GGTP_GL_FUNCTION_LIST
#undef GL_FUNCTION


bool WGLisExtensionSupported(const char *extension)
{
    const size_t extlen = strlen(extension);
    const char *supported = NULL;
    
    // Try To Use wglGetExtensionStringARB On Current DC, If Possible
    PROC wglGetExtString = wglGetProcAddress("wglGetExtensionsStringARB");
    
    if (wglGetExtString)
        supported = ((char*(__stdcall*)(HDC))wglGetExtString)(wglGetCurrentDC());
    
    // If That Failed, Try Standard Opengl Extensions String
    if (supported == NULL)
        supported = (char*)glGetString(GL_EXTENSIONS);
    
    // If That Failed Too, Must Be No Extensions Supported
    if (supported == NULL)
        return false;
    
    // Begin Examination At Start Of String, Increment By 1 On False Match
    for (const char* p = supported; ; p++)
    {
        // Advance p Up To The Next Possible Match
        p = strstr(p, extension);
        
        if (p == NULL)
            return false;                       // No Match
        
        // Make Sure That Match Is At The Start Of The String Or That
        // The Previous Char Is A Space, Or Else We Could Accidentally
        // Match "wglFunkywglExtension" With "wglExtension"
        
        // Also, Make Sure That The Following Character Is Space Or NULL
        // Or Else "wglExtensionTwo" Might Match "wglExtension"
        if ((p==supported || p[-1]==' ') && (p[extlen]=='\0' || p[extlen]==' '))
            return true;                        // Match
    }
}

int ggtp_create_window(int width, int height, const char *window_name, float *dpi_factor_ptr){
    int screen_x = GetSystemMetrics(SM_CXSCREEN);
    int screen_y = GetSystemMetrics(SM_CYSCREEN);
    
    RECT actualSize;
    actualSize.left = (screen_x - width) / 2;
    actualSize.right = actualSize.left + width;
    actualSize.top = (screen_y - height) / 2;
    actualSize.bottom = actualSize.top + height;
    if(!AdjustWindowRect(&actualSize, WS_OVERLAPPEDWINDOW | WS_VISIBLE, FALSE)){
        
    }
    ggt_globals.hWnd = CreateWindow(GGT_PLATFORM_WINDOW_CLASS_NAME, window_name, WS_OVERLAPPEDWINDOW | WS_VISIBLE, actualSize.left, actualSize.top, actualSize.right - actualSize.left, actualSize.bottom - actualSize.top, NULL, NULL, ggt_globals.hInstance, NULL);
    
    HWND hWnd_dummy = CreateWindow(GGT_PLATFORM_WINDOW_CLASS_NAME, window_name, 0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, ggt_globals.hInstance, NULL);
    
    
    if(dpi_factor_ptr != NULL){
        int iDpi = GetDpiForWindow(ggt_globals.hWnd);
        
        *dpi_factor_ptr = iDpi/96.f;
        
        width = MulDiv(width, iDpi, 96); 
        height = MulDiv(height, iDpi, 96); 
        
        actualSize.left = (screen_x - width) / 2;
        actualSize.right = actualSize.left + width;
        actualSize.top = (screen_y - height) / 2;
        actualSize.bottom = actualSize.top + height;
        if(!AdjustWindowRect(&actualSize, WS_OVERLAPPEDWINDOW | WS_VISIBLE, FALSE)){
            
        }
        SetWindowPos(ggt_globals.hWnd, ggt_globals.hWnd, actualSize.left, actualSize.top, actualSize.right - actualSize.left, actualSize.bottom - actualSize.top, SWP_NOZORDER | SWP_NOACTIVATE);
    }
    
    if(!ggt_globals.hWnd){
        GGT_PLATFORM_ALERT_ERROR("Cannot create window.", "1");
        return GGT_FAILURE;
    }
    if(!hWnd_dummy){
        GGT_PLATFORM_ALERT_ERROR("Cannot create dummy window.", "1");
        return GGT_FAILURE;
    }
    
    // Create a dummy openGL context to retrieve extensions
    HDC hDC_dummy = GetDC(hWnd_dummy);
    PIXELFORMATDESCRIPTOR pfd;
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.iLayerType = PFD_MAIN_PLANE;
    pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
    pfd.cDepthBits = 24;
    pfd.cColorBits = 24;
    pfd.iPixelType = PFD_TYPE_RGBA;
    
    int PixelFormat = ChoosePixelFormat(hDC_dummy, &pfd);
    if (!SetPixelFormat(hDC_dummy, PixelFormat, &pfd)) {
        GGT_PLATFORM_ALERT_ERROR("Cannot set pixel format.", "2");
        return GGT_FAILURE;
    }
    
    HGLRC hRC_dummy = wglCreateContext(hDC_dummy);
    wglMakeCurrent(hDC_dummy, hRC_dummy);
    
    // Get extensions
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB =
        (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB =
        (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
    PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT =
        (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
    
    if(!WGLisExtensionSupported("WGL_ARB_multisample"))
        printf("Warning: Could not find 'WGL_ARB_multisample'\n");
    
    
    // Set real pixel format
    ggt_globals.hDC = GetDC(ggt_globals.hWnd);
    
    int pixel_format_attribs[] = {
        WGL_DRAW_TO_WINDOW_ARB,     GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB,     GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB,      GL_TRUE,
        WGL_PIXEL_TYPE_ARB,         WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB,         32,
        WGL_DEPTH_BITS_ARB,         24,
        WGL_STENCIL_BITS_ARB,       8,
        //WGL_SAMPLE_BUFFERS_ARB,     1,
        //WGL_SAMPLES_ARB,            4,
        0
    };
    
    int pixel_format;
    UINT num_formats;
    wglChoosePixelFormatARB(ggt_globals.hDC, pixel_format_attribs, 0, 1, &pixel_format, &num_formats);
    if(!num_formats){
        int pixel_format_attribs_2[] = {
            WGL_DRAW_TO_WINDOW_ARB,     GL_TRUE,
            WGL_SUPPORT_OPENGL_ARB,     GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB,      GL_TRUE,
            WGL_PIXEL_TYPE_ARB,         WGL_TYPE_RGBA_ARB,
            WGL_COLOR_BITS_ARB,         32,
            WGL_DEPTH_BITS_ARB,         24,
            WGL_STENCIL_BITS_ARB,       8,
            0
        };
        wglChoosePixelFormatARB(ggt_globals.hDC, pixel_format_attribs_2, 0, 1, &pixel_format, &num_formats);
        
        if(!num_formats){
            GGT_PLATFORM_ALERT_ERROR("Failed to set pixel format.", "4");
            return GGT_FAILURE;
        }
    }
    
    DescribePixelFormat(ggt_globals.hDC, pixel_format, sizeof(pfd), &pfd);
    if(!SetPixelFormat(ggt_globals.hDC, pixel_format, &pfd)) {
        GGT_PLATFORM_ALERT_ERROR("Failed to set pixel format.", "5");
        return GGT_FAILURE;
    }
    
    // Create real context
    int attriblist[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 2,
        WGL_CONTEXT_MINOR_VERSION_ARB, 1,
        0
    };
    ggt_globals.hRC = wglCreateContextAttribsARB(ggt_globals.hDC, 0, attriblist);
    
    if(!ggt_globals.hRC){
        GGT_PLATFORM_ALERT_ERROR("Could not create OpenGL context.", "3");
        return GGT_FAILURE;
    }
    
    // Destroy dummy context and make the real one the current one
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC_dummy);
    DestroyWindow(hWnd_dummy);
    wglMakeCurrent(ggt_globals.hDC, ggt_globals.hRC);
    
    
    // Set vsync
    if(wglSwapIntervalEXT){
        wglSwapIntervalEXT(1);
    } else {
        printf("Warning: Could not find 'wglSwapIntervalEXT'\n");
    }
    
#define GL_FUNCTION(TYPE, NAME, ...) NAME = (_ggtp_gl_type_##NAME *)wglGetProcAddress(#NAME);
    GGTP_GL_FUNCTION_LIST
    #undef GL_FUNCTION
    
    GGT_PLATFORM_ADD_EVENT(GGTP_EVENT_RESIZE, size, {width, height});
    
    return GGT_SUCCESS;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevious, LPSTR lpCmdLine, int nCmdShow){
    ggt_globals.events.size = 0;
    
    // We register the window class
    WNDCLASS wc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
    wc.hCursor = LoadCursor(hInstance, IDC_ARROW);
    wc.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    wc.lpfnWndProc = WndProc;
    wc.lpszClassName = GGT_PLATFORM_WINDOW_CLASS_NAME;
    wc.lpszMenuName = NULL;
    wc.style = CS_VREDRAW | CS_HREDRAW;
    wc.hInstance = hInstance;
    if(!RegisterClass(&wc)){
        GGT_PLATFORM_ALERT_ERROR("Cannot register window.", "0");
        return GGT_C_FAILURE;
    }
    
    ggt_globals.hInstance = hInstance;
    
#ifdef GGTP_PROGRAM_STATE
    //GGTP_PROGRAM_STATE* program_state = (GGTP_PROGRAM_STATE *)malloc(sizeof(GGTP_PROGRAM_STATE));
    GGTP_PROGRAM_STATE program_state;
#endif
    
    
    for(int i=0; i<GGTP_TOTAL_KEYS; i++) {
        ggt_globals.keys[i] = 0;
    }
    ggt_globals.events.size = 0;
    
    if(GGTP_INIT() == GGT_FAILURE)
        return GGT_C_FAILURE;
    
    if(!ggt_globals.hWnd){
        GGT_PLATFORM_ALERT_ERROR("Window was not created in ggtp_init().", "4");
        return GGT_C_FAILURE;
    }
    
    MSG msg;
    while (1){
        while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)){
            if (!GetMessage(&msg, NULL, 0, 0)){
                break;
            }
            DispatchMessage(&msg);
            TranslateMessage(&msg);
        }
        
        ggt_platform_events events;
        events.size = ggt_globals.events.size;
        for(ggt_u32 i=0; i<events.size; i++)
            events.data[i] = ggt_globals.events.data[i];
        ggt_globals.events.size = 0;
        
        if(GGTP_LOOP(ggt_globals.keys, ggt_globals.mouse, events) == GGT_FAILURE){
            // Exit the program
            wglMakeCurrent(NULL, NULL);
            wglDeleteContext(ggt_globals.hRC);
            ReleaseDC(ggt_globals.hWnd, ggt_globals.hDC);
            return GGT_C_SUCCESS;
        }
        
        GGTP_DRAW();
        
        SwapBuffers(ggt_globals.hDC);
    }
    
    return GGT_C_SUCCESS;
}

void ggtp_set_cursor(ggt_platform_cursor cursor_id) {
    LPCTSTR win_cursor = IDC_ARROW;
    switch (cursor_id) {
        case GGTP_CURSOR_ARROW:
        win_cursor = IDC_ARROW;
        break;
        case GGTP_CURSOR_POINTER:
        win_cursor = IDC_HAND;
        break;
        case GGTP_CURSOR_WAIT:
        win_cursor = IDC_WAIT;
        break;
        case GGTP_CURSOR_WRITE:
        win_cursor = IDC_IBEAM;
        break;
    }
    HCURSOR cursor = (HCURSOR)LoadImageA(NULL, win_cursor, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_DEFAULTCOLOR | LR_SHARED);
    SetCursor(cursor);
}

void ggtp_program_file_path(const char *name, char *dst){
    sprintf(dst, "%s", name);
}
void ggtp_user_file_path(const char *name, char *dst){
    sprintf(dst, "%s", name);
}

#include <sys/types.h>
#include <sys/stat.h>
//#include <unistd.h>
ggt_u64 ggtp_file_modification_date(const char *path){
    struct _stat result;
    if(_stat(path, &result) == 0)
        return result.st_mtime;
    return 0;
}

#elif defined(__EMSCRIPTEN__)

#include <emscripten.h>
#include <emscripten/html5.h>

#ifdef GGTP_PROGRAM_STATE
#define GGTP_INIT() ggtp_init(&ggt_globals.program_state)
#define GGTP_LOOP(a, b, c) ggtp_loop(&ggt_globals.program_state, a, b, c)
#define GGTP_DRAW() ggtp_draw(&ggt_globals.program_state)
#else
#define GGTP_INIT() ggtp_init()
#define GGTP_LOOP(a, b, c) ggtp_loop(a, b, c)
#define GGTP_DRAW() ggtp_draw()
#endif


struct {
    GGTP_PROGRAM_STATE program_state;
    ggt_u8 keys[GGTP_TOTAL_KEYS];
    ggt_platform_events events;
    ggtp_mouse mouse;
    
    ggt_vec2i last_mouse_position;
    float device_pixel_ratio;
} ggt_globals;

EM_JS(int, window_width, (void), {
          return window.innerWidth;
      });

EM_JS(int, window_height, (void), {
          return window.innerHeight;
      });

EM_JS(float, ggtp_get_pixel_ratio, (void), {
          return window.devicePixelRatio || 1;
      });

EM_BOOL key_callback(int eventType, const EmscriptenKeyboardEvent *e, void *userData)
{
    char key = (char)e->keyCode;
    if(key == 16)
        key = GGTP_KEY_SHIFT;
    
    if(eventType == EMSCRIPTEN_EVENT_KEYUP){
        GGT_PLATFORM_ADD_EVENT(GGTP_EVENT_KEY_UP, key, key);
        ggt_globals.keys[key] = 0;
    }else{
        GGT_PLATFORM_ADD_EVENT(GGTP_EVENT_KEY_DOWN, key, key);
        ggt_globals.keys[key] = 1;
    }
    
    return 0;
}

EM_BOOL mouse_callback(int eventType, const EmscriptenMouseEvent *e, void *userData){
    ggt_vec2i position = {(int)(ggt_globals.device_pixel_ratio * e->clientX), (int)(ggt_globals.device_pixel_ratio * e->clientY)};
    switch(eventType){
        case EMSCRIPTEN_EVENT_MOUSEDOWN:
        GGT_PLATFORM_ADD_EVENT(GGTP_EVENT_MOUSE_LEFT_DOWN, coords, position);
        break;
        case EMSCRIPTEN_EVENT_MOUSEUP:
        GGT_PLATFORM_ADD_EVENT(GGTP_EVENT_MOUSE_LEFT_UP, coords, position);
        break;
        case EMSCRIPTEN_EVENT_MOUSEMOVE:
        GGT_PLATFORM_ADD_EVENT(GGTP_EVENT_MOUSE_MOVE, mouse_movement, {position.x, position.y, position.x - ggt_globals.last_mouse_position.x, position.y - ggt_globals.last_mouse_position.y});
        break;
    }
    ggt_globals.last_mouse_position = position;
    
    return 0;
}


EM_BOOL resize_callback(int eventType, const EmscriptenUiEvent *e, void *userData){
    if(eventType == EMSCRIPTEN_EVENT_RESIZE){
        Vec2i dims = {(int)(ggt_globals.device_pixel_ratio*e->windowInnerWidth), (int)(ggt_globals.device_pixel_ratio*e->windowInnerHeight)};
        GGT_PLATFORM_ADD_EVENT(GGTP_EVENT_RESIZE, size, {dims.x, dims.y});
        emscripten_set_canvas_element_size("canvas", dims.x, dims.y);
    }
    
    return 0;
}

void ggtp_set_cursor(ggt_platform_cursor cursor){
}

void main_loop(){
    ggt_platform_events events = ggt_globals.events;
    ggt_globals.events.size = 0;
    GGTP_LOOP(ggt_globals.keys, ggt_globals.mouse, events);
    GGTP_DRAW();
}

int ggtp_create_window(int width, int height, const char *window_name, float *dpi_factor){
    return GGT_SUCCESS;
}

int main(void) {
    emscripten_set_mousedown_callback("#canvas", 0, 1, mouse_callback);
    emscripten_set_mouseup_callback("#canvas", 0, 1, mouse_callback);
    emscripten_set_mousemove_callback("#canvas", 0, 1, mouse_callback);
    emscripten_set_resize_callback((const char *)2, 0, 1, resize_callback);
    emscripten_set_keydown_callback((const char *)1, 0, 1, key_callback);
    emscripten_set_keyup_callback((const char *)1, 0, 1, key_callback);
    
    ggt_globals.device_pixel_ratio = 1.f;
    int width = (int)(ggt_globals.device_pixel_ratio * window_width());
    int height = (int)(ggt_globals.device_pixel_ratio * window_height());
    emscripten_set_canvas_element_size("canvas", width, height);
    double w, h;
    emscripten_get_element_css_size("canvas", &w, &h);
    
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.alpha = 8;
    attrs.depth = 24;
    attrs.stencil = 8;
    attrs.antialias = false;
    attrs.premultipliedAlpha = false;
    attrs.premultipliedAlpha = false;
    attrs.majorVersion = 2;
    attrs.minorVersion = 0;
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attrs);
    if(ctx <= 0){
        printf("Failed to create WebGL context\n");
        switch(-(int)ctx){
            case EMSCRIPTEN_RESULT_SUCCESS:
            printf("EMSCRIPTEN_RESULT_SUCCESS\n");
            break;
            case EMSCRIPTEN_RESULT_DEFERRED:
            printf("EMSCRIPTEN_RESULT_DEFERRED\n");
            break;
            case EMSCRIPTEN_RESULT_NOT_SUPPORTED:
            printf("EMSCRIPTEN_RESULT_NOT_SUPPORTED\n");
            break;
            case EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED:
            printf("EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED\n");
            break;
            case EMSCRIPTEN_RESULT_INVALID_TARGET:
            printf("EMSCRIPTEN_RESULT_INVALID_TARGET\n");
            break;
            case EMSCRIPTEN_RESULT_UNKNOWN_TARGET:
            printf("EMSCRIPTEN_RESULT_UNKNOWN_TARGET\n");
            break;
            case EMSCRIPTEN_RESULT_INVALID_PARAM:
            printf("EMSCRIPTEN_RESULT_INVALID_PARAM\n");
            break;
            case EMSCRIPTEN_RESULT_FAILED:
            printf("EMSCRIPTEN_RESULT_FAILED\n");
            break;
            case EMSCRIPTEN_RESULT_NO_DATA:
            printf("EMSCRIPTEN_RESULT_NO_DATA\n");
            break;
            default:
            printf("UNKNOWN: %i\n", -(int)ctx);
            break;
        }
    }
    emscripten_webgl_make_context_current(ctx);
    
    for(int i=0; i<GGTP_TOTAL_KEYS; i++)
        ggt_globals.keys[i] = 0;
    ggt_globals.events.size = 0;
    
    GGTP_INIT();
    
    
    width = (int)(ggt_globals.device_pixel_ratio * window_width());
    height = (int)(ggt_globals.device_pixel_ratio * window_height());
    GGT_PLATFORM_ADD_EVENT(GGTP_EVENT_RESIZE, size, {width, height});
    
    emscripten_set_main_loop(main_loop, 0, 0);
    
    return 0;
}

void ggtp_toggle_dpi(int status){
    if(status){
        ggt_globals.device_pixel_ratio = ggtp_get_pixel_ratio();
    }else{
        ggt_globals.device_pixel_ratio = 1.f;
    }
    
    int width = (int)(ggt_globals.device_pixel_ratio * window_width());
    int height = (int)(ggt_globals.device_pixel_ratio * window_height());
    GGT_PLATFORM_ADD_EVENT(GGTP_EVENT_RESIZE, size, {width, height});
    emscripten_set_canvas_element_size("canvas", width, height);
}

void ggtp_program_file_path(const char *name, char *dst){
    sprintf(dst, "../run_tree/%s", name);
}
void ggtp_user_file_path(const char *name, char *dst){
    sprintf(dst, "../run_tree/%s", name);
}

ggt_u64 ggtp_file_modification_date(const char *path){
    return 0;
}

#else // linux, etc.
//
// SDL implementation
//

#include <SDL2/SDL.h>

#ifdef GGTP_PROGRAM_STATE
#define GGTP_INIT() ggtp_init(&program_state)
#define GGTP_LOOP(a, b) ggtp_loop(&program_state, a, b)
#define GGTP_DRAW() ggtp_draw(&program_state)
#else
#define GGTP_INIT() ggtp_init()
#define GGTP_LOOP(a, b) ggtp_loop(a, b)
#define GGTP_DRAW() ggtp_draw()
#endif

struct {
    ggt_platform_events events;
    ggt_u8 keys[GGTP_TOTAL_KEYS];
    ggt_vec2i last_mouse_position;
    ggtp_mouse mouse;
    
    SDL_Window *window;
    SDL_GLContext gl_context;
} ggt_globals;

int ggtp_create_window(int width, int height, const char *window_name, float *dpi_factor){
    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        printf("Failed to init SDL\n");
        return GGT_FAILURE;
    }
    
    ggt_globals.window = SDL_CreateWindow(window_name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    
    if(!ggt_globals.window){
        printf("Unable to create window\n");
        return GGT_FAILURE;
    }
    
    ggt_globals.gl_context = SDL_GL_CreateContext(ggt_globals.window);
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_RED_SIZE, 4);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_GREEN_SIZE, 4);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_BLUE_SIZE, 4);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_ALPHA_SIZE, 4);
    
    SDL_GL_SetSwapInterval(1);
    
    glewExperimental = GL_TRUE;
    GLenum error = glewInit();
    if(error != GLEW_OK){
        printf("Couldn't initialize glew\n");
        return GGT_FAILURE;
    }else if(!GLEW_VERSION_2_1){
        printf("Glew doesn't support openGL 2.1\n");
        return GGT_FAILURE;
    }
    
    return GGT_SUCCESS;
}

int main(){
#ifdef GGTP_PROGRAM_STATE
    //GGTP_PROGRAM_STATE* program_state = (GGTP_PROGRAM_STATE *)malloc(sizeof(GGTP_PROGRAM_STATE));
    GGTP_PROGRAM_STATE program_state;
#endif
    
    if(GGTP_INIT() == GGT_FAILURE){
        return GGT_C_FAILURE;
    }
    
    if(!ggt_globals.window){
        printf("Window was not created in ggtp_init().\n");
        return GGT_C_FAILURE;
    }
    
    for(int i=0; i<GGTP_TOTAL_KEYS; i++) {
        ggt_globals.keys[i] = 0;
    }
    ggt_globals.events.size = 0;
    
    while(1){
        
        SDL_Event e;
        while(SDL_PollEvent(&e)){
            switch(e.type){
                case SDL_KEYDOWN: {
                    //e.key.keysym.scancode;
                }   break;
                
                case SDL_KEYUP: {
                }   break;
                
                case SDL_MOUSEBUTTONDOWN:
                if(e.button.button == SDL_BUTTON_LEFT)
                    GGT_PLATFORM_ADD_EVENT(GGTP_EVENT_MOUSE_LEFT_UP, coords, {e.button.x, e.button.y});
                else if(e.button.button == SDL_BUTTON_RIGHT)
                    GGT_PLATFORM_ADD_EVENT(GGTP_EVENT_MOUSE_RIGHT_DOWN, coords, {e.button.x, e.button.y});
                break;
                
                case SDL_MOUSEBUTTONUP:
                if(e.button.button == SDL_BUTTON_LEFT)
                    GGT_PLATFORM_ADD_EVENT(GGTP_EVENT_MOUSE_LEFT_UP, coords, {e.button.x, e.button.y});
                else if(e.button.button == SDL_BUTTON_RIGHT)
                    GGT_PLATFORM_ADD_EVENT(GGTP_EVENT_MOUSE_RIGHT_UP, coords, {e.button.x, e.button.y});
                break;
                
                case SDL_MOUSEMOTION:
                GGT_PLATFORM_ADD_EVENT(GGTP_EVENT_MOUSE_MOVE, mouse_movement, {{e.button.x, e.button.y}, {e.motion.xrel, e.motion.yrel}});
                break;
                
                case SDL_WINDOWEVENT:
                switch(e.window.event){
                    case SDL_WINDOWEVENT_RESIZED:
                    GGT_PLATFORM_ADD_EVENT(GGTP_EVENT_RESIZE, size, {e.window.data1, e.window.data2});
                    break;
                    
                    case SDL_WINDOWEVENT_CLOSE:
                    GGT_PLATFORM_ADD_EVENT(GGTP_EVENT_CLOSE, key, 0);
                    break;
                    
                    //case SDL_WINDOWEVENT_FOCUS_LOST:
                    //
                    //break;
                }
                break;
                
                default:
                break;
            }
        }
        
        
        ggt_platform_events events;
        events.size = ggt_globals.events.size;
        for(ggt_u32 i=0; i<events.size; i++)
            events.data[i] = ggt_globals.events.data[i];
        ggt_globals.events.size = 0;
        
        if(GGTP_LOOP(ggt_globals.keys, mouse, events) == GGT_FAILURE){
            // Exit the program
            SDL_GL_DeleteContext(ggt_globals.gl_context);
            SDL_DestroyWindow(ggt_globals.window);
            SDL_Quit();
            return GGT_C_SUCCESS;
        }
        
        GGTP_DRAW();
        SDL_GL_SwapWindow(ggt_globals.window);
    }
    
    return GGT_C_SUCCESS;
}

void ggtp_set_cursor(ggt_platform_cursor cursor_id){
    
}


void ggtp_program_file_path(const char *name, char *dst){
    sprintf(dst, "../run_tree/%s", name);
}
void ggtp_user_file_path(const char *name, char *dst){
    sprintf(dst, "../run_tree/%s", name);
}

ggt_u64 ggtp_file_modification_date(const char *path){
    return 0;
}

#endif

#undef ggt_globals

#ifdef __cplusplus
//} // exter "C"
#endif
#endif