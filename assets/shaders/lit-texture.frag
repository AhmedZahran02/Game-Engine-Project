#version 330 core

#define DIRECTIONAL 0
#define POINT       1
#define SPOT        2

in Varyings {
    vec4 color;
    vec2 tex_coord;
    vec3 normal;
    vec3 worldPos;
} fs_in;

out vec4 frag_color;
uniform sampler2D tex;

struct Light{
    int lightType;
    vec3 direction;
    vec3 position;
    vec3 color;
    vec3 attenuation;
    vec2 coneAngles;
    float intensity;
}light;

#define MAX_LIGHTS 100

uniform Light lights[MAX_LIGHTS];
uniform int lightCount;

uniform struct Material {
    vec3 ambient;  // Ka
    vec3 diffuse;  // albedo Kd
    vec3 specular; // Ks
    vec3 emission; // Ke
    float roughness;
    float refractionFactor; // Ni
    float dissolveFactor; // d
    float SpecularExponent; // Ns
    float illumModel;
} mat;

uniform vec3 cameraPos;

void main() {
    vec3 viewDir = normalize(cameraPos - fs_in.worldPos);
    vec3 color = vec3(0,0,0);
    vec3 lightDir;
    for(int lightIndex = 0; lightIndex < min(MAX_LIGHTS, lightCount); lightIndex++){
        Light light = lights[lightIndex];
        float attenuation = 1.0;
        if(light.lightType == DIRECTIONAL){
            lightDir = -light.direction;
        } else {
            lightDir = light.position - fs_in.worldPos;
            float d = length(lightDir);
            lightDir /= d;
            attenuation = 1.0 / dot(light.attenuation, vec3(d*d, d, 1.0));
            if(light.lightType == SPOT){
                float angle = dot(-light.direction, lightDir);
                attenuation *= smoothstep(light.coneAngles.y, light.coneAngles.x, angle);
            }
        }

        vec3 ambient = mat.ambient * vec3(fs_in.color);

        vec3 diffuse = light.color * mat.diffuse * max(0.0, dot(normalize(fs_in.normal), lightDir)) * light.intensity;

        vec3 reflectDir = reflect(lightDir, fs_in.normal);
        vec3 specular = vec3(0);
        if (mat.illumModel == 3 && false) {
            // Fresnel reflection calculation for Illumination Model 3
            vec3 viewDir = normalize(viewDir);
            float cosTheta = max(dot(viewDir, reflectDir), 0.0);
            float F0 = pow((mat.refractionFactor - 1.0) / (mat.refractionFactor + 1.0), 2.0);
            float spec = F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0); // Fresnel equation
            specular += mat.specular * light.color * spec * light.intensity;
        } else {
            // Regular specular reflection calculation
            specular += light.color * mat.specular * pow(max(0.0, dot(reflectDir, viewDir)), mat.SpecularExponent) * light.intensity;
        }

        color += ambient + (diffuse + specular) * attenuation;
    }

    frag_color = texture(tex,fs_in.tex_coord) * vec4(color, 1.0);
}