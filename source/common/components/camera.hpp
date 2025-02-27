#pragma once

#include "../ecs/component.hpp"

#include <glm/mat4x4.hpp>

namespace our {

    // An enum that defines the type of the camera (ORTHOGRAPHIC or PERSPECTIVE) 
    enum class CameraType {
        ORTHOGRAPHIC,
        PERSPECTIVE
    };

    // This component denotes that any renderer should draw the scene relative to this camera.
    // We do not define the eye, center or up here since they can be extracted from the entity local to world matrix
    class CameraComponent : public Component {


    public:

        glm::mat4 getNormalModeViewMatrix();
        glm::mat4 getFollowModeViewMatrix();

        CameraType cameraType; // The type of the camera
        float near, far; // The distance from the camera center to the near and far plane
        float fovY; // The field of view angle of the camera if it is a perspective camera
        float orthoHeight; // The orthographic height of the camera if it is an orthographic camera
        bool stopMovingOneFram = false;


        float distance, height;
        glm::vec3 lookAtPoint = {0, 0, -1};
        bool followOwner = false;

        glm::vec3 current_position = {0., 0., 0.};
        glm::vec3 current_lookat = {0., 0., -1.};
        glm::vec3 current_up = {0., 1.0, 0.0};

        // The ID of this component type is "Camera"
        static std::string getID() { return "Camera"; }

        // Reads camera parameters from the given json object
        void deserialize(const nlohmann::json& data) override;

        // Creates and returns the camera view matrix
        glm::mat4 getViewMatrix();
        
        // Creates and returns the camera projection matrix
        // "viewportSize" is used to compute the aspect ratio
        glm::mat4 getProjectionMatrix(glm::ivec2 viewportSize) const;
    };

}