#include "DGLBase.h"

#define ERRCASE(err) case DGLERROR::err: return #err;

namespace DGL
{
std::string error_to_string(DGLERROR _err) {
    switch (_err)
    {
    ERRCASE(CREATION_FAILED)
    ERRCASE(FILE_NOT_EXIST)
    ERRCASE(SHADER_COMPILING_FAILED)
    ERRCASE(SHADER_MULTI_ATTACHMENT)
    ERRCASE(LINK_UNINITED_SHADER)
    ERRCASE(BUFFER_MAPPING_FAILED)
    }
}

}