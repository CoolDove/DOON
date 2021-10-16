#include "Core/Application.h"
#include "Core/Image.h"
#include <Windows.h>

class Renderer {
public:
    Renderer(Application* _app);
    void init();
    void change_image(Image* _image);
    void render();
    void on_ui();

public:
    struct OpenGLInfo {
        string version;
        string vendor;
        string renderer;
        string shading_lang_version;
    } gl_info_;
private:
    Application* app_   = nullptr;
    Image*       image_ = nullptr;
private://GL
    HDC   device_context_;
    HGLRC gl_context_;

    unique_ptr<DGL::Shader> shader_;
    GLuint img_id;
private:// private func
    void init_opengl();
    
};