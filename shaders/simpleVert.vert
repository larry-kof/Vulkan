
#version 450
#extension GL_ARB_separate_shader_objects: enable

layout(location = 0) in vec3 inPosition;
layout(binding = 0) uniform UniformObject {
    mat4 proj;
} ubo;

layout(location = 0) out vec3 outPosition;

void main()
{
    gl_Position = ubo.proj * vec4( inPosition, 1.0 );
    outPosition = inPosition;
}
