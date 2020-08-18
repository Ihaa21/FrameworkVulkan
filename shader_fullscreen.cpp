#version 450

#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 InPos;
layout(location = 1) in vec2 InUv;

layout(location = 0) out vec2 OutUv;

void main()
{
    gl_Position = vec4(InPos, 1);
    OutUv = InUv;
}
