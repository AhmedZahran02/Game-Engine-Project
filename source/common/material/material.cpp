#include "material.hpp"

#include "../asset-loader.hpp"
#include "deserialize-utils.hpp"

#include <iostream>

namespace our
{

    // This function should setup the pipeline state and set the shader to be used
    void Material::setup() const
    {
        // TODO: (Req 7) Write this function
        pipelineState.setup();
        shader->use();
    }

    // This function read the material data from a json object
    void Material::deserialize(const nlohmann::json &data)
    {
        if (!data.is_object())
            return;

        if (data.contains("pipelineState"))
        {
            pipelineState.deserialize(data["pipelineState"]);
        }
        shader = AssetLoader<ShaderProgram>::get(data["shader"].get<std::string>());
        transparent = data.value("transparent", false);
    }

    void Material::setSamplerName(std::string name)
    {
        this->samplerName = name;
    }

    // This function should call the setup of its parent and
    // set the "tint" uniform to the value in the member variable tint
    void TintedMaterial::setup() const
    {
        // TODO: (Req 7) Write this function
        Material::setup();
        shader->set("tint", tint);
    }

    // This function read the material data from a json object
    void TintedMaterial::deserialize(const nlohmann::json &data)
    {
        Material::deserialize(data);
        if (!data.is_object())
            return;
        tint = data.value("tint", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    }

    // This function should call the setup of its parent and
    // set the "alphaThreshold" uniform to the value in the member variable alphaThreshold
    // Then it should bind the texture and sampler to a texture unit and send the unit number to the uniform variable "tex"
    void TexturedMaterial::setup() const
    {
        // TODO: (Req 7) Write this function
        TintedMaterial::setup();
        shader->set("alphaThreshold", alphaThreshold);
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        texture->bind();
        if (sampler)
            sampler->bind(0);
        shader->set(samplerName, 0);
    }

    // This function read the material data from a json object
    void TexturedMaterial::deserialize(const nlohmann::json &data)
    {
        TintedMaterial::deserialize(data);
        if (!data.is_object())
            return;
        alphaThreshold = data.value("alphaThreshold", 0.0f);
        texture = AssetLoader<Texture2D>::get(data.value("texture", ""));
        sampler = AssetLoader<Sampler>::get(data.value("sampler", ""));
    }

    void TexturedMaterial::setTextureUnit(unsigned int textureUnit)
    {
        this->textureUnit = textureUnit;
    }

    void LitMaterial::setup() const
    {
        Material::setup();
        shader->set("mat.ambient", ambient);
        shader->set("mat.diffuse", diffuse);
        shader->set("mat.specular", specular);
        shader->set("mat.emission", emission);
        shader->set("mat.roughness", roughness);
        shader->set("mat.SpecularExponent", SpecularExponent);
        shader->set("mat.refractionFactor", refractionFactor);
        shader->set("mat.dissolveFactor", dissolveFactor);
        shader->set("mat.illumModel", illumModel);
    }

    void LitMaterial::deserialize(const nlohmann::json &data)
    {
        Material::deserialize(data);
        if (!data.is_object())
            return;
        ambient = data.value("ambient", glm::vec3(0.0f, 0.0f, 0.0f));
        diffuse = data.value("diffuse", glm::vec3(0.0f, 0.0f, 0.0f));
        specular = data.value("specular", glm::vec3(0.0f, 0.0f, 0.0f));
        emission = data.value("emission", glm::vec3(0.0f, 0.0f, 0.0f));
        roughness = data.value("roughness", 1.0f);
        refractionFactor = data.value("refractionFactor", 1.0f);
        dissolveFactor = data.value("dissolveFactor", 1.0f);
        SpecularExponent = data.value("SpecularExponent", 0.0f);
        illumModel = data.value("illumModel", 0.0f);
    }
    void LitTexturedMaterial::setup() const
    {
        LitMaterial::setup();
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        texture->bind();
        if (sampler)
            sampler->bind(0);
        shader->set(samplerName, 0);
    }

    void LitTexturedMaterial::setTextureUnit(unsigned int textureUnit)
    {
        this->textureUnit = textureUnit;
    }

    void LitTexturedMaterial::deserialize(const nlohmann::json &data)
    {
        LitMaterial::deserialize(data);
        texture = AssetLoader<Texture2D>::get(data.value("texture", ""));
        sampler = AssetLoader<Sampler>::get(data.value("sampler", ""));
    }
}