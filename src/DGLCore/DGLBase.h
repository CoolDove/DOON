#pragma once
#include <string>

#ifdef DEBUG
#include <assert.h>

#define debug_assert(...) assert(__VA_ARGS__)

#else
#define debug_assert(...)
#endif

namespace DGL
{
enum class DGLERROR : unsigned int{
    CREATION_FAILED,
    FILE_NOT_EXIST,
    SHADER_COMPILING_FAILED,
    SHADER_MULTI_ATTACHMENT,
    LINK_UNINITED_SHADER,
};

std::string error_to_string(DGLERROR _err);

}
