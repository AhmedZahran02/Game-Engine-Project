#pragma once

#include "../ecs/entity.hpp"
#include "../ecs/component.hpp"

#include <glm/glm.hpp>
#include <iostream>
#include <glm/gtx/euler_angles.hpp>

#define MIN_SPEED_FOR_ROTATION 2
#define ROTATION_CONSTANT 0.002f
#define ROTATION_SENSITIVITY 0.05f

#define JUMPING_FORCE 2.4f
#define GRAVITY 2.0f
#define GROUND_LEVEL 1.0f

using glm::vec3, glm::vec4, glm::mat4;

namespace our
{

    // This component denotes that the MovementSystem will move the owning entity by a certain linear and angular velocity.
    // This component is added as a simple example for how use the ECS framework to implement logic.
    // For more information, see "common/systems/movement.hpp"
    // For a more complex example of how to use the ECS framework, see "free-camera-controller.hpp"

    enum Movement_Type
    {
        NORMAL,
        FIXED_DIRECTION,
        FIXED_ROTATION
    };

    class MovementComponent : public Component
    {

    public:
        Movement_Type movementType;

        glm::mat4 initialTransformation;
        bool directedMovementMode = false;

        // directed movement mode
        vec3 targetPointInWorldSpace = {0, 0, 0};

        // linear velocity
        glm::vec3 forward = {0, 0, -1};
        float current_velocity = 0.0f;
        float min_velocity = -8.f;
        float max_velocity = 32.f;
        float slowdownFactor = 8.0f;
        bool constant_movement = false;

        glm::vec3 final_value = glm::vec3(0.0, 0.0, 0.0);

        // angular rotation
        bool canRoll = false;
        vec3 current_angle = {0.0f, 0.0f, 0.0f};
        glm::vec3 angular_velocity = {0, 0, 0};
        float max_angular_velocity = 6.0f;
        float angularSlowdownFactor = 8.0f;

        vec3 lastWallNormal = vec3(0, 0, 0);
        bool stopMovingOneFrame = false;
        glm::vec3 collidedWallNormal = glm::vec3(0.0, 0.0, 0.0);

        // The ID of this component type is "Movement"
        static std::string getID() { return "Movement"; }

        glm::vec3 getMovementDirection(glm::mat4 M)
        {
            return M * vec4(forward, 0.0f);
        }

        glm::vec3 getCurrentForwardVector()
        {
            return getOwner()->getLocalToWorldMatrix() * vec4(forward, 0.0f);
        }

        glm::vec3 getLookAtPoint(glm::mat4 M)
        {
            return M * vec4(forward, 1.0f);
        }

        glm::vec3 getCurrentPositionInWorld()
        {
            glm::mat4 ownerLocalToWorld = getOwner()->getLocalToWorldMatrix();
            return glm::vec3(ownerLocalToWorld[3][0], ownerLocalToWorld[3][1], ownerLocalToWorld[3][2]);
        }

        void setCurrentPositionInWorld(glm::vec3 position)
        {
            getOwner()->localTransform.position = position;
        }

        void setCurrentAngleInWorld(glm::vec3 rotation)
        {
            getOwner()->localTransform.rotation = rotation;
        }

        void clampSpeed()
        {
            if (current_velocity >= max_velocity)
                current_velocity = max_velocity;
            if (current_velocity <= min_velocity)
                current_velocity = min_velocity;
        }

        void adjustSpeed(float factor)
        {
            current_velocity += factor;
            clampSpeed();
        }

        void setSpeed(float speed)
        {
            current_velocity = speed;
            clampSpeed();

        }

        void roll()
        {
            angular_velocity.x = 0.8f * current_velocity;
            angular_velocity.x = std::min(angular_velocity.x, max_angular_velocity);
        }

        void updateAngle(float deltatime)
        {
            current_angle += angular_velocity * (current_velocity > 0 ? 1.0f : current_velocity == 0.0f ? 0
                                                                                                        : -1.0f) *
                             deltatime;
            if (current_angle.x > 360)
                current_angle.x -= 360;
            if (current_angle.y > 360)
                current_angle.y -= 360;
            if (current_angle.z > 360)
                current_angle.z -= 360;
        }

        bool isMoving() { return current_velocity > MIN_SPEED_FOR_ROTATION; }

        float getRotationAngle()
        {
            return ROTATION_CONSTANT * current_velocity;
        }

        void setForward(glm::vec3 forw)
        {
            forw = initialTransformation * vec4(forw, .0f);
            if (glm::length(forw) > 0)
                forw = glm::normalize(forw);
            forward = forw;
        }

        void decreaseSpeed(float deltaTime)
        {
            if (current_velocity == 0 || ascending || descending)
                return;

            float absSpeed = abs(current_velocity);

            int sign = current_velocity > 0 ? 1 : -1;

            absSpeed -= slowdownFactor * deltaTime;

            if (absSpeed < 0)
                absSpeed = 0;

            current_velocity = absSpeed * sign;
            if (canRoll)
                roll();
        }

        bool ascending = false, descending = false;
        float vertical_velocity = 0.0f;
        void jump()
        {
            // if it wasn't grounded last frame then you can't jump
            if (ascending || descending)
                return; 

            vertical_velocity = JUMPING_FORCE;
            ascending = true;
        }

        void updateJumpState(float deltaTime)
        {
            float& y = getOwner()->localTransform.position.y;
            if (ascending || descending)
            {
                y += vertical_velocity * deltaTime;
                vertical_velocity = vertical_velocity - (GRAVITY * deltaTime);
            }

            if (ascending) 
            {
                vertical_velocity = std::max(vertical_velocity , 0.0f);
                if (vertical_velocity == 0 && !descending) descending = true, ascending = false;
            }
            
            if (descending)
            {
                y = std::max(y, GROUND_LEVEL);
                if (y == GROUND_LEVEL) vertical_velocity = 0, descending = false;

            }
        }

        bool boosting = false;
        
        // Reads linearVelocity & angularVelocity from the given json object
        void deserialize(const nlohmann::json &data) override;
    };

}