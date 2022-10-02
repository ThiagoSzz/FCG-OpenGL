#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "collisions.h"

// Referência: https://developer.mozilla.org/en-US/docs/Games/Techniques/3D_collision_detection

bool bigTreeCollision(glm::vec4 point, glm::vec3 sphere, float radius){

    //Point-Sphere Collision
    float distance = sqrt((point.x - sphere.x) * (point.x - sphere.x) +
                          (point.y - sphere.y) * (point.y - sphere.y) +
                          (point.z - sphere.z) * (point.z - sphere.z));

    return distance < radius;
}

bool treeCollision(glm::vec4 point, SceneObject object, glm::vec3 position, float scale, float small_value){

    // Point-Cube Collision
    glm::vec3 bbox_min = object.bbox_min;
    glm::vec3 bbox_max = object.bbox_max;

    return ((point.x >= ((bbox_min.x+small_value)*scale) + position.x) &&
            (point.x <= ((bbox_max.x-small_value)*scale) + position.x) &&
            (point.z >= ((bbox_min.z+small_value)*scale) + position.z) &&
            (point.z <= ((bbox_max.z-small_value)*scale) + position.z));
}

bool logCollision(glm::vec4 point, SceneObject object, glm::vec3 position, float scale){

    // Point-Cube Collision
    glm::vec3 bbox_min = object.bbox_min;
    glm::vec3 bbox_max = object.bbox_max;

    return ((point.x >= ((bbox_min.x)*scale) + position.x) &&
            (point.x <= ((bbox_max.x)*scale) + position.x-16.5f) &&
            (point.z >= ((bbox_min.z)*scale) + position.z) &&
            (point.z <= ((bbox_max.z)*scale) + position.z-2.0f));
}
