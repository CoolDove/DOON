#pragma once

#include <iostream>
#include <glm/glm.hpp>
#include <DGLCore/GLCamera.h>

struct Transform {
    glm::vec3 position;
    glm::vec3 scale;
    glm::vec3 rotation;
};

class Space {
public:
    static glm::mat4 mat_world_camera(const DGL::Camera* _cam);
    static glm::mat4 mat_camera_world(const DGL::Camera* _cam);

    static glm::mat4 mat_camproj(const DGL::Camera* _cam, int _width, int _height);

    static glm::mat4 mat_world_ndc(const DGL::Camera* _cam, int _width, int _height);
    static glm::mat4 mat_ndc_world(const DGL::Camera* _cam, int _width, int _height);


    /*
    [texture space]
    ┌───────────────────────────────────────────────┐
    │(-width, height)                (width, height)│
    │                                               │
    │                     (0, 0)                    │
    │                                               │
    │(-width, -height)             (-width, -height)│
    └───────────────────────────────────────────────┘
    [uv]
    ┌────────────────────────────┐
    │(0, 1)                (1, 1)│
    │                            │
    │           (0, 0)           │
    │                            │
    │(0, 0)                (1, 0)│
    └────────────────────────────┘
    */
    // static glm::mat4 mat_texture_uv(int _width, int _height);
    // static glm::mat4 mat_uv_texture(int _width, int _height);

    // static glm::mat4 mat_brush_world(int _width, int _height);
    // static glm::mat4 mat_world_brush(int _width, int _height);

private:

};