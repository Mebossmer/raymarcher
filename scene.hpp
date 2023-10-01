#pragma once

#include <cstdint>
#include <glm/glm.hpp>

#define SPHERE 0
#define BOX 1
#define TORUS 2
#define PLANE 3

struct ObjectDesc {
    uint32_t type;
    glm::vec3 pos;
    uint32_t material;

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

    void add_object(ObjectDesc obj);
};
