
#version 450
#extension GL_ARB_separate_shader_objects: enable

layout( location = 0 ) out vec4 outColor;

layout(location = 0) in vec3 outPosition;

void main()
{
    if( outPosition.z < 0.0 )
        outColor = vec4(1.0, 0.0, 0.0, 1.0);
    else outColor = vec4(0.0, 1.0, 0.0, 1.0);
}
