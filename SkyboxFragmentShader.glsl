#version 430 core

out vec4 theColor;

in vec3 UV;

uniform samplerCube skybox;

void main() {
    theColor = texture(skybox, UV);
}
