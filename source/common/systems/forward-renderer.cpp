#include "forward-renderer.hpp"
#include "../mesh/mesh-utils.hpp"
#include "../components/ball-component.hpp"
#include "../components/movement.hpp"
#include "../texture/texture-utils.hpp"
#include <GLFW/glfw3.h>
#include <vector>

#define ANGLETHRESHOLD 1

namespace our
{

    void ForwardRenderer::initialize(glm::ivec2 windowSize, const nlohmann::json &config)
    {
        // First, we store the window size for later use
        this->windowSize = windowSize;
        basePixelSize = 0.005f;
        animationSpeed = 1.0f;
        startTime = std::chrono::steady_clock::now();

        // Then we check if there is a sky texture in the configuration
        if (config.contains("sky"))
        {
            // First, we create a sphere which will be used to draw the sky
            this->skySphere = mesh_utils::sphere(glm::ivec2(16, 16));

            // We can draw the sky using the same shader used to draw textured objects
            ShaderProgram *skyShader = new ShaderProgram();
            skyShader->attach("assets/shaders/textured.vert", GL_VERTEX_SHADER);
            skyShader->attach("assets/shaders/textured.frag", GL_FRAGMENT_SHADER);
            skyShader->link();

            // TODO: (Req 10) Pick the correct pipeline state to draw the sky
            //  Hints: the sky will be draw after the opaque objects so we would need depth testing but which depth funtion should we pick?
            //  We will draw the sphere from the inside, so what options should we pick for the face culling.
            PipelineState skyPipelineState{};
            skyPipelineState.faceCulling.enabled = 1;
            skyPipelineState.faceCulling.culledFace = GL_FRONT;
            skyPipelineState.depthTesting.enabled = 1;
            skyPipelineState.depthTesting.function = GL_LEQUAL;
            // Load the sky texture (note that we don't need mipmaps since we want to avoid any unnecessary blurring while rendering the sky)
            std::string skyTextureFile = config.value<std::string>("sky", "");
            Texture2D *skyTexture = texture_utils::loadImage(skyTextureFile, false);

            // Setup a sampler for the sky
            Sampler *skySampler = new Sampler();
            skySampler->set(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            skySampler->set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            skySampler->set(GL_TEXTURE_WRAP_S, GL_REPEAT);
            skySampler->set(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            // Combine all the aforementioned objects (except the mesh) into a material
            this->skyMaterial = new TexturedMaterial();
            this->skyMaterial->shader = skyShader;
            this->skyMaterial->texture = skyTexture;
            this->skyMaterial->sampler = skySampler;
            this->skyMaterial->pipelineState = skyPipelineState;
            this->skyMaterial->tint = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
            this->skyMaterial->alphaThreshold = 1.0f;
            this->skyMaterial->transparent = false;
        }

        // Then we check if there is a postprocessing shader in the configuration
        if (config.contains("postprocess"))
        {
            // TODO: (Req 11) Create a framebuffer
            glGenFramebuffers(1, &postprocessFrameBuffer);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, postprocessFrameBuffer);
            // TODO: (Req 11) Create a color and a depth texture and attach them to the framebuffer
            //  Hints: The color format can be (Red, Green, Blue and Alpha components with 8 bits for each channel).
            //  The depth format can be (Depth component with 24 bits).
            colorTarget = texture_utils::empty(GL_RGBA8, windowSize);
            depthTarget = texture_utils::empty(GL_DEPTH_COMPONENT24, windowSize);
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTarget->getOpenGLName(), 0);
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTarget->getOpenGLName(), 0);
            // TODO: (Req 11) Unbind the framebuffer just to be safe
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            // Create a vertex array to use for drawing the texture
            glGenVertexArrays(1, &postProcessVertexArray);

            // Create a sampler to use for sampling the scene texture in the post processing shader
            Sampler *postprocessSampler = new Sampler();
            postprocessSampler->set(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            postprocessSampler->set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            postprocessSampler->set(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            postprocessSampler->set(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            // Create the post processing shader
            ShaderProgram *postprocessShader = new ShaderProgram();
            postprocessShader->attach("assets/shaders/fullscreen.vert", GL_VERTEX_SHADER);
            postprocessShader->attach(config.value<std::string>("postprocess", ""), GL_FRAGMENT_SHADER);
            postprocessShader->link();

            // Create a post processing material
            postprocessMaterial = new TexturedMaterial();
            postprocessMaterial->shader = postprocessShader;
            postprocessMaterial->texture = colorTarget;
            postprocessMaterial->sampler = postprocessSampler;
            // The default options are fine but we don't need to interact with the depth buffer
            // so it is more performant to disable the depth mask

            postprocessMaterial->pipelineState.depthMask = false;
        }
    }

    void ForwardRenderer::destroy()
    {
        // Delete all objects related to the sky
        if (skyMaterial)
        {
            delete skySphere;
            delete skyMaterial->shader;
            delete skyMaterial->texture;
            delete skyMaterial->sampler;
            delete skyMaterial;
        }
        // Delete all objects related to post processing
        if (postprocessMaterial)
        {
            glDeleteFramebuffers(1, &postprocessFrameBuffer);
            glDeleteVertexArrays(1, &postProcessVertexArray);
            delete colorTarget;
            delete depthTarget;
            delete postprocessMaterial->sampler;
            delete postprocessMaterial->shader;
            delete postprocessMaterial;
        }
    }

    void ForwardRenderer::render(World *world)
    {
        // First of all, we search for a camera and for all the mesh renderers

        CameraComponent *camera = nullptr;
        opaqueCommands.clear();
        transparentCommands.clear();
        lightsSources.clear();
        std::vector<BallCommand> ballModels;
        for (auto entity : world->getEntities())
        {
            // If we hadn't found a camera yet, we look for a camera in this entity
            if (!camera)
                camera = entity->getComponent<CameraComponent>();
            // If this entity has a mesh renderer component

            if (auto meshRenderer = entity->getComponent<MeshRendererComponent>(); meshRenderer)
            {
                // We construct a command from it
                RenderCommand command;
                command.localToWorld = meshRenderer->getOwner()->getLocalToWorldMatrix();
                command.center = glm::vec3(command.localToWorld * glm::vec4(0, 0, 0, 1));
                command.mesh = meshRenderer->mesh;
                command.material = meshRenderer->material;

                // if it is transparent, we add it to the transparent commands list
                if (entity->parent && entity->parent->getComponent<BallComponent>() != nullptr)
                {
                    BallCommand ballCommand;
                    MovementComponent *movement = entity->parent->getComponent<MovementComponent>();
                    ballCommand.angle = movement->current_angle.x;
                    ballCommand.center = command.center, ballCommand.localToWorld = command.localToWorld, ballCommand.mesh = command.mesh, ballCommand.material = command.material;
                    ballCommand.direction = movement->forward;
                    ballCommand.filled = true;
                    ballModels.push_back(ballCommand);
                }
                else if (command.material->transparent)
                {
                    transparentCommands.push_back(command);
                }
                else
                {
                    // Otherwise, we add it to the opaque command list
                    opaqueCommands.push_back(command);
                }
            }

            if (auto lightSource = entity->getComponent<LightComponent>(); lightSource)
            {
                lightsSources.push_back(lightSource);
            }
        }

        // If there is no camera, we return (we cannot render without a camera)
        if (camera == nullptr)
            return;

        // TODO: (Req 9) Modify the following line such that "cameraForward" contains a vector pointing the camera forward direction
        //  HINT: See how you wrote the CameraComponent::getViewMatrix, it should help you solve this one
        // glm::mat4 Matrix = camera->getOwner()->getLocalToWorldMatrix();
        glm::vec3 u = vec4(camera->current_position, 1.0); // viewer
        glm::vec3 v = vec4(camera->current_lookat, 1.0);   // camera
        glm::vec3 normalized_vector = glm::normalize(v - u);
        glm::vec3 cameraForward = normalized_vector;
        std::sort(transparentCommands.begin(), transparentCommands.end(), [cameraForward](const RenderCommand &first, const RenderCommand &second)
                  {
            //TODO: (Req 9) Finish this function
            // HINT: the following return should return true "first" should be drawn before "second". 
            if(glm::dot(cameraForward,first.center)>glm::dot(cameraForward,second.center)){
                return true;
            }
            else{
                return false;
            } });

        glm::mat4 view_projection = camera->getProjectionMatrix(windowSize) * camera->getViewMatrix();

        glViewport(0, 0, this->windowSize.x, this->windowSize.y);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClearDepth(1.0);
        glColorMask(1, 1, 1, 1);
        glDepthMask(1);

        // If there is a postprocess material, bind the framebuffer
        if (postprocessMaterial)
        {
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, postprocessFrameBuffer);
            postprocessMaterial->shader->use();
            auto currentTime = std::chrono::steady_clock::now();
            float elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count();
            elapsedTime /= 25.0;

            // float animatedPixelSize = basePixelSize + std::sin(elapsedTime * animationSpeed) * basePixelSize;
            postprocessMaterial->shader->set("time", elapsedTime);
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (BallCommand ballCommand : ballModels)
        {
            ballCommand.material->setup();
            ballCommand.material->shader->set("transform", view_projection * ballCommand.localToWorld);
            ballCommand.material->shader->set("axis", ballCommand.direction);
            ballCommand.material->shader->set("angle", ballCommand.angle);
            ballCommand.material->shader->set("M", ballCommand.localToWorld);
            ballCommand.material->shader->set("M_IT", glm::transpose(glm::inverse(ballCommand.localToWorld)));
            ballCommand.material->shader->set("cameraPos", ballCommand.center);
            int index = 0;
            for (auto it = lightsSources.begin(); it != lightsSources.end(); it++, index++)
            {
                ballCommand.material->shader->set("lights[" + std::to_string(index) + "].lightType", (*it)->lightType);
                ballCommand.material->shader->set("lights[" + std::to_string(index) + "].direction", (*it)->direction);
                ballCommand.material->shader->set("lights[" + std::to_string(index) + "].color", (*it)->color);
                auto lightPosition = glm::vec3((*it)->getOwner()->getLocalToWorldMatrix() * glm::vec4((*it)->getOwner()->localTransform.position, 1.0));
                ballCommand.material->shader->set("lights[" + std::to_string(index) + "].position", lightPosition);
                ballCommand.material->shader->set("lights[" + std::to_string(index) + "].coneAngles", (*it)->coneAngles);
                ballCommand.material->shader->set("lights[" + std::to_string(index) + "].attenuation", (*it)->attenuation);
                ballCommand.material->shader->set("lights[" + std::to_string(index) + "].intensity", (*it)->intensity);
            }
            ballCommand.material->shader->set("lightCount", (int)lightsSources.size());
            ballCommand.mesh->draw();
        }

        //  Don't forget to set the "transform" uniform to be equal the model-view-projection matrix for each render command
        for (our::RenderCommand &command : opaqueCommands)
        {
            command.material->setup();
            command.material->shader->set("transform", view_projection * command.localToWorld);
            if (dynamic_cast<LitMaterial *>(command.material) || dynamic_cast<LitTexturedMaterial *>(command.material))
            {
                command.material->shader->set("M", command.localToWorld);
                command.material->shader->set("M_IT", glm::transpose(glm::inverse(command.localToWorld)));
                command.material->shader->set("cameraPos", command.center);
                int index = 0;
                for (auto it = lightsSources.begin(); it != lightsSources.end(); it++, index++)
                {
                    command.material->shader->set("lights[" + std::to_string(index) + "].lightType", (*it)->lightType);
                    command.material->shader->set("lights[" + std::to_string(index) + "].direction", (*it)->direction);
                    command.material->shader->set("lights[" + std::to_string(index) + "].color", (*it)->color);
                    auto lightPosition = glm::vec3((*it)->getOwner()->getLocalToWorldMatrix() * glm::vec4((*it)->getOwner()->localTransform.position, 1.0));
                    command.material->shader->set("lights[" + std::to_string(index) + "].position", lightPosition);
                    command.material->shader->set("lights[" + std::to_string(index) + "].coneAngles", (*it)->coneAngles);
                    command.material->shader->set("lights[" + std::to_string(index) + "].attenuation", (*it)->attenuation);
                    command.material->shader->set("lights[" + std::to_string(index) + "].intensity", (*it)->intensity);
                }
                command.material->shader->set("lightCount", (int)lightsSources.size());
            }
            command.mesh->draw();
        }
        // If there is a sky material, draw the sky
        if (this->skyMaterial)
        {
            this->skyMaterial->setup();
            glm::vec3 camera_position = u;
            our::Transform sky_transform;
            sky_transform.position = camera_position;
            glm::mat4 sky_model = sky_transform.toMat4();

            //  We can acheive the is by multiplying by an extra matrix after the projection but what values should we put in it?
            glm::mat4 alwaysBehindTransform = glm::mat4(
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 1.0f);

            skyMaterial->shader->set("transform", alwaysBehindTransform * view_projection * sky_model);
            skySphere->draw();
        }
        // TODO: (Req 9) Draw all the transparent commands
        //  Don't forget to set the "transform" uniform to be equal the model-view-projection matrix for each render command
        for (our::RenderCommand &command : transparentCommands)
        {
            command.material->setup();
            command.material->shader->set("transform", view_projection * command.localToWorld);

            if (dynamic_cast<LitMaterial *>(command.material) || dynamic_cast<LitTexturedMaterial *>(command.material))
            {
                command.material->shader->set("M", command.localToWorld);
                command.material->shader->set("M_IT", glm::transpose(glm::inverse(command.localToWorld)));
                command.material->shader->set("cameraPos", command.center);
                int index = 0;
                for (auto it = lightsSources.begin(); it != lightsSources.end(); it++, index++)
                {
                    command.material->shader->set("lights[" + std::to_string(index) + "].lightType", (*it)->lightType);
                    command.material->shader->set("lights[" + std::to_string(index) + "].direction", (*it)->direction);
                    command.material->shader->set("lights[" + std::to_string(index) + "].color", (*it)->color);
                    auto lightPosition = glm::vec3((*it)->getOwner()->getLocalToWorldMatrix() * glm::vec4((*it)->getOwner()->localTransform.position, 1.0));
                    command.material->shader->set("lights[" + std::to_string(index) + "].position", lightPosition);
                    command.material->shader->set("lights[" + std::to_string(index) + "].coneAngles", (*it)->coneAngles);
                    command.material->shader->set("lights[" + std::to_string(index) + "].attenuation", (*it)->attenuation);
                    command.material->shader->set("lights[" + std::to_string(index) + "].intensity", (*it)->intensity);
                }
                command.material->shader->set("lightCount", (int)lightsSources.size());
            }
            command.mesh->draw();
        }
        // If there is a postprocess material, apply postprocessing
        if (postprocessMaterial)
        {
            // TODO: (Req 11) Return to the default framebuffer
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            // TODO: (Req 11) Setup the postprocess material and draw the fullscreen triangle
            postprocessMaterial->setup();
            glBindVertexArray(this->postProcessVertexArray);

            glDrawArrays(GL_TRIANGLES, 0, 3);
        }
    }
}