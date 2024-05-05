#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 tex_coord;
layout(location = 3) in vec3 normal;

out Varyings {
    vec3 position;
    vec4 color;
    vec2 tex_coord;
    vec3 normal;
    vec3 worldPos;
} vs_out;

uniform mat4 transform;
uniform mat4 M;
uniform mat4 M_IT;
uniform vec3 axis;
uniform float angle;

vec3 getNormalAxis(vec3 axis){
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 normal = cross(axis, up);
    return normalize(normal);
}

mat4 rotationMatrix(vec3 axis, float angle) {
    float c = cos(angle);
    float s = sin(angle);
    float t = 1.0 - c;
    
    float x = axis.x;
    float y = axis.y;
    float z = axis.z;
    
    return mat4(
        t*x*x + c,      t*x*y - s*z,    t*x*z + s*y,    0.0,
        t*x*y + s*z,    t*y*y + c,      t*y*z - s*x,    0.0,
        t*x*z - s*y,    t*y*z + s*x,    t*z*z + c,      0.0,
        0.0,            0.0,            0.0,            1.0
    );
}

mat4 IT(mat4 M)
{
    return transpose(inverse(M));
}

void main(){
    //TODO: (Req 7) Change the next line to apply the transformation matrix

    mat4 animatingMatrix = rotationMatrix(getNormalAxis(axis), angle);
    gl_Position =  transform * animatingMatrix * vec4(position, 1.0);

    // i will try to rotate the box 1 degree every second 
    vs_out.position=position;
    vs_out.color = color;
    vs_out.tex_coord = tex_coord;
    vs_out.normal = normalize((M_IT * IT(animatingMatrix) *  vec4(normal, 0.0)).xyz);
    vs_out.worldPos = vec3(M * vec4(position, 1.0));

}