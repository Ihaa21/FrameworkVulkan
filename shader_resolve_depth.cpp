#version 450

#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform sampler2DMS DepthMsTexture;

layout(location = 0) in vec2 InUv;
layout(location = 0) out float OutDepth;

void main()
{
    // NOTE: Our Z is reversed in this case
    float MaxDepth = 0.0f;

    ivec2 TexelPos = ivec2(gl_FragCoord.xy);
    int NumberOfSamples = textureSamples(DepthMsTexture);
    for (int SampleId = 0; SampleId < NumberOfSamples; ++SampleId)
    {
        MaxDepth = max(MaxDepth, texelFetch(DepthMsTexture, TexelPos, SampleId).x);
    }
    
    OutDepth = MaxDepth;
}
