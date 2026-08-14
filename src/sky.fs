#version 330

in vec3 fragPosition;

out vec4 finalColor;

uniform samplerCube environmentMap;

void main() {

    vec3 color = texture(environmentMap, fragPosition).rgb;

    finalColor = vec4(color, 1.0);
}