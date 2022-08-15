#version 430 core

layout(location=0) in vec3 position;


uniform mat4 modelTransformMatrix;
uniform mat4 viewTransformMatrix;
uniform mat4 projTransformMatrix;

out vec3 UV;


void main()
{
    vec4 v = vec4(position, 1.0);
    vec4 out_position = projTransformMatrix * viewTransformMatrix * modelTransformMatrix * v;
    gl_Position = out_position;
    
    UV = position;
}
