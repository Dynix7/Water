#version 330

in vec3 vertexPosition;

out vec3 fragPosition;

uniform mat4 matProjection;
uniform mat4 matView;

void main() {

    mat4 viewNoTranslation = mat4(mat3(matView));
    vec4 clipPos = matProjection * viewNoTranslation * vec4(vertexPosition, 1.0);

    fragPosition = vertexPosition;
    gl_Position = clipPos.xyww;
}
// This is basically just the skybox example