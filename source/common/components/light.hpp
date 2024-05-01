#pragma once

#include "../ecs/component.hpp"

#include <glm/glm.hpp>

namespace our
{

    class LightComponent : public Component
    {
    public:
        int lightType;
        glm::vec3 direction;
        glm::vec3 color;
        glm::vec3 attenuation;
        glm::vec2 coneAngles;
        float intensity;
        static std::string getID() { return "Light"; }

        void deserialize(const nlohmann::json &data) override;
    };

}