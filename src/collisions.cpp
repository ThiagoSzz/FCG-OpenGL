#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "collisions.h"

// Referência: https://developer.mozilla.org/en-US/docs/Games/Techniques/3D_collision_detection

bool pointSphereCollision(glm::vec4 point, glm::vec3 sphere, float radius){

    //Point-Sphere Collision
    float distance = sqrt((point.x - sphere.x) * (point.x - sphere.x) +
                          (point.y - sphere.y) * (point.y - sphere.y) +
                          (point.z - sphere.z) * (point.z - sphere.z));

    return distance < radius;
}

bool pointCubeCollision(glm::vec4 point, SceneObject object, glm::vec3 position, float scale, float small_value){

    // Point-Cube Collision
    glm::vec3 bbox_min = object.bbox_min;
    glm::vec3 bbox_max = object.bbox_max;

    return ((point.x >= ((bbox_min.x+small_value)*scale) + position.x) &&
            (point.x <= ((bbox_max.x-small_value)*scale) + position.x) &&
            (point.z >= ((bbox_min.z+small_value)*scale) + position.z) &&
            (point.z <= ((bbox_max.z-small_value)*scale) + position.z));
}

bool cubeCubeCollision(glm::vec4 point, SceneObject object, glm::vec3 position, float scale){

    // Cube-Cube Collision
    glm::vec3 bbox_min = object.bbox_min;
    glm::vec3 bbox_max = object.bbox_max;

    float box_size = 0.3f;

    float pbbmin_x = point.x - box_size/2.0;
    float pbbmax_x = point.x + box_size/2.0;
    float pbbmin_z = point.z - box_size/2.0;
    float pbbmax_z = point.z + box_size/2.0;

    return ((pbbmin_x <= ((bbox_max.x)*scale) + position.x-16.5f) &&
            (pbbmax_x >= ((bbox_min.x)*scale) + position.x) &&
            (pbbmin_z <= ((bbox_max.z)*scale) + position.z-2.0f) &&
            (pbbmax_z >= ((bbox_min.z)*scale) + position.z));
}
