#pragma once
#include <Glad/glad.h>
#include <DoveLog.hpp>
#include <assert.h>

#ifdef DEBUG

// NOTE: do i really need exceptions?

inline void APIENTRY 
gl_debug_proc(GLenum source, 
              GLenum type, 
              unsigned int id, 
              GLenum severity, 
              GLsizei length, 
              const char *message, 
              const void *userParam)
{
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; // for what?

    std::string log = "";
    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             log += "[API]"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   log += "[Window System]"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: log += "[Shader Compiler]"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     log += "[Third Party]"; break;
        case GL_DEBUG_SOURCE_APPLICATION:     log += "[Application]"; break;
        case GL_DEBUG_SOURCE_OTHER:           log += "[Other]"; break;
    };

    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         log += "[high]"; break;
        case GL_DEBUG_SEVERITY_MEDIUM:       log += "[medium]"; break;
        case GL_DEBUG_SEVERITY_LOW:          log += "[low]"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: log += "[notification]"; break;
    }

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               log += "Type:Error-"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: log += "Type:Deprecated Behaviour-"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  log += "Type:Undefined Behaviour-"; break; 
        case GL_DEBUG_TYPE_PORTABILITY:         log += "Type:Portability-"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         log += "Type:Performance-"; break;
        case GL_DEBUG_TYPE_MARKER:              log += "Type:Marker-"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          log += "Type:Push Group-"; break;
        case GL_DEBUG_TYPE_POP_GROUP:           log += "Type:Pop Group-"; break;
        case GL_DEBUG_TYPE_OTHER:               log += "Type:Other-"; break;
    }

    log += message;

    DLOG_TRACE("-GL-%s", log.c_str());
}

inline void gl_debug_init() {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(gl_debug_proc, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
}
#else
inline void APIENTRY 
gl_debug_proc(GLenum source, 
              GLenum type, 
              unsigned int id, 
              GLenum severity, 
              GLsizei length, 
              const char *message, 
              const void *userParam) {}
inline void gl_debug_init() {
}
#endif

// EXCEPTION
namespace DGL::EXCEPTION
{

#define EXCEPTION_CLASS(name) class name : public DGLException {\
public:name(std::string _msg):DGLException(_msg) {};name():DGLException(){};};\
    
class DGLException {
public: 
    DGLException(const std::string& _msg)
    :   msg(_msg){}
    DGLException()
    :   msg("--"){}
    std::string msg;
};

EXCEPTION_CLASS(CREATION_FAILED)
EXCEPTION_CLASS(FILE_NOT_EXIST)
EXCEPTION_CLASS(SHADER_COMPILING_FAILED)
EXCEPTION_CLASS(SHADER_MULTI_ATTACHMENT)
EXCEPTION_CLASS(LINK_UNINITED_SHADER)
EXCEPTION_CLASS(BUFFER_MAPPING_FAILED)
EXCEPTION_CLASS(NO_UNIFORM_LOCATION_FOUND)

}
