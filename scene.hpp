#pragma once

#include <cstdint>
#include <glm/glm.hpp>

#define SPHERE 0
#define BOX 1
#define TORUS 2
#define PLANE 3

struct ObjectDesc {
    const char *name;
    uint32_t type;
    glm::vec3 pos;
    glm::vec3 material;

    union {
        float radius; // spheres
        glm::vec3 size; // box
        glm::vec2 torus; // torus;
        glm::vec4 normal; // plane
    };
};

struct Scene {
    ObjectDesc objects[32];
    uint32_t nb_objects;
    uint32_t program;
    bool enable_smoothing;
    float smoothing;

    void update_scene(void);
    void update_object(uint32_t index);
    void add_object(ObjectDesc obj);
};
