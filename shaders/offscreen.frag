
#version 450
#extension GL_ARB_separate_shader_objects: enable

layout( location = 0 ) in vec2 textureCoord;
layout( location = 0 ) out vec4 outColor;
layout( binding = 1 ) uniform sampler2D texSampler;

void main()
{
    vec4 color = texture( texSampler, textureCoord );
    outColor = color.bgra;
}