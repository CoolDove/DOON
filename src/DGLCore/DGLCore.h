#include "BitMaskEnum.h"
#include "GLBuffer.h"
#include "GLCamera.h"
#include "GLGeoBatch.h"
#include "GLShader.h"
#include "GLProgram.h"
#include "DGLBase.h"

// tmp, openGL is inited somewhere else outside DGLCore, and this is controlled by that unkown guy
// but once it's not true, DGLCore cannot work
namespace DGL
{
inline bool gl_inited = false;

}