#include "scene.hpp"
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <string>

void Scene::update_scene(void) {
    glUseProgram(this->program);
    glUniform1i(glGetUniformLocation(program, "enable_smoothing"), this->enable_smoothing);
    glUniform1f(glGetUniformLocation(program, "smoothing"), this->smoothing);
    glUniform1ui(glGetUniformLocation(program, "nb_objects"), this->nb_objects);
}

void Scene::update_object(uint32_t index) {
    glUseProgram(this->program);

    ObjectDesc &obj = this->objects[index];

    std::string prefix = std::string("objects[").append(std::to_string(index)).append("]");
    switch(obj.type) {
    case SPHERE:
        glUniform1ui(glGetUniformLocation(program, std::string(prefix).append(".type").c_str()), 0);
        glUniform3fv(glGetUniformLocation(program, std::string(prefix).append(".pos").c_str()), 1, glm::value_ptr(obj.pos));
        glUniform3fv(glGetUniformLocation(program, std::string(prefix).append(".material").c_str()), 1, glm::value_ptr(obj.material));
        // glUniform1ui(glGetUniformLocation(program, std::string(prefix).append(".material").c_str()), obj.material);
        glUniform1f(glGetUniformLocation(program, std::string(prefix).append(".radius").c_str()), obj.radius);
        break;
    case BOX:
        glUniform1ui(glGetUniformLocation(program, std::string(prefix).append(".type").c_str()), 1);
        glUniform3fv(glGetUniformLocation(program, std::string(prefix).append(".pos").c_str()), 1, glm::value_ptr(obj.pos));
        glUniform3fv(glGetUniformLocation(program, std::string(prefix).append(".material").c_str()), 1, glm::value_ptr(obj.material));
        // glUniform1ui(glGetUniformLocation(program, std::string(prefix).append(".material").c_str()), obj.material);
        glUniform3fv(glGetUniformLocation(program, std::string(prefix).append(".size").c_str()), 1, glm::value_ptr(obj.size));
        break;
    case TORUS:
        glUniform1ui(glGetUniformLocation(program, std::string(prefix).append(".type").c_str()), 2);
        glUniform3fv(glGetUniformLocation(program, std::string(prefix).append(".pos").c_str()), 1, glm::value_ptr(obj.pos));
        glUniform3fv(glGetUniformLocation(program, std::string(prefix).append(".material").c_str()), 1, glm::value_ptr(obj.material));
        // glUniform1ui(glGetUniformLocation(program, std::string(prefix).append(".material").c_str()), obj.material);
        glUniform2fv(glGetUniformLocation(program, std::string(prefix).append(".torus").c_str()), 1, glm::value_ptr(obj.torus));
        break;
    case PLANE:
        glUniform1ui(glGetUniformLocation(program, std::string(prefix).append(".type").c_str()), 3);
        glUniform3fv(glGetUniformLocation(program, std::string(prefix).append(".material").c_str()), 1, glm::value_ptr(obj.material));
        // glUniform1ui(glGetUniformLocation(program, std::string(prefix).append(".material").c_str()), obj.material);
        glUniform4fv(glGetUniformLocation(program, std::string(prefix).append(".normal").c_str()), 1, glm::value_ptr(obj.normal));
        break;
    }
}

void Scene::add_object(ObjectDesc obj) {
    this->objects[this->nb_objects] = obj;

    glUseProgram(this->program);

    this->update_object(this->nb_objects);

    this->nb_objects += 1;
    this->update_scene();
}
