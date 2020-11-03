

#version 450
#extension GL_ARB_separate_shader_objects: enable

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTextureCoord;

layout(location = 0) out vec2 textureCoord;

layout(binding = 0) uniform UniformObject {
    mat4 proj;
} ubo;

void main()
{
    gl_Position = ubo.proj * vec4( inPosition, 0.0, 1.0 );
    textureCoord = inTextureCoord;
}
