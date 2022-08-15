#version 430 core

layout(location=0) in vec3 position;
layout(location=1) in vec2 vertexUV;
layout(location=2) in vec3 normal;

uniform mat4 modelTransformMatrix;
uniform mat4 viewTransformMatrix;
uniform mat4 projTransformMatrix;

out vec2 UV;
out vec3 Normal;
out vec3 FragPos;

void main()
{
    vec4 v = vec4(position, 1.0);
    vec4 out_position = projTransformMatrix * viewTransformMatrix * modelTransformMatrix * v;
    gl_Position = out_position;

    FragPos = vec3(modelTransformMatrix * vec4(position, 1.0f));
    Normal = mat3 (transpose(inverse(modelTransformMatrix))) * normal;
    
    UV = vertexUV;
    
}
