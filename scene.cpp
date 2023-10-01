#include "scene.hpp"
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <string>

void Scene::add_object(ObjectDesc obj) {
    glUseProgram(this->program);

    std::string prefix = std::string("objects[").append(std::to_string(this->nb_objects)).append("]");
    switch(obj.type) {
    case SPHERE:
        glUniform1ui(glGetUniformLocation(program, std::string(prefix).append(".type").c_str()), 0);
        glUniform3fv(glGetUniformLocation(program, std::string(prefix).append(".pos").c_str()), 1, glm::value_ptr(obj.pos));
        glUniform1ui(glGetUniformLocation(program, std::string(prefix).append(".material").c_str()), obj.material);
        glUniform1f(glGetUniformLocation(program, std::string(prefix).append(".radius").c_str()), obj.radius);
        break;
    case BOX:
        glUniform1ui(glGetUniformLocation(program, std::string(prefix).append(".type").c_str()), 1);
        glUniform3fv(glGetUniformLocation(program, std::string(prefix).append(".pos").c_str()), 1, glm::value_ptr(obj.pos));
        glUniform1ui(glGetUniformLocation(program, std::string(prefix).append(".material").c_str()), obj.material);
        glUniform3fv(glGetUniformLocation(program, std::string(prefix).append(".size").c_str()), 1, glm::value_ptr(obj.size));
        break;
    case TORUS:
        glUniform1ui(glGetUniformLocation(program, std::string(prefix).append(".type").c_str()), 2);
        glUniform3fv(glGetUniformLocation(program, std::string(prefix).append(".pos").c_str()), 1, glm::value_ptr(obj.pos));
        glUniform1ui(glGetUniformLocation(program, std::string(prefix).append(".material").c_str()), obj.material);
        glUniform2fv(glGetUniformLocation(program, std::string(prefix).append(".torus").c_str()), 1, glm::value_ptr(obj.torus));
        break;
    case PLANE:
        glUniform1ui(glGetUniformLocation(program, std::string(prefix).append(".type").c_str()), 3);
        glUniform1ui(glGetUniformLocation(program, std::string(prefix).append(".material").c_str()), obj.material);
        glUniform4fv(glGetUniformLocation(program, std::string(prefix).append(".normal").c_str()), 1, glm::value_ptr(obj.normal));
        break;
    }

    this->nb_objects += 1;
    glUniform1ui(glGetUniformLocation(program, "nb_objects"), this->nb_objects);
}
